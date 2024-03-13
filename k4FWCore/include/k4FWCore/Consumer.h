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

    template <size_t Index, typename... In, typename... Handles, typename InputTuple>
    void readMapInputss(const std::tuple<Handles...>& handles, auto thisClass, InputTuple& inputTuple) {
      if constexpr (Index < sizeof...(Handles)) {
        if constexpr (is_map_like<std::tuple_element_t<Index, std::tuple<In...>>>::value) {
          // In case of map types like std::map<std::string, edm4hep::MCParticleCollection&>
          // we have to remove the reference to get the actual type
          using EDM4hepType =
              std::remove_reference_t<typename std::tuple_element_t<Index, std::tuple<In...>>::mapped_type>;
          auto inputMap = std::map<std::string, const EDM4hepType&>();
          for (auto& handle : std::get<Index>(handles)) {
            auto in = get(handle, thisClass, Gaudi::Hive::currentContext());
            inputMap.emplace(handle.objKey(), *static_cast<EDM4hepType*>(in.get()));
          }
          std::get<Index>(inputTuple) = std::move(inputMap);

        } else {
          auto in                     = get(std::get<Index>(handles)[0], thisClass, Gaudi::Hive::currentContext());
          std::get<Index>(inputTuple) = static_cast<std::tuple_element_t<Index, std::tuple<In...>>*>(in.get());
        }

        // Recursive call for the next index
        readMapInputss<Index + 1, In...>(handles, thisClass, inputTuple);
      }
    }

    inline std::vector<DataObjID> to_DataObjID(const std::vector<std::string>& in) {
      std::vector<DataObjID> out;
      out.reserve(in.size());
      std::transform(in.begin(), in.end(), std::back_inserter(out), [](const std::string& i) { return DataObjID{i}; });
      return out;
    }

    template <typename... In> struct filter_evtcontext_ttt {
      static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                    "EventContext can only appear as first argument");

      template <typename Algorithm, typename Handles> static auto apply(const Algorithm& algo, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
      }

      template <typename Algorithm, typename Handles>
      static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
        auto inputTuple = std::tuple<SelectType<In>...>();

        // Build the input tuple by picking up either std::map with an arbitrary
        // number of collections or single collections
        readMapInputss<0, In...>(handles, &algo, inputTuple);

        return std::apply(
            [&](const auto&... input) { return algo(maybeTransformToEDM4hep2<decltype(input)>(input)...); },
            inputTuple);
      }
    };

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
          filter_evtcontext_ttt<In...>::apply(*this, ctx, m_inputs);
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
