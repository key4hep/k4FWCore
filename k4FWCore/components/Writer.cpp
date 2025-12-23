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
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/StatusCode.h"

#include "podio/Frame.h"

#include "IIOSvc.h"

#include "k4FWCore/DataWrapper.h"
#include "k4FWCore/FunctionalUtils.h"
#include "k4FWCore/IMetadataSvc.h"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

class Writer final : public Gaudi::Functional::Consumer<void(const EventContext&)> {
public:
  Writer(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {
    // Non-reentrant algorithms have a cardinality of 1
    setProperty("Cardinality", 1).ignore();
  }

  bool isReEntrant() const override { return false; }

  // Many members are mutable because it's assumed that the Writer is called only once
  mutable std::set<std::string> m_availableCollections;
  // These are the collections that are not in the frame and we want to add to the frame
  mutable std::vector<std::string> m_collectionsToAdd;
  // These are the collections we want to save to the frame
  mutable std::vector<std::string> m_collectionsToSave;

  ServiceHandle<IIOSvc> iosvc{this, "IOSvc", "IOSvc"};
  SmartIF<IHiveWhiteBoard> m_hiveWhiteBoard;
  SmartIF<IDataProviderSvc> m_dataSvc;
  SmartIF<IMetadataSvc> m_metadataSvc;
  mutable bool m_first{true};

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

    m_hiveWhiteBoard = service("EventDataSvc", true);
    if (!m_hiveWhiteBoard) {
      debug() << "Unable to locate IHiveWhiteBoard interface. This isn't a problem if we are not running in a "
                 "multi-threaded environment"
              << endmsg;
    }

    m_metadataSvc = service("MetadataSvc", false);
    if (!m_metadataSvc) {
      error() << "Unable to locate MetadataSvc service" << endmsg;
      return StatusCode::FAILURE;
    }

    podio::Frame config_metadata_frame;

    //// prepare job options metadata ///////////////////////
    // retrieve the configuration of the job
    // and write it to file as vector of strings
    std::vector<std::string> config_data;
    for (const auto& per_property : Gaudi::svcLocator()->getOptsSvc().items()) {
      // sample output:
      // HepMCToEDMConverter.genparticles = "GenParticles"
      // Note that quotes are added to all property values,
      // which leads to problems with ints, lists, dicts and bools.
      // For these types, the quotes must be removed in postprocessing.
      config_data.emplace_back(std::get<0>(per_property) + " = \"" + std::get<1>(per_property) + "\"\n");
    }

    config_metadata_frame.putParameter("gaudiConfigOptions", config_data);
    if (const char* env_key4hep_stack = std::getenv("KEY4HEP_STACK")) {
      config_metadata_frame.putParameter("key4hepstack", env_key4hep_stack);
    }
    iosvc->getWriter().writeFrame(config_metadata_frame, "configuration_metadata");

    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    if (const auto* metadata_frame = m_metadataSvc->getFrame(); metadata_frame) {
      iosvc->getWriter().writeFrame(*metadata_frame, podio::Category::Metadata);
    }

    iosvc->deleteWriter();

    return StatusCode::SUCCESS;
  }

  std::set<std::pair<std::string, std::string_view>> getOutputCollections() const {
    SmartIF<IDataManagerSvc> mgr;
    mgr = eventSvc();

    SmartDataPtr<DataObject> root(eventSvc(), "/Event");
    if (!root) {
      error() << "Failed to retrieve root object /Event" << endmsg;
      return {};
    }

    auto pObj = root->registry();
    if (!pObj) {
      error() << "Failed to retrieve the root registry object" << endmsg;
      return {};
    }
    std::vector<IRegistry*> leaves;
    StatusCode sc = mgr->objectLeaves(pObj, leaves);
    if (!sc.isSuccess()) {
      error() << "Failed to retrieve object leaves" << endmsg;
      return {};
    }

    std::set<std::pair<std::string, std::string_view>> namesAndTypes{};

    for (const auto& pReg : leaves) {
      if (pReg->name() == k4FWCore::frameLocation) {
        continue;
      }
      // info() << "Found leaf: " << pReg->name() << endmsg;
      /// We are only interested in leaves with an object
      // if (!pReg->address() || !pReg->object()) {
      //   info() << "Leaf " << pReg->name() << " has no object" << endmsg;
      //   continue;
      // }
      auto collName = pReg->name().substr(1, pReg->name().size() - 1);
      const auto coll = dynamic_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(pReg->object());
      std::string_view collType = "";
      if (coll) {
        collType = coll->getData()->getTypeName();
      } else {
        // Things stored via DataHandles need special casing
        if (const auto oldColl = dynamic_cast<DataWrapperBase*>(pReg->object())) {
          if (!oldColl->collectionBase()) {
            debug() << collName << " does not have a valid CollectionBase. " << collName << " will be skipped."
                    << endmsg;
            continue;
          }
          collType = oldColl->collectionBase()->getTypeName();
        }
      }

      if (collType.empty()) {
        warning() << "Could not determine type for collection: " << collName << ". Type based selection will not work"
                  << endmsg;
        continue;
      }
      debug() << "Adding " << collName << " (type: " << collType << ") to the list of available collections" << endmsg;
      namesAndTypes.emplace(std::move(collName), collType);
    }
    return namesAndTypes;
  }

  void operator()(const EventContext& ctx) const override {
    if (m_hiveWhiteBoard) {
      // It's never set to valid but it has the slot information
      // if (ctx.valid()) {
      //   info() << "No context found in Writer" << endmsg;
      //   return;
      // }
      debug() << "Setting store to " << ctx.slot() << endmsg;
      if (m_hiveWhiteBoard->selectStore(ctx.slot()).isFailure()) {
        error() << "Error when setting store" << endmsg;
        throw GaudiException("Error when setting store", name(), StatusCode::FAILURE);
      }
    }

    DataObject* p;
    StatusCode code = m_dataSvc->retrieveObject("/Event" + k4FWCore::frameLocation, p);
    std::unique_ptr<AnyDataWrapper<podio::Frame>> ptr;
    // This is the case when we are reading from a file
    // Putting it into a unique_ptr will make sure it's deleted
    if (code.isSuccess()) {
      const auto sc = m_dataSvc->unregisterObject(p);
      if (!sc.isSuccess()) {
        error() << "Failed to unregister object" << endmsg;
        return;
      }
      ptr = std::unique_ptr<AnyDataWrapper<podio::Frame>>(dynamic_cast<AnyDataWrapper<podio::Frame>*>(p));
    }
    // This is the case when no reading is being done
    else {
      ptr = std::make_unique<AnyDataWrapper<podio::Frame>>(podio::Frame());
    }

    const auto& frameCollections = ptr->getData().getAvailableCollections();
    if (m_first) {
      // Assume all the output collections are the same for all events
      // and cache them
      for (const auto& [name, type] : getOutputCollections()) {
        const auto doWrite = iosvc->checkIfWriteCollection(name, type);
        debug() << "Checking if " << name << " (type=" << type << ") should be written: " << (doWrite ? "yes" : "no")
                << endmsg;
        if (doWrite) {
          m_collectionsToSave.push_back(name);
          if (std::find(frameCollections.begin(), frameCollections.end(), name) == frameCollections.end()) {
            debug() << name << " has to be added to the Frame" << endmsg;
            m_collectionsToAdd.push_back(name);
          }
        }
      }
      m_first = false;
    }

    // Remove the collections owned by a Frame (if any) so that they are not
    // deleted by the store (and later deleted by the Frame, triggering a double
    // delete)
    for (const auto& coll : frameCollections) {
      debug() << "Taking ownership of collection " << coll << " from the IOSvc as it belongs to a Frame" << endmsg;
      DataObject* storeCollection;
      if (m_dataSvc->retrieveObject("/Event/" + coll, storeCollection).isFailure()) {
        debug() << "Failed to retrieve collection " << coll << " from data service" << endmsg;
        continue;
      }
      // We take ownership back from the store
      if (m_dataSvc->unregisterObject(storeCollection).isFailure()) {
        error() << "Failed to unregister collection " << coll << " from data service" << endmsg;
        return;
      }
      // We still have to delete the AnyDataWrapper to avoid a leak
      const auto storePtr = dynamic_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(storeCollection);
      // Assign to an unused variable to silence the warning about not using the
      // result of release()
      [[maybe_unused]] auto releasedPtr = storePtr->getData().release();
      delete storePtr;
    }

    std::vector<std::string_view> collectionsToRemove;
    for (const auto& coll : m_collectionsToAdd) {
      debug() << "Adding collection " << coll << " to the IOSvc Frame" << endmsg;
      DataObject* storeCollection;
      if (m_dataSvc->retrieveObject("/Event/" + coll, storeCollection).isFailure()) {
        debug() << "Failed to retrieve collection " << coll << " from the data service" << endmsg;
        continue;
      }
      // We take ownership back from the store
      if (m_dataSvc->unregisterObject(storeCollection).isFailure()) {
        debug() << "Failed to unregister collection " << coll << " from the data service" << endmsg;
        continue;
      }
      const auto collection = dynamic_cast<AnyDataWrapper<std::unique_ptr<podio::CollectionBase>>*>(storeCollection);
      if (collection) {
        ptr->getData().put(std::move(collection->getData()), coll);
        delete collection;
      } else {
        // Check the case when the data has been produced using the old DataHandle
        const auto old_collection = dynamic_cast<DataWrapperBase*>(storeCollection);
        if (!old_collection) {
          // This can happen for objects that are not collections like in the
          // MarlinWrapper for converter maps or a LCEvent, or, in general,
          // anything else
          info() << "Object in the store with name " << coll
                 << " does not look like a collection so it can not be written to the output file" << endmsg;
          // Collections to remove in m_collectionsToAdd are saved for later
          // not to modify the vector while iterating over it
          collectionsToRemove.push_back(coll);
          m_collectionsToSave.erase(std::remove(m_collectionsToSave.begin(), m_collectionsToSave.end(), coll),
                                    m_collectionsToSave.end());
        } else {
          std::unique_ptr<podio::CollectionBase> uptr(
              const_cast<podio::CollectionBase*>(old_collection->collectionBase()));
          ptr->getData().put(std::move(uptr), coll);
        }
      }
    }

    for (const auto& coll : collectionsToRemove) {
      m_collectionsToAdd.erase(std::remove(m_collectionsToAdd.begin(), m_collectionsToAdd.end(), coll),
                               m_collectionsToAdd.end());
    }

    debug() << "Writing frame" << endmsg;
    iosvc->getWriter().writeFrame(ptr->getData(), podio::Category::Event, m_collectionsToSave);
  }
};

DECLARE_COMPONENT(Writer)
