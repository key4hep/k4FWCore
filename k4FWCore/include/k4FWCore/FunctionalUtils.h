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
#pragma once

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"

#include "podio/CollectionBase.h"

#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>

namespace k4FWCore {

  static const std::string frameLocation = "/_Frame";

  namespace details {

    // This function will be used to modify std::shared_ptr<podio::CollectionBase> to the actual collection type
    template <typename T, typename P> const auto& maybeTransformToEDM4hep(const P& arg) { return arg; }

    template <typename T, typename P>
      requires std::same_as<P, std::shared_ptr<podio::CollectionBase>>
    const auto& maybeTransformToEDM4hep(const P& arg) {
      return static_cast<const T&>(*arg);
    }

    template <typename T> struct is_map_like : std::false_type {};

    template <typename Value> struct is_map_like<std::map<std::string, Value>> : std::true_type {};

    inline std::string hash_string(const std::string& str) {
      std::string result = "_";
      for (auto& c : std::to_string(std::hash<std::string>{}(str))) {
        result += 'a' + (c - '0');
      }
      return result;
    }

    template <typename T, typename K> std::string getName(const K& first, bool pair = false) {
      if constexpr (is_map_like<T>::value) {
        if (pair) {
          return hash_string(first.first);
        }
        return first.first;
      }
      if (pair) {
        return first.first;
      }
      return hash_string(first.first);
    }

    template <typename T> struct ExtractInnerType;

    template <typename Value>
    struct ExtractInnerType<DataObjectReadHandle<std::map<std::string, std::shared_ptr<Value>>>> {
      using type = Value;
    };

    template <typename Value> struct ExtractInnerType<std::map<std::string, std::shared_ptr<Value>>> {
      using type = Value;
    };

    template <typename T>
    std::enable_if_t<std::is_base_of_v<podio::CollectionBase, T>, std::shared_ptr<podio::CollectionBase>> transformType(
        const T& arg) {
      return std::shared_ptr<podio::CollectionBase>(arg);
    }

    // template <typename T>
    // std::enable_if_t<std::is_base_of_v<podio::CollectionBase, T>, std::map<std::string, std::shared_ptr<T>>>
    // transformType(const std::map<std::string, std::shared_ptr<T>>& arg) {
    //   return std::map<std::string, std::shared_ptr<T>>();
    // }

    template <typename T>
    std::enable_if_t<!std::is_base_of_v<podio::CollectionBase, T>, T> transformType(const T& arg) {
      // Default: no transformation
      return arg;
    }

    template <typename T, std::enable_if_t<!std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
    auto ptrOrCast(T&& arg) {
      // return arg;
      return std::shared_ptr<podio::CollectionBase>(std::make_shared<T>(std::move(arg)));
    }
    template <typename T, std::enable_if_t<std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
    auto ptrOrCast(T&& arg) {
      // return arg;
      std::cout << "Calling static_cast<const T&>(*arg) (ptrOrCast)" << std::endl;
      return static_cast<const T&>(*arg);
    }

    template <typename... In> struct filter_evtcontext_tt {
      static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                    "EventContext can only appear as first argument");

      template <typename Algorithm, typename Handles> static auto apply(const Algorithm& algo, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
      }

      template <typename Algorithm, typename Handles>
      static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
        return std::apply(
            [&](const auto&... handle) { return algo(maybeTransformToEDM4hep<In>(get(handle, algo, ctx))...); },
            handles);
      }
    };

  }  // namespace details
}  // namespace k4FWCore
