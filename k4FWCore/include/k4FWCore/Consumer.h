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

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike_v<In>)&&...),
                  "Consumer input types must be EDM4hep collections or vectors of collection pointers");

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, T>;

    std::tuple<std::vector<InputHandle_t<typename EventStoreType<In>::type>>...> m_inputs;
    std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)> m_inputLocations{};

    using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

    using KeyValues = typename base_class::KeyValues;

    template <typename IArgs, std::size_t... I>
    Consumer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
        : base_class(std::move(name), locator),
          // The input locations are filled by creating a property with a callback function
          // that creates the handles because that is when the input locations become available
          // (from a steering file, for example) and the handles have to be created for
          // Gaudi to know the data flow
          m_inputLocations{Gaudi::Property<std::vector<DataObjID>>{
              this, std::get<I>(inputs).first, to_DataObjID(std::get<I>(inputs).second),
              [this](Gaudi::Details::PropertyBase&) {
                std::vector<InputHandle_t<EventStoreType_t>> handles;
                for (auto& value : this->m_inputLocations[I].value()) {
                  auto handle = InputHandle_t<EventStoreType_t>(value, this);
                  handles.push_back(std::move(handle));
                }
                std::get<I>(m_inputs) = std::move(handles);
              },
              Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...} {}

    Consumer(std::string name, ISvcLocator* locator,
             Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...(In)> const& inputs)
        : Consumer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}) {}

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
      return m_inputLocations[i] | std::views::transform([](const DataObjID& id) -> const auto& { return id.key(); });
    }
    /**
     * @brief       Get the input locations for a given input name
     * @param name  The name of the input
     * @return      A range of the input locations
     */
    auto inputLocations(std::string_view name) const {
      auto it = std::ranges::find_if(m_inputLocations, [&name](const auto& prop) { return prop.name() == name; });
      if (it == m_inputLocations.end()) {
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
