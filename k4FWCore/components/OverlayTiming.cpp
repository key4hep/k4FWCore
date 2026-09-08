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

#include <random>
#include <utility>
#include <vector>

template <typename T>
inline float time_of_flight(const T& pos) {
  // Returns the time of flight to the radius in ns
  // Assumming positions in mm, then mm/m/s = 10^-3 s = 10^6 ns
  return std::sqrt((pos[0] * pos[0]) + (pos[1] * pos[1]) + (pos[2] * pos[2])) / TMath::C() * 1e6;
}

// Index of the copied background particle a relation should point at, or <0 when
// the relation has to be left unset. The background particles are copied in
// order, so the copy of background particle oldIndex sits at offset + oldIndex.
// An index outside the background collection leaves the relation unset, which
// covers both podio::ObjectID::untracked (-1) and podio::ObjectID::invalid (-2).
inline int overlaid_particle_index(int oldIndex, int offset, int nBgParticles) {
  return (oldIndex >= 0 && oldIndex < nBgParticles) ? offset + oldIndex : -1;
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

  std::vector<std::vector<std::string>> inputFiles;
  inputFiles = m_inputFileNames.value();
  // if (m_startWithBackgroundFile >= 0) {
  //   inputFiles = std::vector<std::string>(m_inputFileNames.begin() + m_startWithBackgroundFile,
  //   m_inputFileNames.end());
  // } else {
  //   inputFiles = m_inputFileNames;
  // }
  // TODO:: shuffle input files
  // std::shuffle(inputFiles.begin(), inputFiles.end(), rng_engine);

  m_bkgEvents = make_unique<EventHolder>(inputFiles);
  for (auto& val : m_bkgEvents->m_totalNumberOfEvents) {
    if (val == 0) {
      std::string err = "No events found in the background files";
      for (auto& file : m_inputFileNames.value()) {
        err += " " + file[0];
      }
      error() << err << endmsg;
      return StatusCode::FAILURE;
    }
  }

  if (std::any_of(m_bkgEvents->m_totalNumberOfEvents.begin(), m_bkgEvents->m_totalNumberOfEvents.end(),
                  [this](const int& val) { return this->m_startWithBackgroundEvent >= val; })) {
    throw GaudiException("StartWithBackgroundEvent is larger than the number of events in the background files", name(),
                         StatusCode::FAILURE);
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

  // Copy the cellID encoding of the input collections over to the overlaid output collections.
  if (m_copyCellIDMetadata) {
    for (const auto& [input, output] :
         {std::make_pair(inputLocations("SimTrackerHits"), outputLocations("OutputSimTrackerHits")),
          std::make_pair(inputLocations("SimCalorimeterHits"), outputLocations("OutputSimCalorimeterHits"))}) {
      for (size_t i = 0; i < input.size(); ++i) {
        const auto value = k4FWCore::getCellIDEncoding(input[i], this);
        if (value.has_value()) {
          k4FWCore::putCellIDEncoding(output[i], value.value(), this);
        } else {
          warning() << "No metadata found for " << input[i] << " when copying CellID metadata was requested" << endmsg;
        }
      }
    }
  }

  return StatusCode::SUCCESS;
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
    ocaloHitContribs.emplace_back();
  }

  // Copy MCParticles for physics event into a new collection
  for (const auto&& part : particles) {
    oparticles.push_back(part.clone(false));
  }
  // Fix relations to point to the new particles
  for (size_t i = 0; i < particles.size(); ++i) {
    for (const auto& parent : particles.at(i).getParents()) {
      if (const auto index = parent.getObjectID().index; index >= 0) {
        oparticles.at(i).addToParents(oparticles.at(index));
      }
    }
    for (const auto& daughter : particles.at(i).getDaughters()) {
      if (const auto index = daughter.getObjectID().index; index >= 0) {
        oparticles.at(i).addToDaughters(oparticles.at(index));
      }
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
        if (const auto index = simTrackerHit.getParticle().getObjectID().index; index >= 0) {
          nhit.setParticle(oparticles.at(index));
        }
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
    // operator[] on purpose: this is where the entry for this collection is created
    auto& calHitMap = cellIDsMap[i];
    auto& caloHitContribs = ocaloHitContribs[i];
    for (const auto&& simCaloHit : *coll) {
      const float tof = time_of_flight(simCaloHit.getPosition());
      bool within_time_window = false;
      std::vector<int> thisContribs;
      for (const auto& contrib : simCaloHit.getContributions()) {
        if (!((contrib.getTime() > this_start + tof) && (contrib.getTime() < this_stop + tof))) {
          continue;
        }
        within_time_window = true;
        // TODO: Make sure a contribution is not added twice
        auto newContrib = contrib.clone(false);
        if (const auto index = contrib.getParticle().getObjectID().index; index >= 0) {
          newContrib.setParticle(oparticles.at(index));
        }
        thisContribs.push_back(caloHitContribs.size());
        caloHitContribs.push_back(std::move(newContrib));
      }
      if (within_time_window) {
        auto newhit = simCaloHit.clone(false);
        for (const auto& contrib : thisContribs) {
          newhit.addToContributions(caloHitContribs.at(contrib));
        }
        calHitMap.emplace(simCaloHit.getCellID(), std::move(newhit));
      }
    }
  }

  auto physBX = m_physBX.value();
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

    // TODO: Check that there is anything to overlay

    debug() << "Starting overlay at event: " << m_bkgEvents->m_nextEntry[groupIndex] << " for the background group "
            << groupIndex << endmsg;

    if (m_startWithBackgroundEvent >= 0) {
      info() << "Skipping to event: " << m_startWithBackgroundEvent << endmsg;
      for (auto& entry : m_bkgEvents->m_nextEntry) {
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

      for (int k = 0; k < NOverlay_to_this_BX; ++k) {
        info() << "Overlaying background event " << m_bkgEvents->m_nextEntry[groupIndex] << " from group " << groupIndex
               << " to BX " << bxInTrain << endmsg;
        if (m_bkgEvents->m_nextEntry[groupIndex] >= m_bkgEvents->m_totalNumberOfEvents[groupIndex] &&
            !m_allowReusingBackgroundFiles) {
          throw GaudiException("No more events in background file", name(), StatusCode::FAILURE);
        }
        const auto backgroundEvent =
            m_bkgEvents->m_rootFileReaders[groupIndex].readEvent(m_bkgEvents->m_nextEntry[groupIndex]);
        m_bkgEvents->m_nextEntry[groupIndex]++;
        m_bkgEvents->m_nextEntry[groupIndex] %= m_bkgEvents->m_totalNumberOfEvents[groupIndex];
        const auto availableCollections = backgroundEvent.getAvailableCollections();

        // Either 0 or negative
        const auto timeOffset = BX_number_in_train * m_deltaT;

        if (std::find(availableCollections.begin(), availableCollections.end(), m_MCParticleCollectionName) ==
            availableCollections.end()) {
          warning() << "Collection " << m_MCParticleCollectionName << " not found in background event" << endmsg;
        }

        const auto& bgParticles = backgroundEvent.get<edm4hep::MCParticleCollection>(m_MCParticleCollectionName);
        // The background particles are copied in order, so the copy of background
        // particle i ends up at offset + i and no old-to-new index map is needed.
        const int offset = static_cast<int>(oparticles.size());
        const int nBgParticles = static_cast<int>(bgParticles.size());

        for (int i = 0; i < nBgParticles; ++i) {
          auto npart = bgParticles[i].clone(false);
          npart.setTime(bgParticles[i].getTime() + timeOffset);
          npart.setOverlay(true);
          oparticles.push_back(npart);
        }

        // The relations can only be wired up once every particle has been copied.
        // Relations pointing outside the background collection are left unset.
        for (int i = 0; i < nBgParticles; ++i) {
          for (const auto& parent : bgParticles[i].getParents()) {
            if (const auto index = overlaid_particle_index(parent.getObjectID().index, offset, nBgParticles);
                index >= 0) {
              oparticles.at(offset + i).addToParents(oparticles.at(index));
            }
          }
          for (const auto& daughter : bgParticles[i].getDaughters()) {
            if (const auto index = overlaid_particle_index(daughter.getObjectID().index, offset, nBgParticles);
                index >= 0) {
              oparticles.at(offset + i).addToDaughters(oparticles.at(index));
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
                  (simTrackerHit.getTime() + timeOffset < this_stop + tof))) {
              continue;
            }
            auto nhit = simTrackerHit.clone(false);
            nhit.setOverlay(true);
            nhit.setTime(simTrackerHit.getTime() + timeOffset);
            if (const auto index =
                    overlaid_particle_index(simTrackerHit.getParticle().getObjectID().index, offset, nBgParticles);
                index >= 0) {
              nhit.setParticle(oparticles.at(index));
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

          auto& calHitMap = cellIDsMap[i];
          auto& calHitContribs = ocaloHitContribs[i];
          for (const auto&& simCaloHit : backgroundEvent.get<edm4hep::SimCalorimeterHitCollection>(name)) {
            if (calHitMap.find(simCaloHit.getCellID()) == calHitMap.end()) {
              // There is no hit at this position. The new hit can be added, if it is not outside the window
              auto calhit = edm4hep::MutableSimCalorimeterHit();
              bool add = false;
              for (const auto& contrib : simCaloHit.getContributions()) {
                if ((contrib.getTime() + timeOffset > this_start) && (contrib.getTime() + timeOffset < this_stop)) {
                  add = true;
                  // TODO: Make sure a contribution is not added twice
                  auto newContrib = contrib.clone(false);
                  if (const auto index =
                          overlaid_particle_index(contrib.getParticle().getObjectID().index, offset, nBgParticles);
                      index >= 0) {
                    newContrib.setParticle(oparticles.at(index));
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
                  if (const auto index =
                          overlaid_particle_index(contrib.getParticle().getObjectID().index, offset, nBgParticles);
                      index >= 0) {
                    newContrib.setParticle(oparticles.at(index));
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
    }
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

DECLARE_COMPONENT(OverlayTiming)
