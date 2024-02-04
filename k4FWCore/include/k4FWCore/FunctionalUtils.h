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

  template <typename T, typename P, std::enable_if_t<!std::is_same_v<std::shared_ptr<podio::CollectionBase>, P>, int> = 0>
  const auto& transformIfDerivedFromBase(const P& arg) {
    return arg;
  }
  template <typename T, typename P, std::enable_if_t<std::is_same_v<std::shared_ptr<podio::CollectionBase>, P>, int> = 0>
  const auto& transformIfDerivedFromBase(const P& arg) {
    return static_cast<const T&>(*arg);
  }


  template <typename T>
  std::enable_if_t<std::is_base_of_v<podio::CollectionBase, T>, std::shared_ptr<podio::CollectionBase>> transformType(const T& arg) {
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


  template <typename... In>
  struct filter_evtcontext_tt {

    static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                  "EventContext can only appear as first argument");

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, Handles& handles) {
      return std::apply(
          [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
    }

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
      return std::apply(
          [&](const auto&... handle) { return algo(transformIfDerivedFromBase<In>(get(handle, algo, ctx))...); },
          handles);
    }
  };

  template <typename... In>
  struct filter_evtcontext_ttt {

    static_assert(!std::disjunction_v<std::is_same<EventContext, In>...>,
                  "EventContext can only appear as first argument");

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, Handles& handles) {
      return std::apply(
          [&](const auto&... handle) { return algo(get(handle, algo, Gaudi::Hive::currentContext())...); }, handles);
    }

    template <typename Algorithm, typename Handles>
    static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
      if constexpr (sizeof...(In) == 1) {
        return algo(*reinterpret_cast<std::map<std::string, std::shared_ptr<podio::CollectionBase>>*>(std::get<0>(handles).get()));
      }
    // return std::apply(
    //     [&](const auto& firstHandle, const auto&... otherHandles) {
    //         // Do nothing for the first handle (SpecificType)
    //       if constexpr (!std::is_same_v<std::decay_t<decltype(firstHandle)>, std::map<std::string, std::shared_ptr<podio::CollectionBase>>>) {
    //             // Perform the transformation for other handles
    //         return algo(firstHandle,
    //                         transformIfDerivedFromBase<In>(get(otherHandles, algo, ctx))...);
    //         }
    //     },
    //     handles);
}

  //   template <typename Algorithm, typename Handles>
  //   static auto apply(const Algorithm& algo, const EventContext& ctx, Handles& handles) {
  //     if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(handles))>, DataObjectReadHandle<std::map<std::string, std::shared_ptr<podio::CollectionBase>>>>) {
  //       return; 
  //     }
  //     return std::apply(
  //         [&](const auto&... handle) { return algo(transformIfDerivedFromBase<In>(get(handle, algo, ctx))...); },
  //         handles);
  //   }
  };


} // namespace details
} // namespace k4FWCore
