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
#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StatusCode.h"

#include "podio/CollectionBase.h"
#include "podio/Frame.h"

#include "IIOSvc.h"
#include "k4FWCore/FunctionalUtils.h"

#include <memory>

class CollectionPusher : public Gaudi::Functional::details::BaseClass_t<Gaudi::Functional::Traits::useDefaults> {
  using Traits_ = Gaudi::Functional::Traits::useDefaults;
  using Out = std::unique_ptr<podio::CollectionBase>;
  using base_class = Gaudi::Functional::details::BaseClass_t<Traits_>;
  static_assert(std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm");

  template <typename T>
  using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

protected:
  std::vector<OutputHandle_t<Out>> m_outputs;

  Gaudi::Property<std::vector<std::string>> m_inputCollections{
      this, "InputCollections", {}, "List of input collections"};

public:
  CollectionPusher(std::string name, ISvcLocator* locator)
      : base_class(std::move(name), locator),
        m_inputCollections{this,
                           "InputCollections",
                           {},
                           [this](Gaudi::Details::PropertyBase&) {
                             const std::string cmd = System::cmdLineArgs()[0];
                             if (cmd.find("genconf") != std::string::npos) {
                               return;
                             }

                             for (auto& c : m_inputCollections.value()) {
                               m_outputs.emplace_back(c, this);
                             }
                           },
                           Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

  // derived classes can NOT implement execute
  StatusCode execute(const EventContext&) const final {
    try {
      auto out = (*this)();

      auto outColls = std::get<std::vector<podio::CollectionBase*>>(out);
      auto outputLocations = std::get<std::vector<std::string>>(out);

      // if (out.size() != m_outputs.size()) {
      //   throw GaudiException("Error during transform: expected " + std::to_string(m_outputs.size()) +
      //                            " containers, got " + std::to_string(out.size()) + " instead",
      //                        this->name(), StatusCode::FAILURE);
      // }
      for (size_t i = 0; i != outColls.size(); ++i) {
        m_outputs[i].put(std::unique_ptr<podio::CollectionBase>(outColls[i]));
      }
      return Gaudi::Functional::FilterDecision::PASSED;
    } catch (GaudiException& e) {
      (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
      return e.code();
    }
  }

  virtual std::tuple<std::vector<podio::CollectionBase*>, std::vector<std::string>> operator()() const = 0;

private:
  ServiceHandle<IDataProviderSvc> m_dataSvc{this, "EventDataSvc", "EventDataSvc"};
};

class Reader final : public CollectionPusher {
public:
  Reader(const std::string& name, ISvcLocator* svcLoc) : CollectionPusher(name, svcLoc) {
    setProperty("Cardinality", 1).ignore();
  }

  bool isReEntrant() const override { return false; }

  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the reader to be deleted so it will call finish()
  // See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/169
  ~Reader() override { m_iosvc->deleteReader(); }

  ServiceHandle<IIOSvc> m_iosvc{this, "IOSvc", "IOSvc"};

  StatusCode initialize() override {
    if (!m_iosvc.isValid()) {
      error() << "Unable to locate IIOSvc interface" << endmsg;
      return StatusCode::FAILURE;
    }

    if (m_inputCollections.empty()) {
      for (const std::string& collName : m_iosvc->getAvailableCollections()) {
        m_outputs.emplace_back(collName, this);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    if (m_iosvc) {
      m_iosvc->deleteReader();
    }
    return StatusCode::SUCCESS;
  }

  // The IOSvc takes care of reading and passing the data
  // By convention the Frame is pushed to the store
  // so that it's deleted at the right time
  std::tuple<std::vector<podio::CollectionBase*>, std::vector<std::string>> operator()() const override {
    auto val = m_iosvc->next();

    auto eds = eventSvc().as<IDataProviderSvc>();
    auto frame = std::move(std::get<podio::Frame>(val));

    auto tmp = new AnyDataWrapper<podio::Frame>(std::move(frame));
    if (eds->registerObject("/Event" + k4FWCore::frameLocation, tmp).isFailure()) {
      error() << "Failed to register Frame object" << endmsg;
    }

    return std::make_tuple(std::get<0>(val), std::get<1>(val));
  }
};

DECLARE_COMPONENT(Reader)
