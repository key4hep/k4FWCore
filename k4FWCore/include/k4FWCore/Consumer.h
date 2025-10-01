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

#include <algorithm>
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
          m_inputLocationsSingle{makeInputPropSingle<InputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_inputLocationsVector{makeInputPropVector<InputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...} {}

    Consumer(std::string name, ISvcLocator* locator,
             const Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, sizeof...(In)>& inputs)
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
      std::vector<std::string> names;
      if (!m_inputLocationsSingle[i].name().empty()) {
        names.push_back(m_inputLocationsSingle[i].value().key());
      } else {
        for (const auto& id : m_inputLocationsVector[i].value()) {
          names.push_back(id.key());
        }
      }
      return names;
    }
    /**
     * @brief       Get the input locations for a given input name
     * @param name  The name of the input
     * @return      A range of the input locations
     */
    auto inputLocations(std::string_view name) const {
      std::vector<std::string> names;
      const auto it =
          std::ranges::find_if(m_inputLocationsVector, [&name](const auto& prop) { return prop.name() == name; });
      if (it != m_inputLocationsVector.end()) {
        for (const auto& id : it->value()) {
          names.push_back(id.key());
        }
      } else {
        const auto it2 =
            std::ranges::find_if(m_inputLocationsSingle, [&name](const auto& prop) { return prop.name() == name; });
        if (it2 == m_inputLocationsSingle.end()) {
          throw std::out_of_range("Called inputLocations with a name that does not exist: " + std::string(name));
        } else {
          names.push_back(it2->value().key());
        }
      }
      return names;
    }
    static constexpr std::size_t inputLocationsSize() { return sizeof...(In); }
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Consumer = details::Consumer<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_CONSUMER_H
