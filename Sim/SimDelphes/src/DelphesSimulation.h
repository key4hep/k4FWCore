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
class ExRootTreeWriter;

namespace edm4hep {
class MCParticleCollection;
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

  // Delphes
  std::unique_ptr<Delphes> m_Delphes{nullptr};

  DataHandle<edm4hep::MCParticleCollection> m_handleGenParticles{"MCParticles", Gaudi::DataHandle::Reader, this};
  int m_eventCounter{0};

  // Delphes ROOT output
  TFile* m_outRootFile{nullptr};
  /// Name of Delphes Root output file, if defined, the Delphes standard tree write out in addition to FCC output
  std::unique_ptr<ExRootConfReader> m_confReader{nullptr};
  //Gaudi::Property<std::vector<std::string>> m_saveToolNames{this, "outputs"};
  //std::vector<IDelphesSaveOutputTool*> m_saveTools;

  // Arrays used by Delphes and internally for initial particles
  TObjArray* m_allParticles{nullptr};
  TObjArray* m_stableParticles{nullptr};
  TObjArray* m_partons{nullptr};
};

#endif  // #define _DELPHESSIMULATION_H
