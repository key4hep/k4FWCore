#include "k4FWCore/PodioDataSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/ISvcLocator.h"

#include "k4FWCore/DataWrapper.h"

#include "TTree.h"

/// Service initialisation
StatusCode PodioDataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode   status  = DataSvc::initialize();
  ISvcLocator* svc_loc = serviceLocator();

  // Attach data loader facility
  m_cnvSvc = svc_loc->service("EventPersistencySvc");
  status   = setDataLoader(m_cnvSvc);

  if (m_filename != "") {
    m_filenames.push_back(m_filename);
  }

  if (m_filenames.size() > 0) {
    if (m_filenames[0] != "") {
      m_reading_from_file = true;
      m_reader.openFiles(m_filenames);
      m_eventMax = m_reader.getEntries("events");

      if (m_1stEvtEntry != 0) {
        m_eventMax -= m_1stEvtEntry;
      }
    }
  }

  if (m_reading_from_file) {
    m_metadataframe = m_reader.readEntry("metadata", 0);
  } else {
    m_metadataframe = podio::Frame();
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
  m_cnvSvc = 0;  // release
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS;
}

StatusCode PodioDataSvc::clearStore() {
  // as the frame takes care of the ownership of the podio::Collections,
  // make sure the DataWrappers don't cause a double delete
  for (auto wrapper : m_podio_datawrappers) {
    wrapper->resetData();
  }
  m_podio_datawrappers.clear();

  DataSvc::clearStore().ignore();
  return StatusCode::SUCCESS;
}

StatusCode PodioDataSvc::i_setRoot(std::string root_path, IOpaqueAddress* pRootAddr) {
  // create a new frame
  if (m_reading_from_file) {
    m_eventframe = podio::Frame(m_reader.readEntry("events", m_eventNum + m_1stEvtEntry));
  } else {
    m_eventframe = podio::Frame();
  }
  return DataSvc::i_setRoot(root_path, pRootAddr);
}

StatusCode PodioDataSvc::i_setRoot(std::string root_path, DataObject* pRootObj) {
  // create a new frame
  if (m_reading_from_file) {
    m_eventframe = podio::Frame(m_reader.readEntry("events", m_eventNum + m_1stEvtEntry));
  } else {
    m_eventframe = podio::Frame();
  }
  return DataSvc::i_setRoot(root_path, pRootObj);
}

void PodioDataSvc::endOfRead() {
  StatusCode sc;
  if (m_eventMax != -1) {
    if (m_eventNum++ >= m_eventMax - 1) {  // we start counting at 0 thus the -1.
      info() << "Reached end of file with event " << m_eventMax << endmsg;
      IEventProcessor* eventProcessor;
      sc = service("ApplicationMgr", eventProcessor);
      sc = eventProcessor->stopRun();
    }
  }
  // todo: figure out sthg to do with sc (added to silence -Wunused-result)
}

/// Standard Constructor
PodioDataSvc::PodioDataSvc(const std::string& name, ISvcLocator* svc) : DataSvc(name, svc) {}

/// Standard Destructor
PodioDataSvc::~PodioDataSvc() {}

StatusCode PodioDataSvc::readCollection(const std::string& collName) {
  const podio::CollectionBase* collection(nullptr);
  collection = m_eventframe.get(collName);
  if (collection == nullptr) {
    error() << "Collection " << collName << " does not exist." << endmsg;
  }
  auto wrapper = new DataWrapper<podio::CollectionBase>;
  wrapper->setData(collection);
  m_podio_datawrappers.push_back(wrapper);
  return DataSvc::registerObject("/Event", "/" + collName, wrapper);
}

StatusCode PodioDataSvc::registerObject(std::string_view parentPath, std::string_view fullPath, DataObject* pObject) {
  DataWrapperBase* wrapper = dynamic_cast<DataWrapperBase*>(pObject);
  if (wrapper != nullptr) {
    podio::CollectionBase* coll = wrapper->collectionBase();
    if (coll != nullptr) {
      size_t      pos = fullPath.find_last_of("/");
      std::string shortPath(fullPath.substr(pos + 1, fullPath.length()));
      // Attention: this passes the ownership of the data to the frame
      m_eventframe.put(std::unique_ptr<podio::CollectionBase>(coll), shortPath);
      m_podio_datawrappers.push_back(wrapper);
    }
  }
  return DataSvc::registerObject(parentPath, fullPath, pObject);
}
