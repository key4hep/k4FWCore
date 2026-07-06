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

/** Background overlay algorithm

    This algorithm overlays background events on top of the signal events. The
    background events are read from sets of input files, and the signal events
    are the input in the main event loop.

    The MCParticleCollection in signal are background are overlaid into one
    collection. The SimTrackerHit collections are cropped and overlayed if they
    are in the time window. The SimCalorimeterHit collections are overlayed
    based on the cellID. If a signal hit has the same cellID as a background
    hit, they are combined into a single hit. Only hits that have
    CaloHitContributions in the time range are considered.

**/

#include "podio/Frame.h"
#include "podio/Reader.h"

#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

#include "k4FWCore/Transformer.h"
#include "k4Interface/IUniqueIDGenSvc.h"

#include "GaudiKernel/GaudiException.h"

// Needed for some of the more complex properties
#include "Gaudi/Parsers/Factory.h"
#include "Gaudi/Property.h"

#include <condition_variable>
#include <future>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// Holds the background events and serializes all ROOT I/O on a single worker
// thread. ROOT TFile access is not thread-safe, so routing every read through
// one worker thread makes a shared EventHolder safe when the (const) overlay
// operator() is scheduled concurrently under intra-event multi-threading.
//
// Two source strategies are supported, selected by m_randomMix:
//   * sequential (default): each group is read as one logical stream through a
//     persistent reader, advancing an internal cursor;
//   * random mix: each file in a group is an independent event source, opened
//     on demand, so the caller can pick a random file per overlay.
//
// Bookkeeping is stored per [group][file]. In sequential mode the file
// dimension has a single slot per group.
struct EventHolder {
  std::vector<std::vector<std::string>> m_fileNames;
  bool                                  m_randomMix{false};
  bool                                  m_allowReuse{false};
  std::string                           m_algName;

  // Sequential mode only: one persistent reader per group.
  std::vector<podio::Reader> m_rootFileReaders;

  // [group][file]. In sequential mode the inner vector has a single element.
  // A total of 0 means "not yet determined" (filled lazily in random-mix mode).
  std::vector<std::vector<size_t>> m_totalNumberOfEvents;
  std::vector<std::vector<size_t>> m_nextEntry;

  // Single worker thread serializing all ROOT I/O.
  struct Request {
    int                        groupIndex;
    int                        fileIndex;
    std::promise<podio::Frame> prom;
  };
  std::queue<Request>     m_requests;
  std::mutex              m_queueMutex;
  std::condition_variable m_queueCV;
  std::thread             m_worker;
  bool                    m_stop{false};

  EventHolder(const std::vector<std::vector<std::string>>& fileNames, bool randomMix, bool allowReuse,
              const std::string& algName)
      : m_fileNames(fileNames), m_randomMix(randomMix), m_allowReuse(allowReuse), m_algName(algName) {
    m_totalNumberOfEvents.resize(m_fileNames.size());
    m_nextEntry.resize(m_fileNames.size());
    for (size_t group = 0; group < m_fileNames.size(); ++group) {
      if (m_randomMix) {
        // One independent event source per file; counts are determined lazily.
        m_totalNumberOfEvents[group].resize(m_fileNames[group].size(), 0);
        m_nextEntry[group].resize(m_fileNames[group].size(), 0);
      } else {
        // The whole group is read as a single logical stream.
        m_rootFileReaders.emplace_back(podio::makeReader(m_fileNames[group]));
        m_totalNumberOfEvents[group].push_back(m_rootFileReaders.back().getEntries("events"));
        m_nextEntry[group].push_back(0);
      }
    }

    m_worker = std::thread([this]() {
      while (true) {
        Request req;
        {
          std::unique_lock<std::mutex> lock(m_queueMutex);
          m_queueCV.wait(lock, [this]() { return m_stop || !m_requests.empty(); });
          if (m_stop && m_requests.empty()) {
            return;
          }
          req = std::move(m_requests.front());
          m_requests.pop();
        }
        try {
          req.prom.set_value(read(req.groupIndex, req.fileIndex));
        } catch (...) {
          req.prom.set_exception(std::current_exception());
        }
      }
    });
  }
  EventHolder() = default;

  ~EventHolder() {
    {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_stop = true;
    }
    m_queueCV.notify_all();
    if (m_worker.joinable()) {
      m_worker.join();
    }
  }

  // Thread-safe: enqueue a read request and block until the worker fulfills it.
  // In sequential mode fileIndex is ignored (always slot 0).
  podio::Frame getFrame(int groupIndex, int fileIndex) {
    Request req{groupIndex, m_randomMix ? fileIndex : 0, std::promise<podio::Frame>()};
    auto     fut = req.prom.get_future();
    {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_requests.push(std::move(req));
    }
    m_queueCV.notify_one();
    return fut.get();
  }

  size_t size() const { return m_fileNames.size(); }

private:
  // Runs exclusively on the worker thread, so cursor bookkeeping needs no extra
  // locking.
  podio::Frame read(int group, int file) {
    size_t& total = m_totalNumberOfEvents[group][file];
    size_t& entry = m_nextEntry[group][file];

    podio::Reader* reader = nullptr;
    std::optional<podio::Reader> ondemand;
    if (m_randomMix) {
      ondemand.emplace(podio::makeReader(m_fileNames[group][file]));
      reader = &ondemand.value();
      if (total == 0) {
        total = reader->getEntries("events");
      }
    } else {
      reader = &m_rootFileReaders[group];
    }

    if (total == 0) {
      throw GaudiException("No events found in background file " + m_fileNames[group][file], m_algName,
                           StatusCode::FAILURE);
    }
    if (entry >= total && !m_allowReuse) {
      throw GaudiException("No more events in background file", m_algName, StatusCode::FAILURE);
    }
    podio::Frame frame = reader->readEvent(entry % total);
    entry              = (entry + 1) % total;
    return frame;
  }
};

using retType =
    std::tuple<edm4hep::MCParticleCollection, std::vector<edm4hep::SimTrackerHitCollection>,
               std::vector<edm4hep::SimCalorimeterHitCollection>, std::vector<edm4hep::CaloHitContributionCollection>>;

struct OverlayTiming : public k4FWCore::MultiTransformer<retType(
                           const edm4hep::EventHeaderCollection& headers, const edm4hep::MCParticleCollection&,
                           const std::vector<const edm4hep::SimTrackerHitCollection*>&,
                           const std::vector<const edm4hep::SimCalorimeterHitCollection*>&)> {
  OverlayTiming(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(name, svcLoc,
                         {KeyValue("EventHeader", {"EventHeader"}), KeyValue("MCParticles", "DefaultMCParticles"),
                          KeyValues("SimTrackerHits", {"DefaultSimTrackerHits"}),
                          KeyValues("SimCalorimeterHits", {"DefaultSimCalorimeterHits"})},
                         {KeyValue("OutputMCParticles", "NewMCParticles"),
                          KeyValues("OutputSimTrackerHits", {"MCParticles1"}),
                          KeyValues("OutputSimCalorimeterHits", {"MCParticles2"}),
                          KeyValues("OutputCaloHitContributions", {"OverlayCaloHitContributions"})}) {}

  template <typename T>
  void overlayCollection(std::string collName, const podio::CollectionBase& inColl);

  virtual StatusCode initialize() final;
  virtual StatusCode finalize() final;

  retType virtual
  operator()(const edm4hep::EventHeaderCollection& headers, const edm4hep::MCParticleCollection& mcParticles,
             const std::vector<const edm4hep::SimTrackerHitCollection*>& simTrackerHits,
             const std::vector<const edm4hep::SimCalorimeterHitCollection*>& simCalorimeterHits) const final;

  std::pair<float, float> define_time_windows(const std::string& Collection_name) const;

private:
  // These correspond to the index position in the argument list
  constexpr static int SIMTRACKERHIT_INDEX_POSITION = 2;
  constexpr static int SIMCALOHIT_INDEX_POSITION = 3;

  Gaudi::Property<bool> m_randomBX{this, "RandomBx", false,
                                   "If true, place the physics event at a random bunch crossing (overrides PhysicsBX)"};
  Gaudi::Property<int> m_physBX{this, "PhysicsBX", 1, "Position of the physics event within the bunch train"};
  Gaudi::Property<int> m_NBunchTrain{this, "NBunchtrain", 1, "Number of bunch crossings in the bunch train"};
  // Gaudi::Property<int>         m_startWithBackgroundFile{this, "StartBackgroundFileIndex", -1,
  //                                                "Which background file to startWith"};
  Gaudi::Property<int> m_startWithBackgroundEvent{this, "StartBackgroundEventIndex", -1,
                                                  "Index of the background event to start from (-1 means start from "
                                                  "the beginning)"};

  Gaudi::Property<std::vector<std::vector<std::string>>> m_inputFileNames{
      this, "BackgroundFileNames", {}, "List of groups of background input files, one group per overlay stream"};

  Gaudi::Property<std::vector<double>> m_Noverlay{
      this, "NumberBackground", {}, "Number of background events to overlay per stream (fixed or Poisson mean)"};

  Gaudi::Property<std::vector<bool>> m_Poisson{
      this,
      "Poisson_random_NOverlay",
      {},
      "If true, draw the number of events from a Poisson distribution with mean NumberBackground"};

  Gaudi::Property<std::string> m_MCParticleCollectionName{this, "BackgroundMCParticleCollectionName", "MCParticle",
                                                          "Name of the MCParticle collection in the background files"};

  Gaudi::Property<float> m_deltaT{this, "Delta_t", float(0.5), "Time between consecutive bunch crossings (ns)"};

  mutable std::unique_ptr<EventHolder> m_bkgEvents{};

  Gaudi::Property<std::map<std::string, std::vector<float>>> m_timeWindows{
      this, "TimeWindows", std::map<std::string, std::vector<float>>(),
      "Map from collection name to [t_min, t_max] (ns) defining the acceptance window. Required for every "
      "SimTrackerHit and SimCalorimeterHit collection."};
  Gaudi::Property<bool> m_allowReusingBackgroundFiles{
      this, "AllowReusingBackgroundFiles", false, "If true, wrap around the background file when events are exhausted"};
  Gaudi::Property<bool> m_copyCellIDMetadata{this, "CopyCellIDMetadata", false,
                                             "Copy cell ID encoding metadata from input to output collections"};

  Gaudi::Property<bool> m_randomMix{
      this, "RandomMixBackgroundFiles", false,
      "Treat each file in a background group as an independent (pseudo-)event source and pick a random file for every "
      "overlaid event (one-event-per-file mixing). Entries of BackgroundFileNames may also be directories, "
      "whose .root files are used."};

  Gaudi::Property<bool> m_mergeMCParticles{
      this, "MergeMCParticles", true,
      "Merge the background MCParticle collection into the output. If false, background particles are not "
      "stored: tracker hits keep the momentum of their originating particle instead of a particle link, and "
      "calorimeter contributions get an empty particle."};

  // Gaudi::Property<int> m_maxCachedFrames{
  //   this, "MaxCachedFrames", 0, "Maximum number of frames cached from background files"};

private:
  SmartIF<IUniqueIDGenSvc> m_uidSvc;
};
