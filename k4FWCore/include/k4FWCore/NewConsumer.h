// /*
//  * Copyright (c) 2014-2024 Key4hep-Project.
//  *
//  * This file is part of Key4hep.
//  * See https://key4hep.github.io/key4hep-doc/ for further info.
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */
// #ifndef FWCORE_CONSUMER_H
// #define FWCORE_CONSUMER_H

// #include <GaudiKernel/FunctionalFilterDecision.h>
// #include "Gaudi/Functional/details.h"
// #include "Gaudi/Functional/utilities.h"

// #include "podio/CollectionBase.h"

// #include "k4FWCore/FunctionalUtils.h"

// #include "GaudiKernel/CommonMessaging.h"

// #include <type_traits>
// #include <utility>
// #include <concepts>

// namespace k4FWCore {

//   namespace details {

//     template <typename F, size_t... Is>
//     auto for_impl( F&& f, std::index_sequence<Is...> ) {
//       // if constexpr ( std::disjunction_v<std::is_void<std::invoke_result_t<F, std::integral_constant<int, Is>>>...> ) {
//       //   ( std::invoke( f, std::integral_constant<int, Is>{} ), ... );
//       // } else {
//         return std::array{ std::invoke( f, std::integral_constant<int, Is>{} )... };
//       // }
//     }

//     template <auto N, typename F>
//     decltype( auto ) for_( F&& f ) {
//       return for_impl( std::forward<F>( f ), std::make_index_sequence<N>{} );
//     }

//     template <typename Signature, typename Traits_> struct Consumer;

//     template <typename... In, typename Traits_>
//     struct Consumer<void(const In&...), Traits_>
//         : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
//     private:
//       using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;
//       template <typename T>
//       using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;

//       template <typename T>
//       struct maybeVector {
//         using type = std::vector<InputHandle_t<T>>;
//       };

//       template <>
//       struct maybeVector<std::map<std::string, std::shared_ptr<podio::CollectionBase>>> {
//         using type = InputHandle_t<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>;
//       };

//       std::tuple<maybeVector<In>...> m_inputs; //   and make the handles properties instead...
//       std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...( In )> m_inputLocations; // TODO/FIXME: remove

//     public:
//       using KeyValue  = typename base_class::KeyValue;
//       using KeyValues = typename base_class::KeyValues;
//       using InKeys     = Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...( In )>;

//     private:
//       auto construct_properties( InKeys inputs ) {
//         return details::for_<sizeof...( In )>( [&]( auto I ) {
//           constexpr auto i   = decltype( I )::value;
//           auto&          ins = std::get<i>( inputs );
//           return Gaudi::Property<std::vector<DataObjID>>{
//             this, ins.first, Gaudi::Functional::details::to_DataObjID( ins.second ),
//               [this]( auto&& ) {
//                 auto& handles = std::get<i>( this->m_inputs );
//                 auto& ins     = std::get<i>( this->m_inputLocations );
//                 using Handles = typename std::decay_t<decltype( handles )>;
//                 handles       = make_vector_of_handles<Handles>( this, ins );
//                 // if ( std::is_pointer_v<typename Handles::value_type> ) { // handle constructor does not (yet) allow to
//                 //                                                          // set
//                 //                                                          // optional flag... so do it
//                 //                                                          // explicitly here...
//                 //   std::for_each( handles.begin(), handles.end(), []( auto& h ) { h.setOptional( true ); } );
//                 // }
//               },
//               Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } };
//         } );
//       }
//     public:

//       Consumer( std::string name, ISvcLocator* locator, InKeys inputs )
//           : base_class( std::move( name ), locator ),
//             m_inputLocations{ construct_properties( inputs ) } {
//       }

//       Consumer(std::string name, ISvcLocator* locator, const KeyValues& inputs)
//         : Consumer{std::move(name), locator, InKeys{inputs}} {
//       }

//       // When reading multiple collections we assume that the variable used is a
//       // std::map<std::string, std::shared_ptr<podio::CollectionBase>>
//       // and read the collections in a space separated string
//       template <size_t Index, typename... Handles>
//       void transformAndApplyAlgoAtIndex(const std::tuple<Handles...>& handles) const {
//         if constexpr (Index < sizeof...(Handles)) {
//           using HandleType = std::decay_t<decltype(std::get<Index>(handles))>;

//           if constexpr (std::is_same_v<
//                             HandleType,
//                             DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>>) {
//             // Transformation logic for the specific type
//             auto               map = std::map<std::string, std::shared_ptr<podio::CollectionBase>>();
//             std::istringstream ss(std::get<Index>(handles).objKey());
//             std::string        token;
//             while (ss >> token) {
//               DataObject* p;
//               auto        sc = this->evtSvc()->retrieveObject(token, p);
//               if (!sc.isSuccess()) {
//                 throw GaudiException("Failed to retrieve object " + token, "Consumer", StatusCode::FAILURE);
//               }
//               const auto collection = dynamic_cast<AnyDataWrapper<std::shared_ptr<podio::CollectionBase>>*>(p);
//               map[token]            = collection->getData();
//             }
//             std::get<Index>(handles).put(std::move(map));
//           }

//           // Recursive call for the next index
//           transformAndApplyAlgoAtIndex<Index + 1>(handles);
//         }
//       }

//       // derived classes are NOT allowed to implement execute ...
//       StatusCode execute(const EventContext& ctx) const override final {
//         try {
//           transformAndApplyAlgoAtIndex<0>(this->m_inputs);
//           filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs);
//           return Gaudi::Functional::FilterDecision::PASSED;
//         } catch (GaudiException& e) {
//           (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
//           return e.code();
//         }
//       }

//       // ... instead, they must implement the following operator
//       virtual void operator()(const In&...) const = 0;
//     };

//   }  // namespace details

//   template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
//   using Consumer = details::Consumer<Signature, Traits_>;

// }  // namespace k4FWCore

// #endif  // FWCORE_CONSUMER_H

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

#include <GaudiKernel/DataObjectHandle.h>
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
        : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
      using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

      template <typename T>
      using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;

      std::tuple<InputHandle_t<decltype(transformType(std::declval<In>()))>...> m_inputs;  //   and make the handles properties instead...
      std::map<std::string, std::vector<InputHandle_t<std::shared_ptr<podio::CollectionBase>>>> m_extraInputs;
      // Gaudi::Property<std::vector<DataObjID>>                                      m_inputLocations;
      // std::map<std::string, Gaudi::Property<std::vector<DataObjID>>>                                      m_inputLocations;
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...(In)> m_inputLocations{};
      mutable std::map<std::string, std::vector<std::string>>            m_inputLocationsMap;

      using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;
      using InKeys    = Gaudi::Functional::details::RepeatValues_<KeyValues, sizeof...(In)>;

      // Consumer(std::string name, ISvcLocator* locator, const std::vector<KeyValues>& inputs)
      template <typename IArgs, std::size_t... I>
      Consumer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
          : base_class(std::move(name), locator),
            m_inputLocations{Gaudi::Property<std::vector<DataObjID>>{
                this,
                std::get<I>(inputs).first,
                {DataObjID{std::get<I>(inputs).second[0]}},
                [this](Gaudi::Details::PropertyBase&) {
                  Gaudi::Algorithm::info() << "Inside the property handler" << endmsg;
                  if constexpr (!std::is_same_v<std::decay_t<decltype(std::get<I>(m_inputs))>,
                                                DataObjectReadHandle<
                                                    std::map<std::string, std::shared_ptr<podio::CollectionBase>>>>) {
                    auto& handle  = std::get<I>(m_inputs);
                    auto& ins     = m_inputLocations[I];
                    handle        = InputHandle_t<decltype(transformType(std::declval<In>()))>(ins.value()[0], this);
                    Gaudi::Algorithm::info() << "Input location (inside lambda): " << ins << endmsg;
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true}
            }
        ...},
            // m_inputs{std::tuple<InputHandle_t<decltype(transformType<In>...)>>(InputHandle_t<decltype(transformType<In>...)>(std::get<I>(inputs).first, this)...)}
            m_inputs{InputHandle_t<decltype(transformType(std::declval<In>()))>(std::get<I>(inputs).first, this)...}

      {
        Gaudi::Algorithm::info() << "Creating Consumer with " << sizeof...(In) << " inputs" << endmsg;
        for (auto& val : std::get<0>(inputs).second) {
          Gaudi::Algorithm::info() << "Input location: " << val << endmsg;
        }
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

      constexpr static std::size_t N_in = sizeof...(In);

      Consumer(std::string name, ISvcLocator* locator,
               Gaudi::Functional::details::RepeatValues_<KeyValues, N_in> const& inputs)
          : Consumer(std::move(name), locator, inputs, std::index_sequence_for<In...>{}) {}

      // When reading multiple collections we assume that the variable used is a
      // std::map<std::string, std::shared_ptr<podio::CollectionBase>>
      // and read the collections in a space separated string
      template <size_t Index, typename... Handles>
      void transformAndApplyAlgoAtIndex(const std::tuple<Handles...>& handles) const {
        if constexpr (Index < sizeof...(Handles)) {
          if constexpr (std::is_same_v<
                            std::decay_t<decltype(std::get<Index>(handles))>,
                            DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>>) {
            auto map = std::map<std::string, std::shared_ptr<podio::CollectionBase>>();

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
              map[value]            = collection->getData();
            }
            std::get<Index>(handles).put(std::move(map));
          }

          // Recursive call for the next index
          transformAndApplyAlgoAtIndex<Index + 1>(handles);
        }
      }

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute(const EventContext& ctx) const override final {
        for (auto& prop : m_inputLocations) {
          Gaudi::Algorithm::info() << "Input location: " << prop << endmsg;
        }
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
