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
#include "GaudiKernel/StatusCode.h"

#include "podio/CollectionBase.h"
#include "podio/Frame.h"

#include "IIOSvc.h"
#include "k4FWCore/FunctionalUtils.h"

#include <memory>

template <typename Container> using vector_of_          = std::vector<Container>;
template <typename Container> using vector_of_optional_ = std::vector<std::optional<Container>>;

class CollectionPusher : public Gaudi::Functional::details::BaseClass_t<Gaudi::Functional::Traits::useDefaults> {
  using Traits_    = Gaudi::Functional::Traits::useDefaults;
  using Out        = std::shared_ptr<podio::CollectionBase>;
  using base_class = Gaudi::Functional::details::BaseClass_t<Traits_>;
  static_assert(std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm");

public:
  CollectionPusher(std::string name, ISvcLocator* locator) : base_class(std::move(name), locator) {}

  // derived classes can NOT implement execute
  StatusCode execute(const EventContext&) const override final {
    try {
      auto out = (*this)();

      auto outColls        = std::get<std::vector<std::shared_ptr<podio::CollectionBase>>>(out);
      auto outputLocations = std::get<std::vector<std::string>>(out);

      // if (out.size() != m_outputs.size()) {
      //   throw GaudiException("Error during transform: expected " + std::to_string(m_outputs.size()) +
      //                            " containers, got " + std::to_string(out.size()) + " instead",
      //                        this->name(), StatusCode::FAILURE);
      // }
      for (unsigned i = 0; i != outColls.size(); ++i) {
        auto objectp = std::make_unique<AnyDataWrapper<Out>>(std::move(outColls[i]));
        if (auto sc = m_dataSvc->registerObject(outputLocations[i], objectp.get()); sc.isFailure()) {
        }
        // The store has the ownership so we shouldn't delete the object
        auto ptr = objectp.release();
      }
      return Gaudi::Functional::FilterDecision::PASSED;
    } catch (GaudiException& e) {
      (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
      return e.code();
    }
  }

  virtual std::tuple<vector_of_<Out>, std::vector<std::string>> operator()() const = 0;

private:
  // if In is a pointer, it signals optional (as opposed to mandatory) input
  // template <typename T>
  // using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
  // Gaudi::Property<std::vector<DataObjID>> m_inputLocations; // TODO/FIXME: remove this duplication...
  // TODO/FIXME: replace vector of DataObjID property + call-back with a
  //             vector<handle> property ... as soon as declareProperty can deal with that.
  ServiceHandle<IDataProviderSvc> m_dataSvc{this, "EventDataSvc", "EventDataSvc"};
};

class Reader final : public CollectionPusher {
public:
  Reader(const std::string& name, ISvcLocator* svcLoc) : CollectionPusher(name, svcLoc) {}

  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the reader to be deleted so it will call finish()
  // See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/169
  ~Reader() { iosvc->deleteReader(); }

  ServiceHandle<IIOSvc> iosvc{this, "IOSvc", "IOSvc"};

  StatusCode initialize() override {
    if (!iosvc.isValid()) {
      error() << "Unable to locate IIOSvc interface" << endmsg;
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    if (iosvc) {
      iosvc->deleteReader();
    }
    return StatusCode::SUCCESS;
  }

  // The IOSvc takes care of reading and passing the data
  // By convention the Frame is pushed to the store
  // so that it's deleted at the right time
  std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>> operator()()
      const override {
    auto val = iosvc->next();

    auto eds   = eventSvc().as<IDataProviderSvc>();
    auto frame = std::move(std::get<podio::Frame>(val));

    auto tmp  = new AnyDataWrapper<podio::Frame>(std::move(frame));
    auto code = eds->registerObject("/Event" + k4FWCore::frameLocation, tmp);

    return std::make_tuple(std::get<0>(val), std::get<1>(val));
  }
};

DECLARE_COMPONENT(Reader)
