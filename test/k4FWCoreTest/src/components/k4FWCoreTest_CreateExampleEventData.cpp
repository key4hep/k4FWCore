#include "k4FWCoreTest_CreateExampleEventData.h"

#include "podio/UserDataCollection.h"

// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"

DECLARE_COMPONENT(k4FWCoreTest_CreateExampleEventData)

k4FWCoreTest_CreateExampleEventData::k4FWCoreTest_CreateExampleEventData(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("simtrackhits", m_simTrackerHitHandle, "Dummy Hit collection (output)");
  declareProperty("trackhits", m_TrackerHitHandle, "Dummy Hit collection (output)");
  declareProperty("tracks", m_trackHandle, "Dummy track collection (output)");
  declareProperty("vectorfloat", m_vectorFloatHandle, "Dummy collection (output)");
}

k4FWCoreTest_CreateExampleEventData::~k4FWCoreTest_CreateExampleEventData() {}

StatusCode k4FWCoreTest_CreateExampleEventData::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_CreateExampleEventData::execute() {
  auto* floatVector = m_vectorFloatHandle.createAndPut();
  floatVector->push_back(125.);
  floatVector->push_back(25.);
  floatVector->push_back(m_event);

  edm4hep::MCParticleCollection* particles = m_mcParticleHandle.createAndPut();

  auto particle = particles->create();

  auto& p4 = particle.momentum();
  p4.x     = m_magicNumberOffset + m_event + 5;
  p4.y     = m_magicNumberOffset + 6;
  p4.z     = m_magicNumberOffset + 7;
  particle.setMass(m_magicNumberOffset + m_event + 8);

  edm4hep::SimTrackerHitCollection* simTrackerHits = m_simTrackerHitHandle.createAndPut();
  auto                              hit            = simTrackerHits->create();
  hit.setPosition({3, 4, 5});

  edm4hep::TrackerHitCollection* trackerHits = m_TrackerHitHandle.createAndPut();
  auto                           trackerHit  = trackerHits->create();
  trackerHit.setPosition({3, 4, 5});

  edm4hep::TrackCollection* tracks = m_trackHandle.createAndPut();
  auto                      track  = tracks->create();
  auto                      track2 = tracks->create();
  // set members
  track.setType(1);
  track.setChi2(2.1);
  track.setNdf(3);
  track.setDEdx(4.1);
  track.setDEdxError(5.1);
  track.setRadiusOfInnermostHit(6.1);
  // set vectormembers
  track.addToSubDetectorHitNumbers(1);
  track.addToSubDetectorHitNumbers(4);
  track.addToTrackStates(edm4hep::TrackState());
  // set associatons
  track.addToTrackerHits(trackerHit);
  track.addToTracks(track2);

  m_event++;

  return StatusCode::SUCCESS;
}

StatusCode k4FWCoreTest_CreateExampleEventData::finalize() { return GaudiAlgorithm::finalize(); }
