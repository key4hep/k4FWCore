#ifndef _DELPHESSIMULATION_H
#define _DELPHESSIMULATION_H

#include "FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"


#include "edm4hep/MCParticleCollection.h"


// STL & System
#include <iostream>
#include <memory>
#include <sstream>

// Forward Delphes
class Delphes;
class DelphesFactory;
class HepMCDelphesConverter;
class Candidate;
class ExRootTreeBranch;
class ExRootConfReader;
class ExRootConfParam;
class ExRootTreeWriter;

namespace edm4hep {
class MCParticleCollection;
class ReconstructedParticleCollection;
class MCRecoParticleAssociationCollection;
class ParticleIDCollection;
}

// Forward ROOT
class TFile;
class TObjArray;

/** @class DelphesSimulation
 *
 */
class DelphesSimulation : public GaudiAlgorithm {

public:
  DelphesSimulation(const std::string& name, ISvcLocator* svcLoc);
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:
  Gaudi::Property<std::string> m_DelphesCard{this, "DelphesCard", "",
                                             "Name of Delphes tcl config file with detector and simulation parameters"};
  // Data Input
  DataHandle<edm4hep::MCParticleCollection> m_handleGenParticles{"MCParticles", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_handleRecoParticles{"RecoParticlesDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_handleRecoJets{"RecoJetsDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_handleRecoSubJets{"RecoSubJetsDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_handleMissingET{"MissingETDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ParticleIDCollection> m_handleScalarHT{"ScalarHTDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ParticleIDCollection> m_handleRecoTags{"RecoTagsDelphes", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoParticleAssociationCollection> m_handleMCRecoParticleAssociation{"MCRecoParticleAssociation", Gaudi::DataHandle::Writer, this};
  
  int m_eventCounter{0};
  Delphes* m_Delphes{nullptr};
  DelphesFactory* m_factory;
  std::unique_ptr<ExRootConfReader> m_confReader{nullptr};
  // Arrays used by Delphes and internally for initial particles
  TObjArray* m_allParticles{nullptr};
  TObjArray* m_stableParticles{nullptr};
  TObjArray* m_partons{nullptr};
  ExRootConfParam* m_branches;
};

#endif  // #define _DELPHESSIMULATION_H
