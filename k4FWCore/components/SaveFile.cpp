/* * Copyright (c) 2014-2023 Key4hep-Project.
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
#include "edm4hep/MCParticleCollection.h"

#include "IIOSvc.h"

#include "k4FWCore/DataWrapper.h"

#include <memory>
#include <thread>

class SaveToFile final : public Gaudi::Functional::Consumer<void()> {
public:
  SaveToFile(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {
  }

  mutable Gaudi::Property<std::vector<std::string>> m_OutputNames{this, "CollectionNames", {}};
  mutable std::set<std::string> m_availableCollections;

  SmartIF<IIOSvc> iosvc;
  SmartIF<IDataProviderSvc> datasvc;
  mutable bool m_first {true};

  StatusCode initialize() override {
    iosvc = service("IOSvc", true);
    if (!iosvc) {
      error() << "Unable to locate IIOSvc interface" << endmsg;
      return StatusCode::FAILURE;
    }

    datasvc = service("EventDataSvc", true);
    if (!datasvc) {
      error() << "Unable to locate IDataSvc interface" << endmsg;
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    iosvc->deleteWriter();
    return StatusCode::SUCCESS;
  }

  void getOutputCollections(IRegistry* pObj) const {
    SmartIF<IDataManagerSvc> m_mgr;
    m_mgr = eventSvc();
    if (!pObj) {
      error() << "Failed to retrieve root object" << endmsg;
      return;
    }
    auto mgr = eventSvc().as<IDataManagerSvc>();
    if (!mgr) {
      error() << "Failed to retrieve IDataManagerSvc" << endmsg;
      return;
    }
    std::vector<IRegistry*> leaves;
    StatusCode sc = m_mgr->objectLeaves(pObj, leaves);
    if (!sc.isSuccess()) {
      error() << "Failed to retrieve object leaves" << endmsg;
      return;
    }
    for (auto& pReg : leaves) {
      if (pReg->name() == "/_Frame") {
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

  void operator()() const override {
    if (m_first) {
      SmartDataPtr<DataObject> root(eventSvc(), "/Event");
      if (!root) {
        info() << "Failed to retrieve root object /Event" << endmsg;
        return;
      }
      getOutputCollections(root->registry());

      m_first = false;
    }

    DataObject *p;
    auto code = datasvc->retrieveObject("/Event/Frame", p);
    AnyDataWrapper<podio::Frame>* ptr;
    if (code.isSuccess()) {
      code = datasvc->unregisterObject(p);
      ptr = dynamic_cast<AnyDataWrapper<podio::Frame>*>(p);
    }
    else {
      ptr = new AnyDataWrapper<podio::Frame>(podio::Frame());
    }
    for (auto& coll : m_availableCollections) {
      DataObject* p;
      auto code = datasvc->retrieveObject("/Event/" + coll, p);
      if (code.isFailure()) {
        error() << "Failed to retrieve collection " << coll << endmsg;
        return;
      }
      // We take ownership back from the store
      code = datasvc->unregisterObject(p);
      if (code.isFailure()) {
        error() << "Failed to unregister collection " << coll << endmsg;
        return;
      }
      info() << "Retrieved collection " << coll << endmsg;
      const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(p);
      if (!collection) {

        // Check the case when the data has been produced using the old DataHandle
        const auto old_collection = dynamic_cast<DataWrapper<podio::CollectionBase>*>(p);
        if (!old_collection) {
          error() << "Failed to cast collection " << coll << endmsg;
          return;
        }
        else {
          std::unique_ptr<podio::CollectionBase> uptr(const_cast<podio::CollectionBase*>(old_collection->getData()));
          ptr->getData().put(std::move(uptr), coll);
        }

      }
      else {
        std::unique_ptr<podio::CollectionBase> uptr(collection->getData().get());
        ptr->getData().put(std::move(uptr), coll);
      }
    }

    // ptr->addRef();
    iosvc->getWriter()->writeFrame(ptr->getData(), podio::Category::Event);

  }
};

DECLARE_COMPONENT(SaveToFile)
