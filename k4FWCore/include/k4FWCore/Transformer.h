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

#include <GaudiKernel/FunctionalFilterDecision.h>
#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"

#include "k4FWCore/FunctionalUtils.h"

// #include "GaudiKernel/CommonMessaging.h"

#include <type_traits>
#include <utility>

namespace k4FWCore {

  namespace details {

    template <typename Signature, typename Traits_> struct Transformer;

    template <typename Out, typename... In, typename Traits_>
    struct Transformer<Out(const In&...), Traits_>
        : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
      using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

      static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isMapToCollLike<In>::value) && ...),
                    "Transformer and Producer input types must be EDM4hep collections or maps to collections");
      static_assert((std::is_base_of_v<podio::CollectionBase, Out> || isMapToCollLike<Out>::value),
                    "Transformer and Producer output types must be EDM4hep collections or maps to collections");

      template <typename T>
      using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      template <typename T>
      using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

      std::tuple<std::vector<InputHandle_t<typename transformType<In>::type>>...> m_inputs;
      std::tuple<std::vector<OutputHandle_t<typename transformType<Out>::type>>>  m_outputs;
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)>          m_inputLocations{};
      std::array<Gaudi::Property<std::vector<DataObjID>>, 1>                      m_outputLocations{};

      using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

      using KeyValues = typename base_class::KeyValues;

      template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
      Transformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
                  const OArgs& outputs, std::index_sequence<J...>)
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
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...},
            m_outputLocations{Gaudi::Property<std::vector<DataObjID>>{
                this, std::get<J>(outputs).first, to_DataObjID(std::get<J>(outputs).second),
                [this](Gaudi::Details::PropertyBase&) {
                  std::vector<OutputHandle_t<typename transformType<Out>::type>> h;
                  std::sort(this->m_outputLocations[J].value().begin(), this->m_outputLocations[J].value().end(),
                            [](const DataObjID& a, const DataObjID& b) { return a.key() < b.key(); });
                  for (auto& inpID : this->m_outputLocations[J].value()) {
                    if (inpID.key().empty()) {
                      continue;
                    }
                    auto handle = OutputHandle_t<typename transformType<Out>::type>(inpID, this);
                    h.push_back(std::move(handle));
                  }
                  std::get<0>(m_outputs) = std::move(h);
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...} {}

      constexpr static std::size_t N_in  = sizeof...(In);
      constexpr static std::size_t N_out = 1;

      Transformer(std::string name, ISvcLocator* locator,
                  Gaudi::Functional::details::RepeatValues_<KeyValues, N_in> const&  inputs,
                  Gaudi::Functional::details::RepeatValues_<KeyValues, N_out> const& outputs)
          : Transformer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}, outputs,
                        std::index_sequence_for<Out>{}) {}

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute(const EventContext& ctx) const override final {
        try {
          if constexpr (isMapToCollLike<Out>::value) {
            std::tuple<Out> tmp = filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs);
            // auto tmp = filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs);
            putMapOutputs<0, Out>(std::move(tmp), m_outputs, this);
            // int i = 0;
            // for (auto& [key, val] : filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs)) {
            //   if (key != m_outputs[i].objKey()) {
            //     throw GaudiException("Transformer",
            //                          "Output key does not match the expected key " + m_outputs[i].objKey(),
            //                          StatusCode::FAILURE);
            //   }
            //   Gaudi::Functional::details::put(m_outputs[i], convertToSharedPtr(std::move(val)));
            //   i++;
            // }
          } else {
            Gaudi::Functional::details::put(
                std::get<0>(this->m_outputs)[0],
                convertToSharedPtr(std::move(filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs))));
          }
          return Gaudi::Functional::FilterDecision::PASSED;
        } catch (GaudiException& e) {
          (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual Out operator()(const In&...) const = 0;
    };

    template <typename Signature, typename Traits_> struct MultiTransformer;

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformer<std::tuple<Out...>(const In&...), Traits_>
        : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
      using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

      static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isMapToCollLike<In>::value) && ...),
                    "Transformer and Producer input types must be EDM4hep collections or maps to collections");
      static_assert(((std::is_base_of_v<podio::CollectionBase, Out> || isMapToCollLike<Out>::value) && ...),
                    "Transformer and Producer output types must be EDM4hep collections or maps to collections");

      template <typename T>
      using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      template <typename T>
      using OutputHandle_t = Gaudi::Functional::details::OutputHandle_t<Traits_, std::remove_pointer_t<T>>;

      std::tuple<std::vector<InputHandle_t<typename transformType<In>::type>>...>   m_inputs;
      std::tuple<std::vector<OutputHandle_t<typename transformType<Out>::type>>...> m_outputs;
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)>            m_inputLocations{};
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(Out)>           m_outputLocations{};

      using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

      using KeyValues = typename base_class::KeyValues;

      template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
      MultiTransformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
                       const OArgs& outputs, std::index_sequence<J...>)
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
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...},
            m_outputLocations{Gaudi::Property<std::vector<DataObjID>>{
                this, std::get<J>(outputs).first, to_DataObjID(std::get<J>(outputs).second),
                [this](Gaudi::Details::PropertyBase&) {
                  std::vector<OutputHandle_t<typename transformType<Out>::type>> h;
                  std::sort(this->m_outputLocations[J].value().begin(), this->m_outputLocations[J].value().end(),
                            [](const DataObjID& a, const DataObjID& b) { return a.key() < b.key(); });
                  for (auto& inpID : this->m_outputLocations[J].value()) {
                    if (inpID.key().empty()) {
                      continue;
                    }
                    auto handle = OutputHandle_t<typename transformType<Out>::type>(inpID, this);
                    h.push_back(std::move(handle));
                  }
                  std::get<J>(m_outputs) = std::move(h);
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...} {}

      constexpr static std::size_t N_in  = sizeof...(In);
      constexpr static std::size_t N_out = sizeof...(Out);

      MultiTransformer(std::string name, ISvcLocator* locator,
                       Gaudi::Functional::details::RepeatValues_<KeyValues, N_in> const&  inputs,
                       Gaudi::Functional::details::RepeatValues_<KeyValues, N_out> const& outputs)
          : MultiTransformer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}, outputs,
                             std::index_sequence_for<Out...>{}) {}

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute(const EventContext& ctx) const override final {
        try {
          auto tmp = filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs);
          putMapOutputs<0, Out...>(std::move(tmp), m_outputs, this);
          return Gaudi::Functional::FilterDecision::PASSED;
        } catch (GaudiException& e) {
          (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()(const In&...) const = 0;
    };

  }  // namespace details

  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
  using MultiTransformer = details::MultiTransformer<Signature, Traits_>;

  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
  using Transformer = details::Transformer<Signature, Traits_>;

}  // namespace k4FWCore

#endif  // FWCORE_TRANSFORMER_H
