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
#include <cstdlib>
#include <filesystem>

#include "PodioOutput.h"
#include "k4FWCore/PodioDataSvc.h"
#include "rootUtils.h"

DECLARE_COMPONENT(PodioOutput)

PodioOutput::PodioOutput(const std::string& name, ISvcLocator* svcLoc)
    : Gaudi::Algorithm(name, svcLoc), m_firstEvent(true) {}

StatusCode PodioOutput::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(evtSvc().get());
  if (nullptr == m_podioDataSvc) {
    error() << "Could not get DataSvc!" << endmsg;
    return StatusCode::FAILURE;
  }

  // check whether output directory needs to be created and eventualy create it
  auto outDirPath = std::filesystem::path(m_filename.value()).parent_path();
  if (!outDirPath.empty() && !std::filesystem::is_directory(outDirPath)) {
    debug() << "Creating output directory:" << endmsg;
    debug() << outDirPath << endmsg;
    auto success = std::filesystem::create_directories(outDirPath);
    if (!success) {
      error() << "Output directory can't be created!" << endmsg;
      error() << outDirPath << endmsg;

      return StatusCode::FAILURE;
    }
  }

  m_framewriter = std::make_unique<podio::ROOTWriter>(m_filename);
  m_switch      = KeepDropSwitch(m_outputCommands);

  return StatusCode::SUCCESS;
}

StatusCode PodioOutput::execute(const EventContext&) const {
  auto& frame = m_podioDataSvc->getEventFrame();

  // register for writing
  if (m_firstEvent) {
    auto collections = frame.getAvailableCollections();
    for (auto& collection_name : collections) {
      if (m_switch.isOn(collection_name)) {
        m_collection_names_to_write.push_back(collection_name);
      }
    }
    m_framewriter->writeFrame(frame, "events", m_collection_names_to_write);
  } else {
    try {
      m_framewriter->writeFrame(frame, "events", m_collection_names_to_write);
    } catch (std::runtime_error&) {
      // In this error message we are only interested in the ones that are
      // missing, since only a missing collection can trigger the exception
      // here. Additional collections that are present in the Frame are not
      // necessarily an issue here, because we might just be configured to not
      // write all of them
      const auto& [missing, _] = m_framewriter->checkConsistency(frame.getAvailableCollections(), "events");
      error() << "Could not write event, because the following collections are not present: ";
      std::string sep = "";
      for (const auto& name : missing) {
        error() << sep << name;
        sep = ", ";
      }
      error() << endmsg;

      return StatusCode::FAILURE;
    }
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
  if (Gaudi::Algorithm::finalize().isFailure())
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
  podio::Frame config_metadata_frame{};
  config_metadata_frame.putParameter("gaudiConfigOptions", config_data);
  if (const char* env_key4hep_stack = std::getenv("KEY4HEP_STACK")) {
    std::string s_env_key4hep_stack = env_key4hep_stack;
    config_metadata_frame.putParameter("key4hepstack", s_env_key4hep_stack);
  }
  m_framewriter->writeFrame(config_metadata_frame, "configuration_metadata");

  auto& metadata_frame = m_podioDataSvc->getMetaDataFrame();
  m_framewriter->writeFrame(metadata_frame, "metadata");

  // write information into file
  m_framewriter->finish();

  return StatusCode::SUCCESS;
}
