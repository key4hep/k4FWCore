#include "PodioOutput.h"
#include "k4FWCore/PodioDataSvc.h"
#include "TFile.h"
#include "rootUtils.h"

DECLARE_COMPONENT(PodioOutput)

PodioOutput::PodioOutput(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc), m_firstEvent(true) {}

StatusCode PodioOutput::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;

  // check whether we have the PodioEvtSvc active
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(evtSvc().get());
  if (nullptr  == m_podioDataSvc) {
    error() << "Could not get DataSvc!" << endmsg;
    return StatusCode::FAILURE;
  }

  m_file = std::unique_ptr<TFile>(TFile::Open(m_filename.value().c_str(), "RECREATE", "data file"));
  // Both trees are written to the ROOT file and owned by it
  // PodioDataSvc has ownership of EventDataTree
  m_datatree = m_podioDataSvc->eventDataTree();
  m_datatree->SetDirectory(m_file.get());
  m_metadatatree = new TTree("metadata", "Metadata tree");
  m_runMDtree =new TTree("run_metadata", "Run metadata tree");
  m_evtMDtree = new TTree("evt_metadata", "Event metadata tree");
  m_colMDtree = new TTree("col_metadata", "Collection metadata tree");

  m_evtMDtree->Branch("evtMD", "GenericParameters", m_podioDataSvc->getProvider().eventMetaDataPtr() ) ;
  m_switch = KeepDropSwitch(m_outputCommands);
  return StatusCode::SUCCESS;
}

void PodioOutput::resetBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections) {
  for (auto& collNamePair : collections) {
    auto collName = collNamePair.first;
    if (m_switch.isOn(collName)) {
      // Reconnect branches and collections
      const auto collBuffers = collNamePair.second->getBuffers();
      m_datatree->SetBranchAddress(collName.c_str(), collBuffers.data);
      auto colls = collBuffers.references;
      if (colls != nullptr) {
        for (size_t j = 0; j < colls->size(); ++j) {
          // We need to make sure that we pass the actual pointer to
          // SetBranchAddress instead of the pointer to the unique_ptr
          auto collPtr = (*colls)[j].get();
          m_datatree->SetBranchAddress((collName + "#" + std::to_string(j)).c_str(), &collPtr);
        }
      }
      auto colls_v = collBuffers.vectorMembers;
      if (colls_v != nullptr) {
        int j = 0;
        for (auto& c : (*colls_v)) {
          void* add = c.second ;
          m_datatree->SetBranchAddress((collName + "_" + std::to_string(j)).c_str(), add);
          ++j;
        }
      }
    }
    collNamePair.second->prepareForWrite();
  }
}

void PodioOutput::createBranches(const std::vector<std::pair<std::string, podio::CollectionBase*>>& collections) {

  // collectionID, collection type, subset collection
  std::vector<std::tuple<int, std::string, bool>>*  collectionInfo = new std::vector<std::tuple<int, std::string, bool>>();
  collectionInfo->reserve( collections.size());

  for (auto& collNamePair : collections) {
    auto collName = collNamePair.first;
    std::string className( collNamePair.second->getValueTypeName() ) ;
    std::string collClassName = "vector<" + className + "Data>";
    int isOn = 0;
    if (m_switch.isOn(collName)) {
      isOn = 1;
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
      if (vminfo != nullptr){
        int i = 0;
        for(auto& c : (*vminfo)){
          std::string typeName = "vector<"+c.first+">" ;
          void* add = c.second ;
          m_datatree->Branch((collName+"_"+std::to_string(i)).c_str(),
                             typeName.c_str(),
                             add);
          ++i;
        }
      }
      }

    }
        
    const auto collID = m_podioDataSvc->getCollectionIDs()->collectionID(collName);
    // No check necessary, only registered collections possible
    auto coll = collNamePair.second;
    const auto collType = coll->getValueTypeName() + "Collection";
    collectionInfo->emplace_back(collID, std::move(collType), coll->isSubsetCollection());
  //}







    debug() << isOn << " Registering collection " << collClassName << " " << collName.c_str() << " containing type "
            << className << endmsg;
    collNamePair.second->prepareForWrite();
  }

    m_metadatatree->Branch("CollectionTypeInfo", collectionInfo);
}

StatusCode PodioOutput::execute() {
  // for now assume identical content for every event
  // register for writing
  if (m_firstEvent) {
    createBranches(m_podioDataSvc->getCollections());
  } else {
    resetBranches(m_podioDataSvc->getCollections());
  }
  m_firstEvent = false;
  debug() << "Filling DataTree .." << endmsg;
  m_datatree->Fill();
  m_evtMDtree->Fill();
  return StatusCode::SUCCESS;
}

/** PodioOutput::finalize
* has to happen after all algorithms that touch the data store finish.
* Here the job options are retrieved and stored to disk as a branch
* in the metadata tree.
*
*/
StatusCode PodioOutput::finalize() {
  if (GaudiAlgorithm::finalize().isFailure()) return StatusCode::FAILURE;
  //// prepare job options metadata ///////////////////////
  // retrieve the configuration of the job
  // and write it to file as vector of strings
  std::vector<std::string> config_data;
  const auto& jobOptionsSvc = Gaudi::svcLocator()->getOptsSvc();
  const auto& configured_properties = jobOptionsSvc.items();
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
    auto svc = service<IProperty>( name );
    if (!svc.isValid()) continue;
    for (const auto* property : svc->getProperties()) {
      config_stream << name << "." << property->name() << " = \"" << property->toString() << "\";" << std::endl;
    }
    config_data.push_back(config_stream.str());
  }
  //// finalize trees and file //////////////////////////////
  m_file->cd();
  m_metadatatree->Branch("gaudiConfigOptions", &config_data);
  m_metadatatree->Branch("CollectionIDs", m_podioDataSvc->getCollectionIDs());



  m_metadatatree->Fill();


    m_colMDtree->Branch("colMD", "std::map<int,podio::GenericParameters>", m_podioDataSvc->getProvider().getColMetaDataMap() ) ;
    m_colMDtree->Fill();
    m_runMDtree->Branch("runMD", "std::map<int,podio::GenericParameters>", m_podioDataSvc->getProvider().getRunMetaDataMap() ) ;
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
