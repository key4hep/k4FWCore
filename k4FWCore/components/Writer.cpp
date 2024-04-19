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

#include "Gaudi/Functional/Consumer.h"
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/StatusCode.h"

#include "podio/Frame.h"

#include "IIOSvc.h"

#include "k4FWCore/DataWrapper.h"
#include "k4FWCore/FunctionalUtils.h"

#include <GaudiKernel/IHiveWhiteBoard.h>
#include <memory>

class Writer final : public Gaudi::Functional::Consumer<void(const EventContext&)> {
public:
  Writer(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {
    // Non-reentrant algorithms have a cardinality of 1
    setProperty("Cardinality", 1).ignore();
  }

  mutable Gaudi::Property<std::vector<std::string>> m_OutputNames{this, "CollectionNames", {}};
  mutable std::set<std::string>                     m_availableCollections;
  mutable std::vector<std::string>                  m_collectionsToAdd;
  mutable std::vector<std::string>                  m_collectionsRemaining;
  mutable std::vector<std::string>                  m_collectionsToSave;

  mutable std::mutex m_mutex;

  ServiceHandle<IIOSvc>     iosvc{this, "IOSvc", "IOSvc"};
  SmartIF<IHiveWhiteBoard>  m_hiveWhiteBoard;
  SmartIF<IDataProviderSvc> m_dataSvc;
  mutable bool              m_first{true};

  StatusCode initialize() override {
    if (!iosvc.isValid()) {
      error() << "Unable to locate IIOSvc interface" << endmsg;
      return StatusCode::FAILURE;
    }

    m_dataSvc = service("EventDataSvc", true);
    if (!m_dataSvc) {
      error() << "Unable to locate EventDataSvc service" << endmsg;
      return StatusCode::FAILURE;
    }

    m_hiveWhiteBoard = service<IHiveWhiteBoard>("EventDataSvc", true);
    if (!m_hiveWhiteBoard) {
      debug() << "Unable to locate IHiveWhiteBoard interface. This isn't a problem if we are not running in a "
                 "multi-threaded environment"
              << endmsg;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    podio::Frame config_metadata_frame;

    //// prepare job options metadata ///////////////////////
    // retrieve the configuration of the job
    // and write it to file as vector of strings
    std::vector<std::string> config_data;
    for (const auto& per_property : Gaudi::svcLocator()->getOptsSvc().items()) {
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
    for (const auto* name : {"NTupleSvc"}) {
      std::stringstream config_stream;
      auto              svc = service<IProperty>(name);
      if (!svc.isValid())
        continue;
      for (const auto* property : svc->getProperties()) {
        config_stream << name << "." << property->name() << " = \"" << property->toString() << "\";" << std::endl;
      }
      config_data.push_back(config_stream.str());
    }

    config_metadata_frame.putParameter("gaudiConfigOptions", config_data);
    if (auto env_key4hep_stack = std::getenv("KEY4HEP_STACK")) {
      config_metadata_frame.putParameter("key4hepstack", env_key4hep_stack);
    }
    iosvc->getWriter()->writeFrame(config_metadata_frame, "configuration_metadata");

    iosvc->deleteWriter();
    return StatusCode::SUCCESS;
  }

  void getOutputCollections() const {
    SmartIF<IDataManagerSvc> m_mgr;
    m_mgr = eventSvc();

    SmartDataPtr<DataObject> root(eventSvc(), "/Event");
    if (!root) {
      error() << "Failed to retrieve root object /Event" << endmsg;
      return;
    }

    auto pObj = root->registry();
    if (!pObj) {
      error() << "Failed to retrieve the root registry object" << endmsg;
      return;
    }
    auto mgr = eventSvc().as<IDataManagerSvc>();
    if (!mgr) {
      error() << "Failed to retrieve IDataManagerSvc" << endmsg;
      return;
    }
    std::vector<IRegistry*> leaves;
    StatusCode              sc = m_mgr->objectLeaves(pObj, leaves);
    if (!sc.isSuccess()) {
      error() << "Failed to retrieve object leaves" << endmsg;
      return;
    }
    for (auto& pReg : leaves) {
      if (pReg->name() == k4FWCore::frameLocation) {
        continue;
      }
      // info() << "Found leaf: " << pReg->name() << endmsg;
      /// We are only interested in leaves with an object
      // if (!pReg->address() || !pReg->object()) {
      //   info() << "Leaf " << pReg->name() << " has no object" << endmsg;
      //   continue;
      // }
      m_availableCollections.insert(pReg->name().substr(1, pReg->name().size() - 1));
      const std::string& id = pReg->identifier();
    }
  }

  void operator()(const EventContext& ctx) const override {
    // It seems that even when setting Cardinality to 1,
    // more than one instance is created
    std::scoped_lock<std::mutex> lock(m_mutex);


    if (m_hiveWhiteBoard) {
      // It's never set to valid but it has the slot information
      // if (ctx.valid()) {
      //   info() << "No context found in Writer" << endmsg;
      //   return;
      // }
      info() << "Setting store to " << ctx.slot() << endmsg;
      if (m_hiveWhiteBoard->selectStore(ctx.slot()).isFailure()) {
        error() << "Error when setting store" << endmsg;
        throw GaudiException("Error when setting store", name(), StatusCode::FAILURE);
      }
    }

    DataObject* p;
    StatusCode code = m_dataSvc->retrieveObject("/Event" + k4FWCore::frameLocation, p);
    AnyDataWrapper<podio::Frame>* ptr;
    // This is the case when we are reading from a file
    if (code.isSuccess()) {
      m_dataSvc->unregisterObject(p).ignore();
      ptr  = dynamic_cast<AnyDataWrapper<podio::Frame>*>(p);
    }
    // This is the case when no reading is being done
    // needs to be fixed? (new without delete)
    else {
      ptr = new AnyDataWrapper<podio::Frame>(podio::Frame());
    }

    if (m_first) {
      // Assume all the output collections are the same for all events
      // and cache them
      getOutputCollections();
      for (const auto& coll : m_availableCollections) {
        if (iosvc->checkIfWriteCollection(coll)) {
          m_collectionsToSave.push_back(coll);
          const auto& frameCollections = ptr->getData().getAvailableCollections();
          if (std::find(frameCollections.begin(), frameCollections.end(), coll) == frameCollections.end()) {
            m_collectionsToAdd.push_back(coll);
          } else {
            m_collectionsRemaining.push_back(coll);
          }
        }
      }

      m_first = false;
    }

    for (auto& coll : ptr->getData().getAvailableCollections()) {
      DataObject* storeCollection;
      if (m_dataSvc->retrieveObject("/Event/" + coll, storeCollection).isFailure()) {
        error() << "Failed to retrieve collection " << coll << endmsg;
        return;
      }
      // We take ownership back from the store
      if (m_dataSvc->unregisterObject(storeCollection).isFailure()) {
        error() << "Failed to unregister collection " << coll << endmsg;
        return;
      }
    }

    for (auto& coll : m_collectionsToAdd) {
      DataObject* storeCollection;
      if (m_dataSvc->retrieveObject("/Event/" + coll, storeCollection).isFailure()) {
        error() << "Failed to retrieve collection " << coll << endmsg;
        return;
      }
      // We take ownership back from the store
      if (m_dataSvc->unregisterObject(storeCollection).isFailure()) {
        error() << "Failed to unregister collection " << coll << endmsg;
        return;
      }
      const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(storeCollection);
      if (!collection) {
        // Check the case when the data has been produced using the old DataHandle
        const auto old_collection = dynamic_cast<DataWrapperBase*>(storeCollection);
        if (!old_collection) {
          error() << "Failed to cast collection " << coll << endmsg;
          return;
        } else {
          std::unique_ptr<podio::CollectionBase> uptr(const_cast<podio::CollectionBase*>(old_collection->collectionBase()));
          ptr->getData().put(std::move(uptr), coll);
        }

      } else {
        std::unique_ptr<podio::CollectionBase> uptr(collection->getData().get());
        ptr->getData().put(std::move(uptr), coll);
      }
    }

    debug() << "Writing frame, with time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count()
            << endmsg;
    iosvc->getWriter()->writeFrame(ptr->getData(), podio::Category::Event, m_collectionsToSave);
  }
};

DECLARE_COMPONENT(Writer)
