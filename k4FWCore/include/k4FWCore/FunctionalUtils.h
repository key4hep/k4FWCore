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
    template <typename T, typename P>
    const auto& maybeTransformToEDM4hep(const P& arg) {
      return arg;
    }

    template <typename T, typename P>
    requires std::same_as<P, std::shared_ptr<podio::CollectionBase>>
    const auto& maybeTransformToEDM4hep(const P& arg) {
      return static_cast<const T&>(*arg);
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<podio::CollectionBase, T>, std::shared_ptr<podio::CollectionBase>> transformType(
        const T& arg) {
      return std::shared_ptr<podio::CollectionBase>(arg);
    }
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
