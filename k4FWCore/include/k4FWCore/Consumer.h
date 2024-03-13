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
#ifndef FWCORE_CONSUMER_H
#define FWCORE_CONSUMER_H

#include <GaudiKernel/FunctionalFilterDecision.h>
#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"

// #include "GaudiKernel/CommonMessaging.h"

#include "k4FWCore/FunctionalUtils.h"

#include <type_traits>
#include <utility>

namespace k4FWCore {

  namespace details {

    template <typename Signature, typename Traits_> struct Consumer;

    template <typename... In, typename Traits_>
    struct Consumer<void(const In&...), Traits_>
        : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
      using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

      template <typename T>
      using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;

      std::tuple<std::vector<InputHandle_t<typename transformType<In>::type>>...> m_inputs;
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)>          m_inputLocations{};

      using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

      using KeyValues = typename base_class::KeyValues;

      template <typename IArgs, std::size_t... I>
      Consumer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
          : base_class(std::move(name), locator),
            m_inputLocations{Gaudi::Property<std::vector<DataObjID>>{
                this, std::get<I>(inputs).first, to_DataObjID(std::get<I>(inputs).second),
                [this](Gaudi::Details::PropertyBase&) {
                  std::vector<InputHandle_t<typename transformType<In>::type>> h;
                  for (auto& value : this->m_inputLocations[I].value()) {
                    auto handle = InputHandle_t<typename transformType<In>::type>(value, this);
                    h.push_back(std::move(handle));
                  }
                  std::get<I>(m_inputs) = std::move(h);
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...}
      {}

      Consumer(std::string name, ISvcLocator* locator,
               Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...(In)> const& inputs)
          : Consumer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}) {}

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute(const EventContext& ctx) const override final {
        try {
          Gaudi::Algorithm::info() << "Executing " << this->name() << endmsg;
          filter_evtcontext_tt<In...>::apply(*this, ctx, m_inputs);
          return Gaudi::Functional::FilterDecision::PASSED;
        } catch (GaudiException& e) {
          (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual void operator()(const In&...) const = 0;
    };

  }  // namespace details

  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
  using Consumer = details::Consumer<Signature, Traits_>;

}  // namespace k4FWCore

#endif  // FWCORE_CONSUMER_H
