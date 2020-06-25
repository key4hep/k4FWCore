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


// ROOT
#include "TFile.h"
#include "TObjArray.h"
#include "TRandom.h"
#include "TStopwatch.h"

DECLARE_COMPONENT(DelphesSimulation)

DelphesSimulation::DelphesSimulation(const std::string& name, ISvcLocator* svcLoc) : GaudiAlgorithm(name, svcLoc) {
  declareProperty("MCParticles", m_handleGenParticles);
}

StatusCode DelphesSimulation::initialize() {
  //gRandom->SetSeed(1234);

  // Read Delphes configuration card (deleted by finalize())
  m_confReader = std::unique_ptr<ExRootConfReader>(new ExRootConfReader);
  m_confReader->ReadFile(m_DelphesCard.value().c_str());

  // Instance of Delphes (deleted by finalize())
  m_Delphes = std::unique_ptr<Delphes>(new Delphes("Delphes"));
  m_Delphes->SetConfReader(m_confReader.get());

  // Delphes needs data structure to be defined (ROOT tree) (deleted by finalize())
  //m_treeWriter = new ExRootTreeWriter(m_outRootFile, "DelphesSim");
  //m_branchEvent = m_treeWriter->NewBranch("Event", HepMCEvent::Class());
  //m_Delphes->SetTreeWriter(m_treeWriter);

  // Define event readers
  //
  //  HepMC reader --> reads either from a file or directly from data store (deleted by finalize())
  //m_hepMCConverter = std::unique_ptr<HepMCDelphesConverter>(new HepMCDelphesConverter);

  // Create following arrays of Delphes objects --> starting objects
  m_allParticles = m_Delphes->ExportArray("allParticles");
  m_stableParticles = m_Delphes->ExportArray("stableParticles");
  m_partons = m_Delphes->ExportArray("partons");

  // Init Delphes - read in configuration & define modules to be executed
  m_Delphes->InitTask();

  // Print Delphes modules to be used
  ExRootConfParam param = m_confReader->GetParam("::ExecutionPath");
  Long_t size = param.GetSize();
  debug() << "Delphes simulation will use the following modules: " << endmsg;
  for (Long_t k = 0; k < size; ++k) {

    TString name = param[k].GetString();
    debug() << "-- Module: " << name << endmsg;
  }

  m_eventCounter = 0;

  //m_treeWriter->Clear();
  m_Delphes->Clear();

  //for (auto& toolname : m_saveToolNames) {
    //m_saveTools.push_back(tool<IDelphesSaveOutputTool>(toolname));
    // FIXME: check StatusCode once the m_saveTools is a ToolHandleArray
    // if (!) {
    //   error() << "Unable to retrieve the output saving tool." << endmsg;
    //   return StatusCode::FAILURE;
    // }

  return StatusCode::SUCCESS;
}

StatusCode DelphesSimulation::execute() {

  //
  // Read event & initialize event variables
  TStopwatch readStopWatch;
  readStopWatch.Start();

  StatusCode sc;

  // Read event
  //const HepMC::GenEvent* hepMCEvent = m_hepmcHandle.get();

  //sc = m_hepMCConverter->hepMCEventToArrays(
  //    hepMCEvent, *m_Delphes->GetFactory(), *m_allParticles, *m_stableParticles, *m_partons);
  //if (!sc.isSuccess()) {
  //  return sc;
  //}
  m_eventCounter++;
  readStopWatch.Stop();

  //
  // Process event
  TStopwatch procStopWatch;

  // Delphes process
  procStopWatch.Start();
  m_Delphes->ProcessTask();
  procStopWatch.Stop();



  m_Delphes->Clear();

  return StatusCode::SUCCESS;
}

StatusCode DelphesSimulation::finalize() {

  // Finish Delphes task
  m_Delphes->FinishTask();


  debug() << "Exiting Delphes..." << endmsg;

  if (m_Delphes.get() != nullptr) m_Delphes.reset();                // Releases also the memory allocated by treeWriter
  if (m_confReader.get() != nullptr) m_confReader.reset();

  return GaudiAlgorithm::finalize();
}

