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

#include <GAUDI_VERSION.h>
#include <Gaudi/Functional/details.h>
#include <Gaudi/Functional/utilities.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/FunctionalFilterDecision.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/StatusCode.h>
// #include "GaudiKernel/CommonMessaging.h"

#include <podio/CollectionBase.h>

#include "k4FWCore/FunctionalUtils.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace k4FWCore {

namespace details {

#if GAUDI_MAJOR_VERSION >= 41
  using EmptyTypeList = Gaudi::Functional::details::type_list<>;
#else
  using EmptyTypeList = std::tuple<>;
#endif

  template <typename Signature, typename Traits_>
  struct Consumer;

  template <typename... In, typename Traits_>
  struct Consumer<void(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<EmptyTypeList, EmptyTypeList, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<EmptyTypeList, EmptyTypeList, Traits_>::DataHandleMixin;

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike_v<In> ||
                    std::is_same_v<In, EventContext>) &&
                   ...),
                  "Consumer input types must be EDM4hep collections or vectors of collection pointers");

    static constexpr size_t N_in = filter_evtcontext<In...>::size;

    using base_class = Gaudi::Functional::details::DataHandleMixin<EmptyTypeList, EmptyTypeList, Traits_>;

    using KeyValue = base_class::KeyValue;
    using KeyValues = base_class::KeyValues;

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, T>;

    tuple_of_handle_vec_t<InputHandle_t, filter_evtcontext_t<In...>> m_inputs;

    std::array<Gaudi::Property<DataObjID>, N_in> m_inputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, N_in> m_inputLocationsVector;

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
             const Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_in>& inputs)
        : Consumer(std::move(name), locator, inputs, std::make_index_sequence<N_in>{}) {}

    StatusCode execute(const EventContext& ctx) const final {
      try {
        filter_evtcontext<In...>::apply(*this, ctx, m_inputs);
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    virtual void operator()(const In&...) const = 0;

    /**
     * @brief    Get the input locations for a given input index
     * @param i  The index of the input
     * @return   A range of the input locations
     */
    std::vector<std::string> inputLocations(size_t i) const {
      if (i >= N_in) {
        throw std::out_of_range("Called inputLocations with an index out of range, index: " + std::to_string(i) +
                                ", number of inputs: " + std::to_string(N_in));
      }
      if (!m_inputLocationsSingle[i].name().empty()) {
        return {m_inputLocationsSingle[i].value().key()};
      }
      return dataObjIDKeys(m_inputLocationsVector[i].value());
    }
    /**
     * @brief       Get the input locations for a given input name
     * @param name  The name of the input
     * @return      A range of the input locations
     */
    std::vector<std::string> inputLocations(std::string_view name) const {
      const auto hasName = [name](const auto& prop) { return prop.name() == name; };
      if (const auto it = std::ranges::find_if(m_inputLocationsVector, hasName); it != m_inputLocationsVector.end()) {
        return dataObjIDKeys(it->value());
      }
      if (const auto it = std::ranges::find_if(m_inputLocationsSingle, hasName); it != m_inputLocationsSingle.end()) {
        return {it->value().key()};
      }
      throw std::out_of_range("Called inputLocations with a name that does not exist: " + std::string(name));
    }
    static constexpr std::size_t inputLocationsSize() { return N_in; }
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Consumer = details::Consumer<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_CONSUMER_H
