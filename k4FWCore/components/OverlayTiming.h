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

// Needed for some of the more complex properties
#include "Gaudi/Parsers/Factory.h"
#include "Gaudi/Property.h"

#include <map>
#include <string>
#include <vector>

struct EventHolder {
  std::vector<std::vector<std::string>> m_fileNames;
  std::vector<podio::Reader> m_rootFileReaders;
  std::vector<size_t> m_totalNumberOfEvents;
  std::map<int, podio::Frame> m_events;

  std::vector<size_t> m_nextEntry;

  EventHolder(const std::vector<std::vector<std::string>>& fileNames) : m_fileNames(fileNames) {
    for (auto& names : m_fileNames) {
      m_rootFileReaders.emplace_back(podio::makeReader(names));
      m_totalNumberOfEvents.push_back(m_rootFileReaders.back().getEntries("events"));
    }
    m_nextEntry.resize(m_fileNames.size(), 0);
  }
  EventHolder() = default;

  // TODO: Cache functionality
  // podio::Frame& read

  size_t size() const { return m_fileNames.size(); }
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
      this, "NumberBackground", {},
      "Number of background events to overlay per stream (fixed or Poisson mean)"};

  Gaudi::Property<std::vector<bool>> m_Poisson{
      this,
      "Poisson_random_NOverlay",
      {},
      "If true, draw the number of events from a Poisson distribution with mean NumberBackground"};

  Gaudi::Property<std::string> m_MCParticleCollectionName{
      this, "BackgroundMCParticleCollectionName", "MCParticle",
      "Name of the MCParticle collection in the background files"};

  Gaudi::Property<float> m_deltaT{this, "Delta_t", float(0.5), "Time between consecutive bunch crossings (ns)"};

  mutable std::unique_ptr<EventHolder> m_bkgEvents{};

  Gaudi::Property<std::map<std::string, std::vector<float>>> m_timeWindows{
      this, "TimeWindows", std::map<std::string, std::vector<float>>(),
      "Map from collection name to [t_min, t_max] (ns) defining the acceptance window. Required for every "
      "SimTrackerHit and SimCalorimeterHit collection."};
  Gaudi::Property<bool> m_allowReusingBackgroundFiles{
      this, "AllowReusingBackgroundFiles", false,
      "If true, wrap around the background file when events are exhausted"};
  Gaudi::Property<bool> m_copyCellIDMetadata{this, "CopyCellIDMetadata", false,
                                             "Copy cell ID encoding metadata from input to output collections"};

  // Gaudi::Property<int> m_maxCachedFrames{
  //   this, "MaxCachedFrames", 0, "Maximum number of frames cached from background files"};

private:
  SmartIF<IUniqueIDGenSvc> m_uidSvc;
};
