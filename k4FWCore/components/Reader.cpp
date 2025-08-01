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
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class Reader : public Gaudi::Functional::details::BaseClass_t<Gaudi::Functional::Traits::useDefaults> {
  using Traits_ = Gaudi::Functional::Traits::useDefaults;
  using Out = std::unique_ptr<podio::CollectionBase>;
  using base_class = Gaudi::Functional::details::BaseClass_t<Traits_>;
  static_assert(std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm");

  template <typename T>
  using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

public:
  Reader(const std::string& name, ISvcLocator* svcLoc)
      : base_class(name, svcLoc), m_inputCollections{this,
                                                     "InputCollections",
                                                     {},
                                                     [this](Gaudi::Details::PropertyBase&) {
                                                       const std::string cmd = System::cmdLineArgs()[0];
                                                       if (cmd.find("genconf") != std::string::npos) {
                                                         return;
                                                       }
                                                       for (const auto& c : m_inputCollections.value()) {
                                                         m_outputs.emplace_back(c, this);
                                                       }
                                                     },
                                                     Gaudi::Details::Property::ImmediatelyInvokeHandler{true}},
        m_iosvc{this, "IOSvc", "IOSvc"}, m_dataSvc{this, "EventDataSvc", "EventDataSvc"} {
    setProperty("Cardinality", 1).ignore();
  }

  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the reader to be deleted so it will call finish()
  // See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/169
  ~Reader() override { m_iosvc->deleteReader(); }

  bool isReEntrant() const override { return false; }

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

  StatusCode execute(const EventContext&) const final {
    try {
      const auto readData = nextCollections();

      auto readCollections = std::get<0>(readData);

      for (size_t i = 0; i != readCollections.size(); ++i) {
        m_outputs[i].put(std::unique_ptr<podio::CollectionBase>(readCollections[i]));
      }
      return Gaudi::Functional::FilterDecision::PASSED;
    } catch (GaudiException& e) {
      (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
      return e.code();
    }
  }

private:
  // The IOSvc takes care of reading and passing the data
  // By convention the Frame is pushed to the store
  // so that it's deleted at the right time
  std::tuple<std::vector<podio::CollectionBase*>, std::vector<std::string>> nextCollections() const {

    auto nextData = m_iosvc->next();

    const auto tmpWrapper = new AnyDataWrapper<podio::Frame>(std::move(std::get<podio::Frame>(nextData)));
    const auto eds = eventSvc().as<IDataProviderSvc>();
    if (eds->registerObject("/Event" + k4FWCore::frameLocation, tmpWrapper).isFailure()) {
      error() << "Failed to register Frame object" << endmsg;
    }

    return std::make_tuple(std::get<0>(nextData), std::get<1>(nextData));
  }

  Gaudi::Property<std::vector<std::string>> m_inputCollections;
  mutable std::vector<OutputHandle_t<Out>> m_outputs;
  ServiceHandle<IIOSvc> m_iosvc;
  ServiceHandle<IDataProviderSvc> m_dataSvc;
};

DECLARE_COMPONENT(Reader)
