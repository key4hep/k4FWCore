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
#include "k4FWCoreTest_CreateExampleEventData.h"

#include "podio/UserDataCollection.h"

// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#if __has_include("edm4hep/EDM4hepVersion.h")
#include "edm4hep/EDM4hepVersion.h"
#else
// Copy the necessary parts from the header above to make whatever we need to work here
#define EDM4HEP_VERSION(major, minor, patch) ((UINT64_C(major) << 32) | (UINT64_C(minor) << 16) | (UINT64_C(patch)))
// v00-09 is the last version without the capitalization change of the track vector members
#define EDM4HEP_BUILD_VERSION EDM4HEP_VERSION(0, 9, 0)
#endif

DECLARE_COMPONENT(k4FWCoreTest_CreateExampleEventData)

k4FWCoreTest_CreateExampleEventData::k4FWCoreTest_CreateExampleEventData(const std::string& aName, ISvcLocator* aSvcLoc)
    : Gaudi::Algorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("simtrackhits", m_simTrackerHitHandle, "Dummy Hit collection (output)");
  declareProperty("trackhits", m_TrackerHitHandle, "Dummy Hit collection (output)");
  declareProperty("tracks", m_trackHandle, "Dummy track collection (output)");
  declareProperty("vectorfloat", m_vectorFloatHandle, "Dummy collection (output)");
  declareProperty("recoparticles", m_recoHandle, "Dummy collection (output)");
  declareProperty("links", m_linkHandle, "Dummy collection (output)");
  // Set Cardinality to 1 because this algorithm is not prepared to run in parallel
  setProperty("Cardinality", 1).ignore();
}

StatusCode k4FWCoreTest_CreateExampleEventData::execute(const EventContext&) const {
  auto* floatVector = m_vectorFloatHandle.createAndPut();
  floatVector->push_back(125.);
  floatVector->push_back(25.);
  floatVector->push_back(m_event);

  edm4hep::MCParticleCollection* particles = m_mcParticleHandle.createAndPut();

  auto particle = particles->create();
  particle.setPDG(1);
  particle.setGeneratorStatus(2);
  particle.setSimulatorStatus(3);
  particle.setCharge(4);
  particle.setTime(5);
  particle.setMass(m_magicNumberOffset + 6);
  particle.setMomentum({m_magicNumberOffset + m_event + 0.0, m_magicNumberOffset + 6.0, m_magicNumberOffset + 7.0});

  particles->create(2, 3, 4, 5.f, 6.f, 7.f);

  edm4hep::SimTrackerHitCollection* simTrackerHits = m_simTrackerHitHandle.createAndPut();
  auto hit = simTrackerHits->create();
  hit.setPosition({3, 4, 5});

  edm4hep::TrackerHit3DCollection* trackerHits = m_TrackerHitHandle.createAndPut();
  auto trackerHit = trackerHits->create();
  trackerHit.setPosition({3, 4, 5});

  edm4hep::TrackCollection* tracks = m_trackHandle.createAndPut();
  auto track = tracks->create();
  auto track2 = tracks->create();
  // set members
  track.setType(1);
  track.setChi2(2.1);
  track.setNdf(3);
  // set vectormembers
#if EDM4HEP_BUILD_VERSION > EDM4HEP_VERSION(0, 9, 0)
  track.addToSubdetectorHitNumbers(1);
  track.addToSubdetectorHitNumbers(4);
#else
  track.addToSubDetectorHitNumbers(1);
  track.addToSubDetectorHitNumbers(4);
#endif
  track.addToTrackStates(edm4hep::TrackState());
  // set associatons
  track.addToTrackerHits(trackerHit);
  track.addToTracks(track2);

  m_event++;

  auto* recos = m_recoHandle.createAndPut();
  for (int i = 0; i < 5; ++i) {
    auto reco = recos->create();
    reco.setPDG(i);
  }

  auto links = m_linkHandle.createAndPut();
  for (size_t i = 0; i < 2; ++i) {
    auto link = links->create();
    link.setFrom((*recos)[i]);
    link.setTo((*particles)[i]);
  }

  return StatusCode::SUCCESS;
}
