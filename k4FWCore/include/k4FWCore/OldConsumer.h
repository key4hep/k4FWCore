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

#include "GaudiKernel/CommonMessaging.h"

#include <type_traits>
#include <utility>

namespace k4FWCore {

  namespace details {

    template <typename Signature, typename Traits_> struct Consumer;

    template <typename... In, typename Traits_>
    struct Consumer<void(const In&...), Traits_>
        : Gaudi::Functional::details::DataHandleMixin<
              std::tuple<>,
              Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>, Traits_> {
      using Gaudi::Functional::details::DataHandleMixin<
          std::tuple<>, Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>,
          Traits_>::DataHandleMixin;

      // When reading multiple collections we assume that the variable used is a
      // std::map<std::string, std::shared_ptr<podio::CollectionBase>>
      // and read the collections in a space separated string
      template <size_t Index, typename... Handles>
      void transformAndApplyAlgoAtIndex(const std::tuple<Handles...>& handles) const {
        if constexpr (Index < sizeof...(Handles)) {
          using HandleType = std::decay_t<decltype(std::get<Index>(handles))>;

          if constexpr (std::is_same_v<
                            HandleType,
                            DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>>) {
            // Transformation logic for the specific type
            auto               map = std::map<std::string, std::shared_ptr<podio::CollectionBase>>();
            std::istringstream ss(std::get<Index>(handles).objKey());
            std::string        token;
            while (ss >> token) {
              DataObject* p;
              auto sc = this->evtSvc()->retrieveObject(token, p);
              if (!sc.isSuccess()) {
                throw GaudiException("Failed to retrieve object " + token, "Consumer", StatusCode::FAILURE);
              }
              const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(p);
              map[token]            = collection->getData();
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
