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
#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include "GaudiKernel/StatusCode.h"

#include "IIOSvc.h"

#include "podio/CollectionBase.h"
#include "podio/Frame.h"

#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <memory>

using podio::CollectionBase;

namespace Gaudi::Functional {
template <typename Container>
using vector_of_ = std::vector<Container>;
template <typename Container>
using vector_of_optional_ = std::vector<std::optional<Container>>;

namespace details {

  class MyTransformer : public BaseClass_t<Gaudi::Functional::Traits::useDefaults> {
    using Traits_ = Gaudi::Functional::Traits::useDefaults;
    using Out = std::shared_ptr<podio::CollectionBase>;
    using base_class = BaseClass_t<Traits_>;
    static_assert(std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm");

  public:
    using KeyValues = std::pair<std::string, std::vector<std::string>>;

    MyTransformer(std::string name, ISvcLocator* locator, const KeyValues& outputs) :
        base_class(std::move(name), locator),
        m_outputLocations(
            this, outputs.first, details::to_DataObjID(outputs.second),
            [this](Gaudi::Details::PropertyBase&) {
              this->m_outputs = details::make_vector_of_handles<decltype(this->m_outputs)>(this, m_outputLocations);
              // if constexpr (details::is_optional_v<Out>) { // handle constructor does not (yet) allow to
              //                                              // set optional flag... so
              //                                              // do it explicitly here...
              //   std::for_each(this->m_outputs.begin(), this->m_outputs.end(), [](auto& h) { h.setOptional(true); });
              // }
            },
            Gaudi::Details::Property::ImmediatelyInvokeHandler{true})
    {}

    // derived classes can NOT implement execute
    StatusCode execute(const EventContext&) const override final {
      // Gaudi::Algorithm::info() << "MyTransformer::execute()" << endmsg;
      try {
        // TODO:FIXME: how does operator() know the number and order of expected outputs?
        auto out = (*this)();
        // if (out.size() != m_outputs.size()) {
        //   throw GaudiException("Error during transform: expected " + std::to_string(m_outputs.size()) +
        //                            " containers, got " + std::to_string(out.size()) + " instead",
        //                        this->name(), StatusCode::FAILURE);
        // }
        for (unsigned i = 0; i != out.size(); ++i) {
          // Gaudi::Algorithm::info() << "MyTransformer::execute() : putting " << m_outputs[i].fullKey() << endmsg;
          details::put(m_outputs[0], std::move(out[i]));
        }
        // details::put(m_frame[0], std::move(frame));
        return FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // TODO/FIXME: how does the callee know in which order to produce the outputs?
    //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
    //              and only those entries which contain an Out are stored)
    virtual vector_of_<Out> operator()() const = 0;

  private:
    // if In is a pointer, it signals optional (as opposed to mandatory) input
    // template <typename T>
    // using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
    // Gaudi::Property<std::vector<DataObjID>> m_inputLocations; // TODO/FIXME: remove this duplication...
    // TODO/FIXME: replace vector of DataObjID property + call-back with a
    //             vector<handle> property ... as soon as declareProperty can deal with that.
    template <typename T>
    using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
    std::vector<OutputHandle<Out>> m_outputs;
    Gaudi::Property<std::vector<DataObjID>> m_outputLocations;
    // Gaudi::Property<std::vector<DataObjID>> m_outputFrameLocations;
    // std::vector<OutputHandle<std::shared_ptr<CollectionBase>>> m_frame{};

  };

} // namespace details

} // namespace Gaudi::Functional

using Gaudi::Functional::details::MyTransformer;

class Reader final : public MyTransformer {
// class Reader final : public MyTransformer<std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::shared_ptr<CollectionBase>>()> {
public:
  Reader(const std::string& name, ISvcLocator* svcLoc) :
    MyTransformer(name, svcLoc, {"OutputLocations", {"MCParticless"}}
                  
                  ) {
  }

  // Gaudi doesn't run the destructor of the Services so we have to
  // manually ask for the reader to be deleted so it will call finish()
  ~Reader() {
    iosvc->deleteReader();
  }

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

  std::vector<std::shared_ptr<podio::CollectionBase>> operator()() const override {
    info() << "Reader::operator()" << endmsg;
    auto val = iosvc->next();

    auto eds = eventSvc().as<IDataProviderSvc>();
    auto frame = std::move(std::get<1>(val));

    // We'll hand the ownership of 
    auto tmp = new AnyDataWrapper<podio::Frame>(std::move(frame));
    auto code = eds->registerObject("/Event/_Frame", tmp);


    return std::get<0>(val);
  }

};

DECLARE_COMPONENT(Reader)
