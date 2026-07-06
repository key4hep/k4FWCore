/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "OverlayTiming.h"
#include <GaudiKernel/MsgStream.h>

#include "podio/FrameCategories.h"

#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/Constants.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

#include "k4FWCore/MetadataUtils.h"

#include <TMath.h>
#include <TROOT.h>

#include <tbb/parallel_pipeline.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// Returns the .root files contained in a directory (non-recursive).
static std::vector<std::string> filesInFolder(const std::string& folderPath) {
  std::vector<std::string> files;
  for (const auto& entry : fs::directory_iterator(folderPath)) {
    if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".root") {
      files.push_back(entry.path().string());
    }
  }
  std::sort(files.begin(), files.end());
  return files;
}

template <typename T>
inline float time_of_flight(const T& pos) {
  // Returns the time of flight to the radius in ns
  // Assumming positions in mm, then mm/m/s = 10^-3 s = 10^6 ns
  return std::sqrt((pos[0] * pos[0]) + (pos[1] * pos[1]) + (pos[2] * pos[2])) / TMath::C() * 1e6;
}

std::pair<float, float> OverlayTiming::define_time_windows(const std::string& collection_name) const {
  try {
    return {m_timeWindows.value().at(collection_name)[0], m_timeWindows.value().at(collection_name)[1]};
  } catch (const std::out_of_range& e) {
    error() << "No time window defined for collection " << collection_name << endmsg;
    throw e;
  }
}

StatusCode OverlayTiming::initialize() {
  m_uidSvc = service<IUniqueIDGenSvc>("UniqueIDGenSvc", true);
  if (!m_uidSvc) {
    error() << "Unable to get UniqueIDGenSvc" << endmsg;
  }

  // Make ROOT's global state safe for concurrent reads. IOSvc already does this,
  // but call it defensively so the OverlayThreads read pipeline is safe even if
  // OverlayTiming is driven differently.
  ROOT::EnableThreadSafety();

  // Expand any directory entries into their list of .root files. This is
  // typically used together with RandomMixBackgroundFiles, where each file is
  // an independent pseudo-event source.
  std::vector<std::vector<std::string>> inputFiles;
  for (const auto& group : m_inputFileNames.value()) {
    std::vector<std::string> expanded;
    for (const auto& entry : group) {
      if (fs::is_directory(entry)) {
        const auto found = filesInFolder(entry);
        expanded.insert(expanded.end(), found.begin(), found.end());
      } else {
        expanded.push_back(entry);
      }
    }
    inputFiles.push_back(std::move(expanded));
  }

  m_bkgEvents = make_unique<EventHolder>(inputFiles, m_randomMix.value(), m_allowReusingBackgroundFiles.value(), name());

  // In sequential mode the event counts are known upfront and can be validated
  // here. In random-mix mode they are determined lazily on first read, so an
  // empty file is reported at that point instead.
  if (!m_randomMix) {
    for (auto& counts : m_bkgEvents->m_totalNumberOfEvents) {
      for (auto& val : counts) {
        if (val == 0) {
          std::string err = "No events found in the background files";
          for (auto& file : m_inputFileNames.value()) {
            err += " " + file[0];
          }
          error() << err << endmsg;
          return StatusCode::FAILURE;
        }
      }
      if (std::any_of(counts.begin(), counts.end(),
                      [this](const size_t& val) { return this->m_startWithBackgroundEvent >= static_cast<int>(val); })) {
        throw GaudiException("StartWithBackgroundEvent is larger than the number of events in the background files",
                             name(), StatusCode::FAILURE);
      }
    }
  }

  if (m_Noverlay.empty()) {
    info() << "Using the default number of overlay events (1) for each group, since none was specified with "
              "NumberBackground "
           << endmsg;
    m_Noverlay = std::vector<double>(m_bkgEvents->size(), 1);
  }

  if (m_Poisson.empty()) {
    info() << "Using the default overlay mode (no Poission distribution) for each group, since none was specified with "
              "Poisson_random_NOverlay"
           << endmsg;
    m_Poisson = std::vector<bool>(m_bkgEvents->size(), false);
  }

  return StatusCode::SUCCESS;
}

void OverlayTiming::mergeBackgroundFrame(
    const podio::Frame& backgroundEvent, float timeOffset, int BX_number_in_train, int physBX,
    const std::vector<const edm4hep::SimTrackerHitCollection*>&           simTrackerHits,
    const std::vector<const edm4hep::SimCalorimeterHitCollection*>&       simCaloHits,
    edm4hep::MCParticleCollection&                                        oparticles,
    std::vector<edm4hep::SimTrackerHitCollection>&                        osimTrackerHits,
    std::map<int, std::map<uint64_t, edm4hep::MutableSimCalorimeterHit>>& cellIDsMap,
    std::vector<edm4hep::CaloHitContributionCollection>&                  ocaloHitContribs) const {
  const auto availableCollections = backgroundEvent.getAvailableCollections();

  if (std::find(availableCollections.begin(), availableCollections.end(), m_MCParticleCollectionName) ==
      availableCollections.end()) {
    warning() << "Collection " << m_MCParticleCollectionName << " not found in background event" << endmsg;
  }

  // To fix the relations we will need to have a map from old to new particle index
  std::map<int, int>                                           oldToNewMap;
  std::map<int, std::pair<std::vector<int>, std::vector<int>>> parentDaughterMap;

  if (m_mergeMCParticles) {
    const auto& bgParticles = backgroundEvent.get<edm4hep::MCParticleCollection>(m_MCParticleCollectionName);
    int         j           = oparticles.size();
    for (size_t i = 0; i < bgParticles.size(); ++i) {
      auto npart = bgParticles[i].clone(false);

      npart.setTime(bgParticles[i].getTime() + timeOffset);
      npart.setOverlay(true);
      oparticles.push_back(npart);
      for (const auto& parent : bgParticles[i].getParents()) {
        parentDaughterMap[j].first.push_back(parent.getObjectID().index);
      }
      for (const auto& daughter : bgParticles[i].getDaughters()) {
        parentDaughterMap[j].second.push_back(daughter.getObjectID().index);
      }
      oldToNewMap[i] = j;
      j++;
    }
    for (const auto& [index, parentsDaughters] : parentDaughterMap) {
      const auto& [parents, daughters] = parentsDaughters;
      for (const auto& parent : parents) {
        if (parentDaughterMap.find(oldToNewMap[parent]) == parentDaughterMap.end()) {
          continue;
        }
        oparticles[index].addToParents(oparticles[oldToNewMap[parent]]);
      }
      for (const auto& daughter : daughters) {
        if (parentDaughterMap.find(oldToNewMap[daughter]) == parentDaughterMap.end()) {
          continue;
        }
        oparticles[index].addToDaughters(oparticles[oldToNewMap[daughter]]);
      }
    }
  }

  for (size_t i = 0; i < simTrackerHits.size(); ++i) {
    const auto name = inputLocations(SIMTRACKERHIT_INDEX_POSITION)[i];
    debug() << "Processing collection " << name << endmsg;
    if (std::find(availableCollections.begin(), availableCollections.end(), name) == availableCollections.end()) {
      warning() << "Collection " << name << " not found in background event" << endmsg;
      continue;
    }
    const auto [this_start, this_stop] = define_time_windows(name);
    // There are only contributions to the readout if the hits are in the integration window
    if (this_stop <= (BX_number_in_train - physBX) * m_deltaT) {
      info() << "Skipping collection " << name << " as it is not in the integration window" << endmsg;
      continue;
    }
    auto& ocoll = osimTrackerHits[i];
    for (const auto&& simTrackerHit : backgroundEvent.get<edm4hep::SimTrackerHitCollection>(name)) {
      const float tof = time_of_flight(simTrackerHit.getPosition());

      if (!((simTrackerHit.getTime() + timeOffset > this_start + tof) &&
            (simTrackerHit.getTime() + timeOffset < this_stop + tof)))
        continue;
      auto nhit = simTrackerHit.clone(false);
      nhit.setOverlay(true);
      nhit.setTime(simTrackerHit.getTime() + timeOffset);
      if (m_mergeMCParticles) {
        nhit.setParticle(oparticles[oldToNewMap[simTrackerHit.getParticle().getObjectID().index]]);
      } else {
        edm4hep::MCParticle mcp = simTrackerHit.getParticle();
        if (mcp.isAvailable()) {
          // Preserve the momentum of the originating particle
          edm4hep::Vector3d mom = mcp.getMomentum();
          nhit.setMomentum({(float)mom.x, (float)mom.y, (float)mom.z});
        }
      }
      ocoll.push_back(nhit);
    }
  }

  for (size_t i = 0; i < simCaloHits.size(); ++i) {
    const auto name = inputLocations(SIMCALOHIT_INDEX_POSITION)[i];
    debug() << "Processing collection " << name << endmsg;
    if (std::find(availableCollections.begin(), availableCollections.end(), name) == availableCollections.end()) {
      warning() << "Collection " << name << " not found in background event" << endmsg;
      continue;
    }
    const auto [this_start, this_stop] = define_time_windows(name);
    // There are only contributions to the readout if the hits are in the integration window
    if (this_stop <= (BX_number_in_train - physBX) * m_deltaT) {
      info() << "Skipping collection " << name << " as it is not in the integration window" << endmsg;
      continue;
    }

    auto& calHitMap      = cellIDsMap[i];
    auto& calHitContribs = ocaloHitContribs[i];
    for (const auto&& simCaloHit : backgroundEvent.get<edm4hep::SimCalorimeterHitCollection>(name)) {
      if (calHitMap.find(simCaloHit.getCellID()) == calHitMap.end()) {
        // There is no hit at this position. The new hit can be added, if it is not outside the window
        auto calhit = edm4hep::MutableSimCalorimeterHit();
        bool add    = false;
        for (const auto& contrib : simCaloHit.getContributions()) {
          if ((contrib.getTime() + timeOffset > this_start) && (contrib.getTime() + timeOffset < this_stop)) {
            add = true;
            // TODO: Make sure a contribution is not added twice
            auto newContrib = contrib.clone(false);
            if (m_mergeMCParticles) {
              newContrib.setParticle(oparticles[oldToNewMap[contrib.getParticle().getObjectID().index]]);
            } else {
              newContrib.setParticle(edm4hep::MCParticle());
            }
            newContrib.setTime(contrib.getTime() + timeOffset);
            calhit.addToContributions(newContrib);
            calHitContribs.push_back(newContrib);
          }
        }
        if (add) {
          calhit.setCellID(simCaloHit.getCellID());
          calhit.setEnergy(simCaloHit.getEnergy());
          calhit.setPosition(simCaloHit.getPosition());
          calHitMap[calhit.getCellID()] = calhit;
        }
      } else {
        // there is already a hit at this position
        auto& calhit = calHitMap[simCaloHit.getCellID()];
        for (const auto& contrib : simCaloHit.getContributions()) {
          if ((contrib.getTime() + timeOffset > this_start) && (contrib.getTime() + timeOffset < this_stop)) {
            // TODO: Make sure a contribution is not added twice
            auto newContrib = contrib.clone(false);
            if (m_mergeMCParticles) {
              newContrib.setParticle(oparticles[oldToNewMap[contrib.getParticle().getObjectID().index]]);
            } else {
              newContrib.setParticle(edm4hep::MCParticle());
            }
            newContrib.setTime(contrib.getTime() + timeOffset);
            calhit.addToContributions(newContrib);
            calHitContribs.push_back(newContrib);
          }
        }
      }
    }
  }
}

retType OverlayTiming::operator()(const edm4hep::EventHeaderCollection& headers,
                                  const edm4hep::MCParticleCollection& particles,
                                  const std::vector<const edm4hep::SimTrackerHitCollection*>& simTrackerHits,
                                  const std::vector<const edm4hep::SimCalorimeterHitCollection*>& simCaloHits) const {
  const auto seed = m_uidSvc->getUniqueID(headers[0].getEventNumber(), headers[0].getRunNumber(), this->name());
  auto rng_engine = std::mt19937(seed);

  // Output collections
  auto oparticles = edm4hep::MCParticleCollection();
  auto osimTrackerHits = std::vector<edm4hep::SimTrackerHitCollection>();
  auto osimCaloHits = std::vector<edm4hep::SimCalorimeterHitCollection>();
  auto ocaloHitContribs = std::vector<edm4hep::CaloHitContributionCollection>();
  for (size_t i = 0; i < simCaloHits.size(); ++i) {
    ocaloHitContribs.emplace_back(edm4hep::CaloHitContributionCollection());
  }

  // Copy MCParticles for physics event into a new collection
  for (const auto&& part : particles) {
    oparticles.push_back(part.clone(false));
  }
  // Fix relations to point to the new particles
  for (size_t i = 0; i < particles.size(); ++i) {
    for (const auto& parent : particles[i].getParents()) {
      oparticles[i].addToParents(oparticles[parent.getObjectID().index]);
    }
    for (const auto& daughter : particles[i].getDaughters()) {
      oparticles[i].addToDaughters(oparticles[daughter.getObjectID().index]);
    }
  }

  // Copy the SimTrackerHits and crop them
  for (size_t i = 0; i < simTrackerHits.size(); ++i) {
    const auto& coll = simTrackerHits[i];
    const auto name = inputLocations(SIMTRACKERHIT_INDEX_POSITION)[i];
    const auto [this_start, this_stop] = define_time_windows(name);
    auto ocoll = edm4hep::SimTrackerHitCollection();
    for (const auto&& simTrackerHit : *coll) {
      const float tof = time_of_flight(simTrackerHit.getPosition());
      if ((simTrackerHit.getTime() > this_start + tof) && (simTrackerHit.getTime() < this_stop + tof)) {
        auto nhit = simTrackerHit.clone(false);
        if (simTrackerHit.getParticle().getObjectID().index != -1)
          nhit.setParticle(oparticles[simTrackerHit.getParticle().getObjectID().index]);
        ocoll.push_back(nhit);
      }
    }
    osimTrackerHits.emplace_back(std::move(ocoll));
  }

  // Copy the SimCalorimeterHits and crop them together with the contributions
  std::map<int, std::map<uint64_t, edm4hep::MutableSimCalorimeterHit>> cellIDsMap;
  for (size_t i = 0; i < simCaloHits.size(); ++i) {
    const auto& coll = simCaloHits[i];
    const auto name = inputLocations(SIMCALOHIT_INDEX_POSITION)[i];
    const auto [this_start, this_stop] = define_time_windows(name);
    auto& calHitMap = cellIDsMap[i];
    auto& caloHitContribs = ocaloHitContribs[i];
    for (const auto&& simCaloHit : *coll) {
      const float tof = time_of_flight(simCaloHit.getPosition());
      bool within_time_window = false;
      std::vector<int> thisContribs;
      for (const auto& contrib : simCaloHit.getContributions()) {
        if (!((contrib.getTime() > this_start + tof) && (contrib.getTime() < this_stop + tof)))
          continue;
        within_time_window = true;
        // TODO: Make sure a contribution is not added twice
        auto newContrib = contrib.clone(false);
        newContrib.setParticle(oparticles[contrib.getParticle().getObjectID().index]);
        thisContribs.push_back(caloHitContribs.size());
        caloHitContribs.push_back(std::move(newContrib));
      }
      if (within_time_window) {
        auto newhit = simCaloHit.clone(false);
        for (const auto& contrib : thisContribs) {
          newhit.addToContributions(caloHitContribs[contrib]);
        }
        calHitMap.emplace(simCaloHit.getCellID(), std::move(newhit));
      }
    }
  }

  auto physBX = m_physBX.value();

  // Phase 1: draw all randomness and build the ordered list of background reads.
  // Building it up front keeps the RNG sequence -- and therefore the result --
  // identical whether the reads are later executed serially or in parallel.
  struct BkgRead {
    int    group;
    int    fileIndex;
    size_t entry;
    float  timeOffset;
    int    bxNumber;
    int    physBX;
  };
  std::vector<BkgRead> reads;

  // Iterate over each group of files and parameters
  for (size_t groupIndex = 0; groupIndex < m_bkgEvents->size(); groupIndex++) {
    if (m_randomBX) {
      physBX = std::uniform_int_distribution<int>(0, m_NBunchTrain - 1)(rng_engine);
      debug() << "Physics Event was placed in the " << physBX << " bunch crossing!" << endmsg;
    }

    // define a permutation for the events to overlay -- the physics event is per definition at position 0
    std::vector<int> permutation;

    // Permutation has negative values and the last one is 0
    // if (!m_randomBX) then physBX (default = 1)
    for (int i = -(physBX - 1); i < m_NBunchTrain - (physBX - 1); ++i) {
      permutation.push_back(i);
    }
    std::shuffle(permutation.begin(), permutation.end(), rng_engine);

    // In random-mix mode pick which files of the group to read from, at random.
    // In sequential mode the file index is ignored, so this stays trivial.
    std::vector<int> fileIndices(m_bkgEvents->m_fileNames[groupIndex].size());
    std::iota(fileIndices.begin(), fileIndices.end(), 0);
    if (m_randomMix) {
      std::shuffle(fileIndices.begin(), fileIndices.end(), rng_engine);
    }

    // TODO: Check that there is anything to overlay

    debug() << "Starting overlay for the background group " << groupIndex << endmsg;

    if (m_startWithBackgroundEvent >= 0) {
      info() << "Skipping to event: " << m_startWithBackgroundEvent << endmsg;
      for (auto& entry : m_bkgEvents->m_nextEntry[groupIndex]) {
        entry = m_startWithBackgroundEvent;
      }
    }

    // Overlay the background events to each bunchcrossing in the bunch train
    for (int bxInTrain = 0; bxInTrain < m_NBunchTrain; ++bxInTrain) {
      const int BX_number_in_train = permutation.at(bxInTrain);

      int NOverlay_to_this_BX = 0;

      if (m_Poisson[groupIndex]) {
        NOverlay_to_this_BX = std::poisson_distribution<>(m_Noverlay[groupIndex])(rng_engine);
      } else {
        NOverlay_to_this_BX = m_Noverlay[groupIndex];
      }

      debug() << "Will overlay " << NOverlay_to_this_BX << " events to BX number " << BX_number_in_train + physBX
              << endmsg;

      if (m_randomMix && fileIndices.empty()) {
        warning() << "No background files available for group " << groupIndex << ", skipping overlay" << endmsg;
        continue;
      }
      const float timeOffset = BX_number_in_train * m_deltaT;
      for (int k = 0; k < NOverlay_to_this_BX; ++k) {
        // In random-mix mode pick a random file of the group; in sequential mode
        // the file index is ignored. reserve() advances the per-file cursor now
        // (serially); the actual ROOT read happens in phase 2, possibly on
        // several threads.
        const int    fileIndex = m_randomMix ? fileIndices[k % fileIndices.size()] : 0;
        const size_t entry     = m_bkgEvents->reserve(groupIndex, fileIndex);
        reads.push_back({static_cast<int>(groupIndex), fileIndex, entry, timeOffset, BX_number_in_train, physBX});
      }
    }
  }

  // Phase 2 + 3: read (and decompress) the background frames -- optionally on
  // several threads -- and merge them into the outputs in list order. The merge
  // is always serial and in-order, so the result is independent of OverlayThreads.
  if (m_overlayThreads <= 1) {
    for (const auto& r : reads) {
      const auto backgroundEvent = m_bkgEvents->readAt(r.group, r.fileIndex, r.entry);
      mergeBackgroundFrame(backgroundEvent, r.timeOffset, r.bxNumber, r.physBX, simTrackerHits, simCaloHits, oparticles,
                           osimTrackerHits, cellIDsMap, ocaloHitContribs);
    }
  } else {
    // Parallel read + decompress (bounded by the number of in-flight tokens),
    // serial in-order merge.
    struct Item {
      const BkgRead* read = nullptr;
      podio::Frame   frame;
    };
    std::atomic<size_t> nextRead{0};
    const size_t        ntokens = static_cast<size_t>(m_overlayThreads.value());
    tbb::parallel_pipeline(
        ntokens,
        tbb::make_filter<void, std::shared_ptr<Item>>(
            tbb::filter_mode::serial_in_order,
            [&](tbb::flow_control& fc) -> std::shared_ptr<Item> {
              const size_t idx = nextRead++;
              if (idx >= reads.size()) {
                fc.stop();
                return {};
              }
              return std::make_shared<Item>(Item{&reads[idx], {}});
            }) &
            tbb::make_filter<std::shared_ptr<Item>, std::shared_ptr<Item>>(
                tbb::filter_mode::parallel,
                [&](std::shared_ptr<Item> item) -> std::shared_ptr<Item> {
                  const auto& r = *item->read;
                  item->frame   = m_bkgEvents->readAt(r.group, r.fileIndex, r.entry);
                  // Force decompression/materialization here (in parallel) so the
                  // serial merge only touches already in-memory data.
                  for (const auto& name : item->frame.getAvailableCollections()) {
                    item->frame.get(name);
                  }
                  return item;
                }) &
            tbb::make_filter<std::shared_ptr<Item>, void>(
                tbb::filter_mode::serial_in_order, [&](std::shared_ptr<Item> item) {
                  const auto& r = *item->read;
                  mergeBackgroundFrame(item->frame, r.timeOffset, r.bxNumber, r.physBX, simTrackerHits, simCaloHits,
                                       oparticles, osimTrackerHits, cellIDsMap, ocaloHitContribs);
                }));
  }
  // Move the SimCalorimeterHitCollections to the output vector
  // So far they are stored in a map with the cellID as key
  // but they don't belong to any collection yet
  for (const auto& [index, calHitMap] : cellIDsMap) {
    auto ocoll = edm4hep::SimCalorimeterHitCollection();
    for (const auto& [cellID, hit] : calHitMap) {
      ocoll.push_back(std::move(hit));
    }
    osimCaloHits.emplace_back(std::move(ocoll));
  }

  return std::make_tuple(std::move(oparticles), std::move(osimTrackerHits), std::move(osimCaloHits),
                         std::move(ocaloHitContribs));
}

StatusCode OverlayTiming::finalize() {
  if (m_copyCellIDMetadata) {
    for (const auto& [input, output] :
         {std::make_pair(inputLocations("SimTrackerHits"), outputLocations("OutputSimTrackerHits")),
          std::make_pair(inputLocations("SimCalorimeterHits"), outputLocations("OutputSimCalorimeterHits"))}) {
      for (size_t i = 0; i < input.size(); ++i) {
        const auto value = k4FWCore::getParameter<std::string>(
            podio::collMetadataParamName(input[i], edm4hep::labels::CellIDEncoding), this);
        if (value.has_value()) {
          k4FWCore::putParameter(podio::collMetadataParamName(output[i], edm4hep::labels::CellIDEncoding),
                                 value.value(), this);
        } else {
          warning() << "No metadata found for " << input[i] << " when copying CellID metadata was requested" << endmsg;
        }
      }
    }
  }

  return Gaudi::Algorithm::finalize();
}

DECLARE_COMPONENT(OverlayTiming)
