#include "k4FWCore/PodioDataSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/ISvcLocator.h"

#include "k4FWCore/DataWrapper.h"

#include "TTree.h"

#include "edm4hep/MCParticleCollection.h"

/// Service initialisation
StatusCode PodioDataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode status = DataSvc::initialize();
  ISvcLocator* svc_loc = serviceLocator();


  // Attach data loader facility
  m_cnvSvc = svc_loc->service("EventPersistencySvc");
  status = setDataLoader(m_cnvSvc);

  if (m_filename != "") {
    m_filenames.push_back(m_filename);
  }

  if (m_filenames.size() > 0) {
    if (m_filenames[0] != "") {
      m_reader.openFiles(m_filenames);
      m_eventMax = m_reader.getEntries();

      m_provider.setReader(&m_reader);

      auto idTable = m_provider.getCollectionIDTable();
      setCollectionIDs(idTable);

      if (m_1stEvtEntry != 0 ) {
          m_reader.goToEvent(m_1stEvtEntry);
          m_eventMax -= m_1stEvtEntry;
      }
    }
  }
  return status;
}
/// Service reinitialisation
StatusCode PodioDataSvc::reinitialize() {
  // Do nothing for this service
  return StatusCode::SUCCESS;
}
/// Service finalization
StatusCode PodioDataSvc::finalize() {
  m_cnvSvc = 0; // release
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS;
}

StatusCode PodioDataSvc::clearStore() {
  for (auto& collNamePair : m_collections) {
    if (collNamePair.second != nullptr) {
      collNamePair.second->clear();
    }
  }
  for (auto& collNamePair : m_readCollections) {
    if (collNamePair.second != nullptr) {
      collNamePair.second->clear();
    }
  }
  DataSvc::clearStore().ignore();
  m_collections.clear();
  m_readCollections.clear();
  return StatusCode::SUCCESS;
}

void PodioDataSvc::endOfRead() {
  StatusCode sc;
  if (m_eventMax != -1) {
    m_provider.clearCaches();
    m_reader.endOfEvent();
    if (m_eventNum++ > m_eventMax) {
      info() << "Reached end of file with event " << m_eventMax << endmsg;
      IEventProcessor* eventProcessor;
      sc = service("ApplicationMgr", eventProcessor);
      sc = eventProcessor->stopRun();
    }
  }
  // todo: figure out sthg to do with sc (added to silence -Wunused-result)
}

void PodioDataSvc::setCollectionIDs(podio::CollectionIDTable* collectionIds) {
  if (m_collectionIDs != nullptr) {
    delete m_collectionIDs;
  }
  m_collectionIDs = collectionIds;
}

/// Standard Constructor
PodioDataSvc::PodioDataSvc(const std::string& name, ISvcLocator* svc)
    : DataSvc(name, svc), m_collectionIDs(new podio::CollectionIDTable()) {

  m_eventDataTree = new TTree("events", "Events tree");
    }

/// Standard Destructor
PodioDataSvc::~PodioDataSvc() {}

StatusCode PodioDataSvc::readCollection(const std::string& collName, int collectionID) {
  podio::CollectionBase* collection(nullptr);

  auto idTable = m_provider.getCollectionIDTable();
  m_provider.get(collectionID, collection);

  if (collection->isSubsetCollection()) {
    return StatusCode::SUCCESS;
  }
  auto wrapper = new DataWrapper<podio::CollectionBase>;
  int id = m_collectionIDs->add(collName);
  collection->setID(id);
  collection->prepareAfterRead();
  wrapper->setData(collection);
  m_readCollections.emplace_back(std::make_pair(collName, collection));
  return registerObject("/Event", "/" + collName, wrapper);
}

StatusCode PodioDataSvc::registerObject(std::string_view parentPath, std::string_view fullPath, DataObject* pObject) {
  DataWrapperBase* wrapper = dynamic_cast<DataWrapperBase*>(pObject);
  if (wrapper != nullptr) {
    podio::CollectionBase* coll = wrapper->collectionBase();
    if (coll != nullptr) {
      size_t pos = fullPath.find_last_of("/");
      std::string shortPath(fullPath.substr(pos + 1, fullPath.length()));
      int id = m_collectionIDs->add(shortPath);
      coll->setID(id);
      m_collections.emplace_back(std::make_pair(shortPath, coll));
    }
  }
  return DataSvc::registerObject(parentPath, fullPath, pObject);
}
