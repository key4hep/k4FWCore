#include "DelphesSimulation.h"

#include "SimDelphes/ParticleStatus.h"
#include <limits>

// Delphes
#include "ExRootAnalysis/ExRootConfReader.h"
#include "ExRootAnalysis/ExRootTask.h"
#include "ExRootAnalysis/ExRootTreeBranch.h"
#include "ExRootAnalysis/ExRootTreeWriter.h"
#include "classes/DelphesClasses.h"
#include "classes/DelphesFactory.h"
#include "classes/DelphesModule.h"
#include "modules/Delphes.h"


#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"
#include "edm4hep/ParticleIDCollection.h"

// ROOT
#include "TFile.h"
#include "TObjArray.h"
#include "TRandom.h"
#include "TStopwatch.h"


#include "DelphesUtils.h"


DECLARE_COMPONENT(DelphesSimulation)

DelphesSimulation::DelphesSimulation(const std::string& name, ISvcLocator* svcLoc) : GaudiAlgorithm(name, svcLoc) {
  declareProperty("MCParticles", m_handleGenParticles);
}

StatusCode DelphesSimulation::initialize() {
  m_confReader = std::unique_ptr<ExRootConfReader>(new ExRootConfReader);
  m_confReader->ReadFile(m_DelphesCard.value().c_str());
  m_Delphes = new Delphes("Delphes");
  m_Delphes->SetConfReader(m_confReader.get());
  m_factory = m_Delphes->GetFactory();
  // Create following arrays of Delphes objects --> starting objects
  m_allParticles = m_Delphes->ExportArray("allParticles");
  m_stableParticles = m_Delphes->ExportArray("stableParticles");
  m_partons = m_Delphes->ExportArray("partons");
  // Init Delphes - read in configuration & define modules to be executed
  m_Delphes->InitTask();
  m_branches = new ExRootConfParam(m_confReader->GetParam("TreeWriter::Branch"));
  ExRootConfParam param = m_confReader->GetParam("::ExecutionPath");
  // Print Delphes modules to be used
  debug() << "Delphes simulation will use the following modules: " << endmsg;
  for (size_t k = 0; k < param.GetSize(); ++k) {
    debug() << "-- Module: " << param[k].GetString() << endmsg;
  }
  m_Delphes->Clear();
  return StatusCode::SUCCESS;
}

StatusCode DelphesSimulation::execute() {
  m_Delphes->Clear();
  info() << "DelphesEvent " << m_eventCounter << endmsg;
  // convert input particles to delphes candidates
  const edm4hep::MCParticleCollection* coll = m_handleGenParticles.get();

  for (auto p : *coll) {
    std::cout << p << std::endl;
    Candidate* cand = m_factory->NewCandidate();
    auto _pdgCode = p.getPDG();
    cand->PID = _pdgCode;
    cand->Status = p.getGeneratorStatus();
    auto _mom = p.getMomentum();
    cand->Momentum.SetXYZM(_mom[0], _mom[1], _mom[2], p.getMass());
    auto _pos = p.getVertex();
    cand->Position.SetXYZT(_pos[0], _pos[1], _pos[2], p.getTime());
    cand->IsPU = 0;
    cand->Charge = p.getCharge();
    cand->M1 = -1;
    cand->M2 = -1;
    cand->D1 = -1;
    cand->D2 = -1;
    m_allParticles->Add(cand);
    if (cand->Status == 1) {
      m_stableParticles->Add(cand);
    } else if (_pdgCode <= 5 || _pdgCode == 21 || _pdgCode == 15) {
      m_partons->Add(cand);
    }
  }


  m_eventCounter++;
  m_Delphes->ProcessTask();

  int maxEvents = m_confReader->GetInt("::MaxEvents", 0);
  auto branches = *m_branches;
  int nParams = branches.GetSize();


  auto coll_recoParticles = m_handleRecoParticles.createAndPut();
  auto coll_recoJets = m_handleRecoJets.createAndPut();
  auto coll_recoSubJets = m_handleRecoSubJets.createAndPut();
  auto coll_recoTags = m_handleRecoTags.createAndPut();
  auto coll_scalarHT = m_handleScalarHT.createAndPut();
  auto coll_missingET = m_handleMissingET.createAndPut();
  auto coll_mcRecoParticleAssociations = m_handleMCRecoParticleAssociation.createAndPut();

  for(int b = 0; b < nParams; b += 3) {
    TString input = branches[b].GetString();
    TString name = branches[b + 1].GetString();
    TString className = branches[b + 2].GetString();
    std::string _name;
    const TObjArray* delphesColl = m_Delphes->ImportArray(input);


    if (className == "Track") {
     //std::cout <<  input << "\t" << name << "\t" << className << "\t" << delphesColl->GetEntries() << std::endl;
      for (int j = 0; j < delphesColl->GetEntries(); j++) {
        auto cand = static_cast<Candidate*>(delphesColl->At(j));
        auto track = convertTrack(cand, 2 /*magFieldBz [T]*/); 
      }
    } else if (className == "Electron" || className == "Photon" || className == "Muon") {
      for (int iCand = 0; iCand < delphesCollection->GetEntriesFast(); ++iCand) {
      auto* delphesCand = static_cast<Candidate*>(delphesCollection->At(iCand));

      auto cand = coll_recoParticles->create();
      cand.setCharge(delphesCand->Charge);
      cand.setMass(delphesCand->Mass);
      cand.setMomentum({
        (float) delphesCand->Momentum.Px(),
        (float) delphesCand->Momentum.Py(),
        (float) delphesCand->Momentum.Pz()
      });
      cand.setVertex({
          (float) delphesCand->Position.X(),
          (float) delphesCand->Position.Y(),
          (float) delphesCand->Position.Z()});
        cand.setPDG(delphesCand->PID); // delphes uses whatever hepevt.idhep provides
      }
    } else if (className == "Jet") {
      for (int iCand = 0; iCand < delphesCollection->GetEntriesFast(); ++iCand) {
      auto* delphesCand = static_cast<Candidate*>(delphesCollection->At(iCand));

      auto jet = coll_recoJets->create();
      jet.setCharge(delphesCand->Charge);
      jet.setMomentum({(float) delphesCand->Momentum.Px(),
                       (float) delphesCand->Momentum.Py(),
                       (float) delphesCand->Momentum.Pz()});
      jet.setEnergy(delphesCand->Momentum.E());

      }
    }
  }






  m_Delphes->Clear();
  return StatusCode::SUCCESS;
}

StatusCode DelphesSimulation::finalize() {

  // Finish Delphes task
  m_Delphes->FinishTask();


  debug() << "Exiting Delphes..." << endmsg;

  //if (m_Delphes != nullptr) m_Delphes.reset();                // Releases also the memory allocated by treeWriter
  if (m_confReader.get() != nullptr) m_confReader.reset();

  return GaudiAlgorithm::finalize();
}

