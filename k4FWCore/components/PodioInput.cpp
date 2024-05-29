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
#include "PodioInput.h"
#include "Gaudi/Functional/Consumer.h"

#include "k4FWCore/k4DataSvc.h"

#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/Constants.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoClusterParticleAssociationCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackerHitPlaneAssociationCollection.h"
#include "edm4hep/ParticleIDCollection.h"
#include "edm4hep/RawCalorimeterHitCollection.h"
#include "edm4hep/RawTimeSeriesCollection.h"
#include "edm4hep/RecDqdxCollection.h"
#include "edm4hep/RecIonizationClusterCollection.h"
#include "edm4hep/RecoParticleVertexAssociationCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/SimPrimaryIonizationClusterCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TimeSeriesCollection.h"
#include "edm4hep/TrackCollection.h"
#if __has_include("edm4hep/TrackerHit3DCollection.h")
#include "edm4hep/TrackerHit3DCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
namespace edm4hep {
  using TrackerHit3DCollection = edm4hep::TrackerHitCollection;
}  // namespace edm4hep
#endif
#include "edm4hep/TrackerHitPlaneCollection.h"
#include "edm4hep/TrackerPulseCollection.h"
#include "edm4hep/VertexCollection.h"

#include "podio/UserDataCollection.h"

DECLARE_COMPONENT(PodioInput)

template <typename T> inline void PodioInput::maybeRead(std::string_view collName) const {
  if (m_podioDataSvc->readCollection<T>(std::string(collName)).isFailure()) {
    error() << "Failed to register collection " << collName << endmsg;
  }
}

void PodioInput::fillReaders() {
  m_readers["edm4hep::MCParticleCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCParticleCollection>(collName);
  };
  m_readers["edm4hep::SimTrackerHitCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::SimTrackerHitCollection>(collName);
  };
  m_readers["edm4hep::CaloHitContributionCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::CaloHitContributionCollection>(collName);
  };
  m_readers["edm4hep::SimCalorimeterHitCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::SimCalorimeterHitCollection>(collName);
  };
  m_readers["edm4hep::RawCalorimeterHitCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RawCalorimeterHitCollection>(collName);
  };
  m_readers["edm4hep::CalorimeterHitCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::CalorimeterHitCollection>(collName);
  };
  m_readers["edm4hep::ParticleIDCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::ParticleIDCollection>(collName);
  };
  m_readers["edm4hep::ClusterCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::ClusterCollection>(collName);
  };
  m_readers["edm4hep::TrackerHit3DCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackerHit3DCollection>(collName);
  };
  m_readers["edm4hep::TrackerHitCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackerHit3DCollection>(collName);
  };
  m_readers["edm4hep::TrackerHitPlaneCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackerHitPlaneCollection>(collName);
  };
  m_readers["edm4hep::RawTimeSeriesCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RawTimeSeriesCollection>(collName);
  };
  m_readers["edm4hep::TrackCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackCollection>(collName);
  };
  m_readers["edm4hep::VertexCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::VertexCollection>(collName);
  };
  m_readers["edm4hep::ReconstructedParticleCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::ReconstructedParticleCollection>(collName);
  };
  m_readers["edm4hep::MCRecoParticleAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoParticleAssociationCollection>(collName);
  };
  m_readers["edm4hep::MCRecoCaloAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoCaloAssociationCollection>(collName);
  };
  m_readers["edm4hep::MCRecoTrackerAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoTrackerAssociationCollection>(collName);
  };
  m_readers["edm4hep::MCRecoTrackerHitPlaneAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoTrackerHitPlaneAssociationCollection>(collName);
  };
  m_readers["edm4hep::MCRecoClusterParticleAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoClusterParticleAssociationCollection>(collName);
  };
  m_readers["edm4hep::MCRecoTrackParticleAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::MCRecoTrackParticleAssociationCollection>(collName);
  };
  m_readers["edm4hep::RecoParticleVertexAssociationCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RecoParticleVertexAssociationCollection>(collName);
  };
  m_readers["edm4hep::SimPrimaryIonizationClusterCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::SimPrimaryIonizationClusterCollection>(collName);
  };
  m_readers["edm4hep::TrackerPulseCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackerPulseCollection>(collName);
  };
  m_readers["edm4hep::RecIonizationClusterCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RecIonizationClusterCollection>(collName);
  };
  m_readers["edm4hep::TimeSeriesCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TimeSeriesCollection>(collName);
  };
  m_readers["edm4hep::RecDqdxCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RecDqdxCollection>(collName);
  };
  m_readers["podio::UserDataCollection<int>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<int>>(collName);
  };
  m_readers["podio::UserDataCollection<float>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<float>>(collName);
  };
  m_readers["podio::UserDataCollection<double>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<double>>(collName);
  };
  m_readers["podio::UserDataCollection<int8_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<int8_t>>(collName);
  };
  m_readers["podio::UserDataCollection<int16_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<int16_t>>(collName);
  };
  m_readers["podio::UserDataCollection<int32_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<int32_t>>(collName);
  };
  m_readers["podio::UserDataCollection<int64_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<int64_t>>(collName);
  };
  m_readers["podio::UserDataCollection<uint8_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<uint8_t>>(collName);
  };
  m_readers["podio::UserDataCollection<uint16_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<uint16_t>>(collName);
  };
  m_readers["podio::UserDataCollection<uint32_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<uint32_t>>(collName);
  };
  m_readers["podio::UserDataCollection<uint64_t>"] = [&](std::string_view collName) {
    maybeRead<podio::UserDataCollection<uint64_t>>(collName);
  };
  m_readers["edm4hep::EventHeaderCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::EventHeaderCollection>(collName);
  };
}

PodioInput::PodioInput(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {
  // do not do anything during the genconf step
  const std::string cmd = System::cmdLineArgs()[0];
  if (cmd.find("genconf") != std::string::npos)
    return;

  // check whether we have the PodioEvtSvc active
  m_podioDataSvc = dynamic_cast<k4DataSvc*>(evtSvc().get());
  if (!m_podioDataSvc) {
    error() << "Could not get k4DataSvc" << endmsg;
  }
  fillReaders();
}

StatusCode PodioInput::initialize() {
  // If someone uses the collections property from the command line and passes
  // an empty string we assume they want all collections (as a simple way to
  // override whatever is in the options file)
  if (m_collectionNames.size() == 1 && m_collectionNames[0].empty()) {
    m_collectionNames.clear();
  }

  return StatusCode::SUCCESS;
}

void PodioInput::operator()() const {
  if (m_podioDataSvc->getEventFrame().get(edm4hep::EventHeaderName)) {
    m_readers[edm4hep::EventHeaderCollection::typeName](edm4hep::EventHeaderName);
  } else {
    info() << "No EventHeader collection found in the event. Not reading it" << endmsg;
  }

  const auto& collsToRead = [&]() {
    if (m_collectionNames.empty()) {
      return m_podioDataSvc->getEventFrame().getAvailableCollections();
    } else {
      return m_collectionNames.value();
    }
  }();

  for (const auto& collName : collsToRead) {
    debug() << "Registering collection to read " << collName << endmsg;
    if (!m_podioDataSvc->getEventFrame().get(collName)) {
      warning() << "Collection " << collName << " is not available from file." << endmsg;
      continue;
    }
    auto type = m_podioDataSvc->getCollectionType(collName);
    if (m_readers.find(type) != m_readers.end()) {
      m_readers[type](collName);
    } else {
      maybeRead<podio::CollectionBase>(collName);
    }
  }

  // Tell data service that we are done with requested collections
  m_podioDataSvc->endOfRead();
}
