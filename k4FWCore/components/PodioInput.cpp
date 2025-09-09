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

#include "k4FWCore/PodioDataSvc.h"

#include "edm4hep/edm4hep.h"

#include "podio/UserDataCollection.h"

DECLARE_COMPONENT(PodioInput)

template <typename T>
inline void PodioInput::maybeRead(std::string_view collName) const {
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
  m_readers["edm4hep::RecoMCParticleLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::RecoMCParticleLinkCollection>(collName);
  };
  m_readers["edm4hep::CaloHitSimCaloHitLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::CaloHitSimCaloHitLinkCollection>(collName);
  };
  m_readers["edm4hep::TrackerHitSimTrackerHitLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackerHitSimTrackerHitLinkCollection>(collName);
  };
  m_readers["edm4hep::CaloHitMCParticleLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::CaloHitMCParticleLinkCollection>(collName);
  };
  m_readers["edm4hep::ClusterMCParticleLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::ClusterMCParticleLinkCollection>(collName);
  };
  m_readers["edm4hep::TrackMCParticleLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::TrackMCParticleLinkCollection>(collName);
  };
  m_readers["edm4hep::VertexRecoParticleLinkCollection"] = [&](std::string_view collName) {
    maybeRead<edm4hep::VertexRecoParticleLinkCollection>(collName);
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
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(evtSvc().get());
  if (!m_podioDataSvc) {
    error() << "Could not get PodioDataSvc" << endmsg;
  }
  fillReaders();
}

StatusCode PodioInput::initialize() {
  warning() << "PodioInput is deprecated and will be removed. Use the IOSvc instead" << endmsg;
  // If someone uses the collections property from the command line and passes
  // an empty string we assume they want all collections (as a simple way to
  // override whatever is in the options file)
  if (m_collectionNames.size() == 1 && m_collectionNames[0].empty()) {
    m_collectionNames.clear();
  }

  debug() << "Setting collections to read to: " << m_collectionNames.value() << endmsg;
  m_podioDataSvc->setCollsToRead(m_collectionNames);

  return StatusCode::SUCCESS;
}

StatusCode PodioInput::finalize() {
  warning() << "PodioInput is deprecated and will be removed. Use the IOSvc instead" << endmsg;
  return StatusCode::SUCCESS;
}

void PodioInput::operator()() const {
  if (m_podioDataSvc->getEventFrame().get(edm4hep::labels::EventHeader)) {
    m_readers[edm4hep::EventHeaderCollection::typeName](edm4hep::labels::EventHeader);
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
