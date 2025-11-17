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
#ifndef FWCORE_TRANSFORMER_H
#define FWCORE_TRANSFORMER_H

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include <GaudiKernel/FunctionalFilterDecision.h>

#include "k4FWCore/FunctionalUtils.h"

// #include "GaudiKernel/CommonMessaging.h"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace k4FWCore {

namespace details {

  template <typename Signature, typename Traits_>
  struct Transformer;

  template <typename Out, typename... In, typename Traits_>
  struct Transformer<Out(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike_v<In> ||
                    std::is_same_v<In, EventContext>) &&
                   ...),
                  "Transformer and Producer input types must be EDM4hep collections or vectors of collections");
    static_assert((std::is_base_of_v<podio::CollectionBase, Out> || isVectorLike_v<Out> ||
                   std::is_same_v<podio::CollectionBase*, Out>),
                  "Transformer and Producer output types must be EDM4hep collections or vectors of collections");

    static constexpr std::size_t N_in = filter_evtcontext<In...>::size;
    static constexpr std::size_t N_out = 1;

    using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

    using KeyValue = base_class::KeyValue;
    using KeyValues = base_class::KeyValues;

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
    template <typename T>
    using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

    wrap_tuple_types<InputHandle_t, filter_evtcontext_t<In...>>::type m_inputs;
    std::tuple<std::vector<OutputHandle_t<EventStoreType_t>>> m_outputs;

    std::array<Gaudi::Property<DataObjID>, N_in> m_inputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, N_in> m_inputLocationsVector;
    Gaudi::Property<DataObjID> m_outputLocationsSingle;
    Gaudi::Property<std::vector<DataObjID>> m_outputLocationsVector;

    template <typename IArgs, typename OArgs, std::size_t... I>
    Transformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
                const OArgs& outputs)
        : base_class(std::move(name), locator),
          // The input locations are filled by creating a property with a
          // callback function that creates the handles because when the
          // callback runs is when the input locations become available (from
          // a steering file, for example) and the handles have to be created
          // for Gaudi to know the data flow
          m_inputLocationsSingle{makeInputPropSingle<InputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_inputLocationsVector{makeInputPropVector<InputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_outputLocationsSingle{makeOutputPropSingle<OutputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<0>(outputs), std::get<0>(m_outputs), this)},
          m_outputLocationsVector{makeOutputPropVector<OutputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<0>(outputs), std::get<0>(m_outputs), this)} {}

    Transformer(std::string name, ISvcLocator* locator,
                Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_in> const& inputs,
                Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_out> const& outputs)
        : Transformer(std::move(name), locator, inputs, std::make_index_sequence<N_in>{}, outputs) {}

    StatusCode execute(const EventContext& ctx) const final {
      try {
        if constexpr (isVectorLike<Out>::value) {
          std::tuple<Out> tmp = filter_evtcontext<In...>::apply(*this, ctx, this->m_inputs);
          putVectorOutputs<0, Out>(std::move(tmp), m_outputs, this);
        } else {
          Gaudi::Functional::details::put(
              std::get<0>(this->m_outputs)[0],
              convertToUniquePtr(std::move(filter_evtcontext<In...>::apply(*this, ctx, this->m_inputs))));
        }
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
      return StatusCode::SUCCESS;
    }

    /**
     * @brief    Get the input locations for a given input index
     * @param i  The index of the input
     * @return   A range of the input locations
     */
    auto inputLocations(const size_t i) const {
      if (i >= N_in) {
        throw std::out_of_range("Called inputLocations with an index out of range, index: " + std::to_string(i) +
                                ", number of inputs: " + std::to_string(N_in));
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

    /**
     * @brief    Get the output locations
     * @return   A range of the output locations
     */
    auto outputLocations() const {
      std::vector<std::string> names;
      if (!m_outputLocationsSingle.name().empty()) {
        names.push_back(m_outputLocationsSingle.value().key());
      } else {
        for (const auto& id : m_outputLocationsVector.value()) {
          names.push_back(id.key());
        }
      }
      return names;
    }
    /**
     * @brief       Get the output locations for a given output name
     * @param name  The name of the output
     * @return      A range of the output locations
     */
    auto outputLocations(std::string_view name) const {
      std::vector<std::string> names;
      if (m_outputLocationsSingle.name() == name) {
        names.push_back(m_outputLocationsSingle.value().key());
      } else if (m_outputLocationsVector.name() == name) {
        for (const auto& id : m_outputLocationsVector.value()) {
          names.push_back(id.key());
        }
      } else {
        throw std::runtime_error("Called outputLocations with an unknown name: " + std::string(name));
      }
      return names;
    }
    static constexpr std::size_t inputLocationsSize() { return N_in; }

    virtual Out operator()(const In&...) const = 0;
  };

  template <typename Signature, typename Traits_>
  struct MultiTransformer;

  template <typename... Out, typename... In, typename Traits_>
  struct MultiTransformer<std::tuple<Out...>(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike<In>::value ||
                    std::is_same_v<In, EventContext>) &&
                   ...),
                  "Transformer and Producer input types must be EDM4hep collections or vectors of collections");
    static_assert(((std::is_base_of_v<podio::CollectionBase, Out> || isVectorLike<Out>::value) && ...),
                  "Transformer and Producer output types must be EDM4hep collections or vectors of collections");

    static constexpr std::size_t N_in = filter_evtcontext<In...>::size;
    static constexpr std::size_t N_out = sizeof...(Out);

    using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

    using KeyValue = base_class::KeyValue;
    using KeyValues = base_class::KeyValues;

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
    template <typename T>
    using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

    wrap_tuple_types<InputHandle_t, filter_evtcontext_t<In...>>::type m_inputs;
    std::tuple<std::vector<OutputHandle_t<typename EventStoreType<Out>::type>>...> m_outputs;

    std::array<Gaudi::Property<DataObjID>, N_in> m_inputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, N_in> m_inputLocationsVector;
    std::array<Gaudi::Property<DataObjID>, N_out> m_outputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, N_out> m_outputLocationsVector;

    template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
    MultiTransformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
                     const OArgs& outputs, std::index_sequence<J...>)
        : base_class(std::move(name), locator),
          m_inputLocationsSingle{makeInputPropSingle<InputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_inputLocationsVector{makeInputPropVector<InputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_outputLocationsSingle{makeOutputPropSingle<OutputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<J>(outputs), std::get<J>(m_outputs), this)...},
          m_outputLocationsVector{makeOutputPropVector<OutputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<J>(outputs), std::get<J>(m_outputs), this)...} {}

    MultiTransformer(std::string name, ISvcLocator* locator,
                     Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_in> const& inputs,
                     Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_out> const& outputs)
        : MultiTransformer(std::move(name), locator, inputs, std::make_index_sequence<N_in>{}, outputs,
                           std::make_index_sequence<N_out>{}) {}

    // derived classes are NOT allowed to implement execute ...
    StatusCode execute(const EventContext& ctx) const final {
      try {
        auto tmp = filter_evtcontext<In...>::apply(*this, ctx, this->m_inputs);
        putVectorOutputs<0, Out...>(std::move(tmp), m_outputs, this);
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    /**
     * @brief    Get the input locations for a given input index
     * @param i  The index of the input
     * @return   A range of the input locations
     */
    auto inputLocations(size_t i) const {
      if (i >= N_in) {
        throw std::out_of_range("Called inputLocations with an index out of range, index: " + std::to_string(i) +
                                ", number of inputs: " + std::to_string(N_in));
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

    /**
     * @brief    Get the output locations for a given output index
     * @param i  The index of the output
     * @return   A range of the output locations
     */
    auto outputLocations(size_t i) const {
      if (i >= N_out) {
        throw std::out_of_range("Called outputLocations with an index out of range");
      }
      std::vector<std::string> names;
      if (!m_outputLocationsSingle[i].name().empty()) {
        names.push_back(m_outputLocationsSingle[i].value().key());
      } else {
        for (const auto& id : m_outputLocationsVector[i].value()) {
          names.push_back(id.key());
        }
      }
      return names;
    }
    /**
     * @brief       Get the output locations for a given output name
     * @param name  The name of the output
     * @return      A range of the output locations
     */
    auto outputLocations(std::string_view name) const {
      std::vector<std::string> names;
      const auto it =
          std::ranges::find_if(m_outputLocationsVector, [&name](const auto& prop) { return prop.name() == name; });
      if (it != m_outputLocationsVector.end()) {
        for (const auto& id : it->value()) {
          names.push_back(id.key());
        }
      } else {
        const auto it2 =
            std::ranges::find_if(m_outputLocationsSingle, [&name](const auto& prop) { return prop.name() == name; });
        if (it2 == m_outputLocationsSingle.end()) {
          throw std::out_of_range("Called outputLocations with a name that does not exist: " + std::string(name));
        } else {
          names.push_back(it2->value().key());
        }
      }
      return names;
    }

    static constexpr std::size_t inputLocationsSize() { return N_in; }
    static constexpr std::size_t outputLocationsSize() { return N_out; }

    // ... instead, they must implement the following operator
    virtual std::tuple<Out...> operator()(const In&...) const = 0;
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using MultiTransformer = details::MultiTransformer<Signature, Traits_>;

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Transformer = details::Transformer<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_TRANSFORMER_H
