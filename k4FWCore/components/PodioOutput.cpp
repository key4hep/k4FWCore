#include <cstdlib>

#include "PodioOutput.h"
#include "TFile.h"
#include "k4FWCore/PodioDataSvc.h"
#include "rootUtils.h"

DECLARE_COMPONENT(PodioOutput)

PodioOutput::PodioOutput(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc), m_firstEvent(true) {}

StatusCode PodioOutput::initialize() {
  if (GaudiAlgorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(evtSvc().get());
  if (nullptr == m_podioDataSvc) {
    error() << "Could not get DataSvc!" << endmsg;
    return StatusCode::FAILURE;
  }

  m_framewriter = std::make_unique<podio::ROOTFrameWriter>(m_filename); 
  m_switch = KeepDropSwitch(m_outputCommands);

  return StatusCode::SUCCESS;
}

StatusCode PodioOutput::execute() {
  auto& frame = m_podioDataSvc->getFrame();

  // for now assume identical content for every event
  // register for writing
  if (m_firstEvent) {
    auto collections = frame.getAvailableCollections();
    std::vector<std::string> collection_names_to_write;
    for (auto collection_name : collections) {
      if (m_switch.isOn(collection_name)) {
	  collection_names_to_write.push_back(collection_name);
      }
    }
    m_framewriter->writeFrame(frame, "events", collection_names_to_write);
  } else {
    m_framewriter->writeFrame(frame, "events");
  }
  m_firstEvent = false;

  return StatusCode::SUCCESS;
}

/** PodioOutput::finalize
* has to happen after all algorithms that touch the data store finish.
* Here the job options are retrieved and stored to disk as a branch
* in the metadata tree.
*
*/
StatusCode PodioOutput::finalize() {
  if (GaudiAlgorithm::finalize().isFailure())
    return StatusCode::FAILURE;
  //// prepare job options metadata ///////////////////////
  // retrieve the configuration of the job
  // and write it to file as vector of strings
  std::vector<std::string> config_data;
  const auto&              jobOptionsSvc         = Gaudi::svcLocator()->getOptsSvc();
  const auto&              configured_properties = jobOptionsSvc.items();
  for (const auto& per_property : configured_properties) {
    std::stringstream config_stream;
    // sample output:
    // HepMCToEDMConverter.genparticles = "GenParticles";
    // Note that quotes are added to all property values,
    // which leads to problems with ints, lists, dicts and bools.
    // For theses types, the quotes must be removed in postprocessing.
    config_stream << std::get<0>(per_property) << " = \"" << std::get<1>(per_property) << "\";" << std::endl;
    config_data.push_back(config_stream.str());
  }
  // Some default components are not captured by the job option service
  // and have to be traversed like this. Note that Gaudi!577 will improve this.
  for (const auto* name : {"ApplicationMgr", "MessageSvc", "NTupleSvc"}) {
    std::stringstream config_stream;
    auto              svc = service<IProperty>(name);
    if (!svc.isValid())
      continue;
    for (const auto* property : svc->getProperties()) {
      config_stream << name << "." << property->name() << " = \"" << property->toString() << "\";" << std::endl;
    }
    config_data.push_back(config_stream.str());
  }

  // Collect all the metadata
  podio::Frame metadata_frame{};
  metadata_frame.putParameter("gaudiConfigOptions", config_data);
  if (const char* env_key4hep_stack = std::getenv("KEY4HEP_STACK")) {
    std::string s_env_key4hep_stack = env_key4hep_stack;
    metadata_frame.putParameter("key4hepstack", s_env_key4hep_stack);
  }
  m_framewriter->writeFrame(metadata_frame, "metadata");

  // TODO: Create frames for run metadata and collection metadata

  // write information into file
  m_framewriter->finish();

  return StatusCode::SUCCESS;
}
