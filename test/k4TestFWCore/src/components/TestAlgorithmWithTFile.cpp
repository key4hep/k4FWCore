#include "TestAlgorithmWithTFile.h"


// datamodel
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"


DECLARE_COMPONENT(TestAlgorithmWithTFile)

TestAlgorithmWithTFile::TestAlgorithmWithTFile(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("mcparticles", m_mcParticleHandle, "Dummy Particle collection (output)");
  declareProperty("trackhits", m_simTrackerHitHandle, "Dummy Hit collection (output)");
}

TestAlgorithmWithTFile::~TestAlgorithmWithTFile() {}

StatusCode TestAlgorithmWithTFile::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  /// for testing
  m_value = 0.1;
  m_file = TFile::Open("tmp.root", "RECREATE");
  m_tree = new TTree("mytree", "testing");

  m_tree->Branch("value", &m_value, "value/F");

  return StatusCode::SUCCESS;
}

StatusCode TestAlgorithmWithTFile::execute() {

  m_singleIntHandle.put(new int(12345));

  std::vector<float>* floatVector = m_vectorFloatHandle.createAndPut();
  floatVector->emplace_back( 125.);
  floatVector->emplace_back( 25.);

  edm4hep::MCParticleCollection* particles = m_mcParticleHandle.createAndPut();

  auto particle = particles->create();

  auto& p4 = particle.momentum();
  p4.x = m_magicNumberOffset + 5; 
  p4.y = m_magicNumberOffset + 6;
  p4.z = m_magicNumberOffset + 7;
  particle.setMass(m_magicNumberOffset + 8);

  auto* hits = m_simTrackerHitHandle.createAndPut();
  auto hit = hits->create();
  hit.setPosition({3, 4, 5});

  /// for testing
  m_value += 1.1;
  m_tree->Fill();

  return StatusCode::SUCCESS;
}

StatusCode TestAlgorithmWithTFile::finalize() {
  /// for testing
  if ( true ) {
      /// m_tree will be wrote to the podio output file, but not m_file
      m_file->cd();
      m_tree->Write();
      m_file->Close();
  }
  else {
      /// this is OK when there is not too many entries, and no auto-flush has been invoked
      /// otherwise there will be a crash
      const char* preDir = gDirectory->GetPath();
      m_file->cd();
      m_tree->Write();
      m_file->Close();
      gDirectory->cd(preDir);
  }

  return GaudiAlgorithm::finalize();
}
