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

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"

// #include "GaudiKernel/CommonMessaging.h"

#include "k4FWCore/FunctionalUtils.h"

#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace k4FWCore {

namespace details {

  template <typename Signature, typename Traits_>
  struct Consumer;

  template <typename... In, typename Traits_>
  struct Consumer<void(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike_v<In>) && ...),
                  "Consumer input types must be EDM4hep collections or vectors of collection pointers");

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, T>;

    std::tuple<std::vector<InputHandle_t<typename EventStoreType<In>::type>>...> m_inputs;

    std::array<Gaudi::Property<DataObjID>, sizeof...(In)> m_inputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)> m_inputLocationsVector;

    template <size_t I>
    Gaudi::Property<DataObjID> make_input_prop_single(const auto& inp) {
      if (inp.index() == 0) {
        const auto& input = std::get<KeyValue>(inp);
        return {Gaudi::Property<DataObjID>(
            this, input.first, to_DataObjID(input.second)[0],
            [this](Gaudi::Details::PropertyBase& p) {
              std::vector<InputHandle_t<EventStoreType_t>> handles;
              auto handle = InputHandle_t<EventStoreType_t>(static_cast<Gaudi::Property<DataObjID>&>(p).value(), this);
              handles.push_back(std::move(handle));
              std::get<I>(m_inputs) = std::move(handles);
            },
            Gaudi::Details::Property::ImmediatelyInvokeHandler{true})};
      } else {
        return Gaudi::Property<DataObjID>{};
      }
    }
    template <size_t I>
    Gaudi::Property<std::vector<DataObjID>> make_input_prop_vector(const auto& inp) {
      // KeyValues
      if (inp.index() == 1) {
        const auto& input = std::get<KeyValues>(inp);
        return {Gaudi::Property<std::vector<DataObjID>>(
            this, input.first, to_DataObjID(input.second),
            [this](Gaudi::Details::PropertyBase& p) {
              const auto& tmpprop = static_cast<Gaudi::Property<std::vector<DataObjID>>&>(p);
              const auto& tmpval = tmpprop.value();
              std::vector<InputHandle_t<EventStoreType_t>> handles;
              handles.reserve(tmpval.size());
              for (const auto& value : tmpval) {
                auto handle = InputHandle_t<EventStoreType_t>(value, this);
                handles.push_back(std::move(handle));
              }
              std::get<I>(m_inputs) = std::move(handles);
            },
            Gaudi::Details::Property::ImmediatelyInvokeHandler{true})};
      } else {
        return Gaudi::Property<std::vector<DataObjID>>{};
      }
    }

    using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

    using KeyValues = typename base_class::KeyValues;
    using KeyValue = typename base_class::KeyValue;

    template <typename IArgs, std::size_t... I>
    Consumer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
        : base_class(std::move(name), locator),
          // The input locations are filled by creating a property with a callback function
          // that creates the handles because that is when the input locations become available
          // (from a steering file, for example) and the handles have to be created for
          // Gaudi to know the data flow
          m_inputLocationsSingle{make_input_prop_single<I>(std::get<I>(inputs))...},
          m_inputLocationsVector{make_input_prop_vector<I>(std::get<I>(inputs))...} {}

    Consumer(std::string name, ISvcLocator* locator,
             const Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, sizeof...(In)>& inputs)
        : Consumer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}) {
    }

    // derived classes are NOT allowed to implement execute ...
    StatusCode execute(const EventContext& ctx) const final {
      try {
        filter_evtcontext_tt<In...>::apply(*this, ctx, m_inputs);
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // ... instead, they must implement the following operator
    virtual void operator()(const In&...) const = 0;

    /**
     * @brief    Get the input locations for a given input index
     * @param i  The index of the input
     * @return   A range of the input locations
     */
    auto inputLocations(size_t i) const {
      if (i >= sizeof...(In)) {
        throw std::out_of_range("Called inputLocations with an index out of range, index: " + std::to_string(i) +
                                ", number of inputs: " + std::to_string(sizeof...(In)));
      }
      return m_inputLocationsVector[i] |
             std::views::transform([](const DataObjID& id) -> const auto& { return id.key(); });
    }
    /**
     * @brief       Get the input locations for a given input name
     * @param name  The name of the input
     * @return      A range of the input locations
     */
    auto inputLocations(std::string_view name) const {
      auto it = std::ranges::find_if(m_inputLocationsVector, [&name](const auto& prop) { return prop.name() == name; });
      if (it == m_inputLocationsVector.end()) {
        throw std::runtime_error("Called inputLocations with an unknown name");
      }
      return it->value() | std::views::transform([](const DataObjID& id) -> const auto& { return id.key(); });
    }
    static constexpr std::size_t inputLocationsSize() { return sizeof...(In); }
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Consumer = details::Consumer<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_CONSUMER_H
