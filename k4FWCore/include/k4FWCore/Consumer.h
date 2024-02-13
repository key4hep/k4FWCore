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

#include "podio/CollectionBase.h"

#include "k4FWCore/FunctionalUtils.h"

#include <memory>
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

      std::tuple<InputHandle_t<decltype(transformType(std::declval<In>()))>...>
          m_inputs;
      std::map<std::string, std::vector<InputHandle_t<std::shared_ptr<podio::CollectionBase>>>> m_extraInputs;
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)> m_inputLocations{};
      std::array<Gaudi::Property<DataObjID>, sizeof...(In)>              m_inputLocationsPair{};
      mutable std::map<std::string, std::vector<std::string>>            m_inputLocationsMap;

      using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;
      using InKeys    = Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...(In)>;

      template <typename IArgs, std::size_t... I>
      Consumer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
          : base_class(std::move(name), locator),
            m_inputLocations{Gaudi::Property<std::vector<DataObjID>>{
                this, getName<In>(std::get<I>(inputs), false), {DataObjID{std::get<I>(inputs).second[0]}}}...},
            m_inputLocationsPair{Gaudi::Property<DataObjID>{
                this, getName<In>(std::get<I>(inputs), true), DataObjID{std::get<I>(inputs).second[0]},
                [this](Gaudi::Details::PropertyBase&) {
                  if constexpr (!is_map_like<In>::value) {
                    auto& handle = std::get<I>(m_inputs);
                    auto& ins    = m_inputLocationsPair[I];
                    handle       = InputHandle_t<decltype(transformType(std::declval<In>()))>(ins.value(), this);
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}}...},
            m_inputs{InputHandle_t<decltype(transformType(std::declval<In>()))>(std::get<I>(inputs).first, this)...}

      {
        // if constexpr (std::is_same_v<In, std::map<std::string, std::shared_ptr<podio::CollectionBase>>>) {
        //   // for (auto& value : std::get<I...>(inputs).second) {
        //   //   Gaudi::Algorithm::info() << "Adding extra input " << value << endmsg;
        //   //   m_extraInputs["InputCollection"].emplace_back(InputHandle_t<std::shared_ptr<podio::CollectionBase>>(value, this));
        //   // }
        //   // m_inputs = std::make_tuple( typename maybeVector<In>::type()... );
        //   // m_inputs = std::make_tuple( DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>() );
        //   // auto& handles = std::get<0>(m_inputs);
        //   // handles.push_back(DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>(DataObjID{"InputCollection"}, this));
        // }
      }

      Consumer(std::string name, ISvcLocator* locator,
               Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...(In)> const& inputs)
          : Consumer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}) {}

      // When reading multiple collections we assume that the variable used is a
      // std::map<std::string, std::shared_ptr<podio::CollectionBase>>
      // and read the collections in a space separated string
      template <size_t Index, typename... Handles>
      void transformAndApplyAlgoAtIndex(const std::tuple<Handles...>& handles) const {
        if constexpr (Index < sizeof...(Handles)) {
          if constexpr (is_map_like<std::tuple_element_t<Index, std::tuple<In...>>>::value) {
            using EDM4hepType =
                typename ExtractInnerType<typename std::decay_t<decltype(std::get<Index>(handles))>>::type;
            auto map = std::map<std::string, std::shared_ptr<EDM4hepType>>();

            // To be locked
            if (!m_inputLocationsMap.contains(std::get<Index>(handles).objKey())) {
              auto vec = std::vector<std::string>();
              vec.reserve(m_inputLocations[Index].value().size());
              for (auto& val : m_inputLocations[Index].value()) {
                vec.push_back(val.key());
              }
              m_inputLocationsMap[std::get<Index>(handles).objKey()] = vec;
            }

            for (auto& value : m_inputLocationsMap.at(std::get<Index>(handles).objKey())) {
              DataObject* p;
              auto        sc = this->evtSvc()->retrieveObject(value, p);
              if (!sc.isSuccess()) {
                throw GaudiException("Failed to retrieve object " + value, "Consumer", StatusCode::FAILURE);
              }
              const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(p);
              map[value]            = std::dynamic_pointer_cast<EDM4hepType>(collection->getData());
            }
            std::get<Index>(handles).put(std::move(map));
          }

          // Recursive call for the next index
          transformAndApplyAlgoAtIndex<Index + 1>(handles);
        }
      }

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute(const EventContext& ctx) const override final {
        try {
          transformAndApplyAlgoAtIndex<0>(this->m_inputs);
          filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs);
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
