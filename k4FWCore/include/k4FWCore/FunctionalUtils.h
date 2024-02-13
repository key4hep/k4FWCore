#pragma once

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include "GaudiKernel/CommonMessaging.h"

#include "podio/CollectionBase.h"

#include <stdexcept>
#include <tuple>
#include <type_traits>

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

    static std::atomic<int> i = 0;

    template <typename T, typename K>
    std::string getName(const K& first, bool pair=false) {
      // Gaudi::Algorithm::info() << "T is " << typeid(T).name() << endmsg;
      // Gaudi::Algorithm::info() << "getName " << first.first << endmsg;
      if constexpr (is_map_like<T>::value) {
        if (pair) {
          // Gaudi::Algorithm::info() << "returning " << "_" + std::to_string(i) << endmsg;
          return "_" + std::to_string(i++);
        }
        // Gaudi::Algorithm::info() << "returning " << first.first << endmsg;
        return first.first;
      }
      if (pair) {
        // Gaudi::Algorithm::info() << "returning " << first.first << endmsg;
        return first.first;
      }
      // Gaudi::Algorithm::info() << "returning " << "_" + std::to_string(i) << endmsg;
      return "_" + std::to_string(i++);
    }


    template <typename... Args>
    struct CountType {
      static constexpr size_t value = 0;
    };

    template <typename First, typename... Rest>
    struct CountType<First, Rest...> {
      static constexpr size_t value = (is_map_like<First>::value ? 1 : 0) + CountType<Rest...>::value;
    };

    template <typename T> struct ExtractInnerType;

    // Specialization for DataObjectReadHandle with map and shared_ptr
    template <typename Value>
    struct ExtractInnerType<DataObjectReadHandle<std::map<std::string, std::shared_ptr<Value>>>> {
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
