/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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

#include "PodioLegacyOutput.h"
#include "TFile.h"
#include "k4FWCore/PodioLegacyDataSvc.h"
#include "rootUtils.h"

DECLARE_COMPONENT(PodioLegacyOutput)

PodioLegacyOutput::PodioLegacyOutput(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc), m_firstEvent(true) {}

StatusCode PodioLegacyOutput::initialize() {
  if (GaudiAlgorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioLegacyDataSvc = dynamic_cast<PodioLegacyDataSvc*>(evtSvc().get());
  if (nullptr == m_podioLegacyDataSvc) {
    error() << "Could not get DataSvc!" << endmsg;
    return StatusCode::FAILURE;
  }

  m_file = std::unique_ptr<TFile>(TFile::Open(m_filename.value().c_str(), "RECREATE", "data file"));
  // Both trees are written to the ROOT file and owned by it
  // PodioDataSvc has ownership of EventDataTree
  m_datatree = m_podioLegacyDataSvc->eventDataTree();
  m_datatree->SetDirectory(m_file.get());
  m_metadatatree = new TTree("metadata", "Metadata tree");
  m_runMDtree    = new TTree("run_metadata", "Run metadata tree");
  m_evtMDtree    = new TTree("evt_metadata", "Event metadata tree");
  m_colMDtree    = new TTree("col_metadata", "Collection metadata tree");

  m_evtMDtree->Branch("evtMD", "GenericParameters", m_podioLegacyDataSvc->getProvider().eventMetaDataPtr());
  m_switch = KeepDropSwitch(m_outputCommands);
  return StatusCode::SUCCESS;
}

void PodioLegacyOutput::resetBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections) {
  for (auto& collNamePair : collections) {
    auto collName = collNamePair.first;
    if (m_switch.isOn(collName)) {
      // Reconnect branches and collections
      const auto collBuffers = collNamePair.second->getBuffers();
      m_datatree->SetBranchAddress(collName.c_str(), collBuffers.data);
      auto colls = collBuffers.references;
      if (colls != nullptr) {
        for (size_t j = 0; j < colls->size(); ++j) {
          auto l_branch = m_datatree->GetBranch((collName + "#" + std::to_string(j)).c_str());
          l_branch->SetAddress(&(*colls)[j]);
        }
      }
      auto colls_v = collBuffers.vectorMembers;
      if (colls_v != nullptr) {
        int j = 0;
        for (auto& c : (*colls_v)) {
          void* add = c.second;
          m_datatree->SetBranchAddress((collName + "_" + std::to_string(j)).c_str(), add);
          ++j;
        }
      }
    }
    collNamePair.second->prepareForWrite();
  }
}

void PodioLegacyOutput::createBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections) {
  // collectionID, collection type, subset collection
  std::vector<std::tuple<int, std::string, bool>>* collectionInfo =
      new std::vector<std::tuple<int, std::string, bool>>();
  collectionInfo->reserve(collections.size());

  for (auto& collNamePair : collections) {
    auto        collName = collNamePair.first;
    std::string className(collNamePair.second->getValueTypeName());
    std::string collClassName = "vector<" + className + "Data>";
    int         isOn          = 0;
    if (m_switch.isOn(collName)) {
      isOn                   = 1;
      const auto collBuffers = collNamePair.second->getBuffers();
      m_datatree->Branch(collName.c_str(), collClassName.c_str(), collBuffers.data);
      // Create branches for collections holding relations
      if (auto refColls = collBuffers.references) {
        int i = 0;
        for (auto& c : (*refColls)) {
          const auto brName = podio::root_utils::refBranch(collName, i);
          m_datatree->Branch(brName.c_str(), c.get());
          ++i;
        }
        // ---- vector members
        auto vminfo = collBuffers.vectorMembers;
        if (vminfo != nullptr) {
          int i = 0;
          for (auto& c : (*vminfo)) {
            std::string typeName = "vector<" + c.first + ">";
            void*       add      = c.second;
            m_datatree->Branch((collName + "_" + std::to_string(i)).c_str(), typeName.c_str(), add);
            ++i;
          }
        }
      }
    }

    const auto collID = m_podioLegacyDataSvc->getCollectionIDs()->collectionID(collName).value();
    // No check necessary, only registered collections possible
    auto       coll     = collNamePair.second;
    const auto collType = std::string(coll->getValueTypeName()) + "Collection";
    collectionInfo->emplace_back(collID, std::move(collType), coll->isSubsetCollection());
    //}

    debug() << isOn << " Registering collection " << collClassName << " " << collName.c_str() << " containing type "
            << className << endmsg;
    collNamePair.second->prepareForWrite();
  }

  m_metadatatree->Branch("CollectionTypeInfo", collectionInfo);
}

StatusCode PodioLegacyOutput::execute() {
  // for now assume identical content for every event
  // register for writing
  if (m_firstEvent) {
    createBranches(m_podioLegacyDataSvc->getCollections());
  } else {
    resetBranches(m_podioLegacyDataSvc->getCollections());
  }
  m_firstEvent = false;
  debug() << "Filling DataTree .." << endmsg;
  m_datatree->Fill();
  m_evtMDtree->Fill();
  return StatusCode::SUCCESS;
}

/** PodioLegacyOutput::finalize
* has to happen after all algorithms that touch the data store finish.
* Here the job options are retrieved and stored to disk as a branch
* in the metadata tree.
*
*/
StatusCode PodioLegacyOutput::finalize() {
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
  //// finalize trees and file //////////////////////////////
  m_file->cd();

  if (const char* env_key4hep_stack = std::getenv("KEY4HEP_STACK")) {
    std::string s_env_key4hep_stack = env_key4hep_stack;
    m_metadatatree->Branch("key4hepStack", &s_env_key4hep_stack);
  }

  m_metadatatree->Branch("gaudiConfigOptions", &config_data);
  m_metadatatree->Branch("CollectionIDs", m_podioLegacyDataSvc->getCollectionIDs());

  m_metadatatree->Fill();

  m_colMDtree->Branch("colMD", "std::map<int,podio::GenericParameters>",
                      m_podioLegacyDataSvc->getProvider().getColMetaDataMap());
  m_colMDtree->Fill();
  m_runMDtree->Branch("runMD", "std::map<int,podio::GenericParameters>",
                      m_podioLegacyDataSvc->getProvider().getRunMetaDataMap());
  m_runMDtree->Fill();

  m_datatree->Write();
  m_file->Write();
  m_file->Close();
  info() << "Data written to: " << m_filename.value();
  if (!m_filenameRemote.value().empty()) {
    TFile::Cp(m_filename.value().c_str(), m_filenameRemote.value().c_str(), false);
    info() << " and copied to: " << m_filenameRemote.value() << endmsg;
  }
  return StatusCode::SUCCESS;
}
