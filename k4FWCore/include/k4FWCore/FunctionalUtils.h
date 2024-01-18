#pragma once

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"

#include "podio/CollectionBase.h"

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
      // Transformation logic for types derived from podio::CollectionBase
      // For example:
      // return /* Transformation logic for derived types */;
      // Replace /* Transformation logic for derived types */ with your specific transformation logic
    return std::shared_ptr<podio::CollectionBase>(arg);
  }
  // Transformation function for types not derived from podio::CollectionBase
  template <typename T>
  std::enable_if_t<!std::is_base_of_v<podio::CollectionBase, T>, T> transformType(const T& arg) {
      // Default: no transformation
      // return arg;
      return std::shared_ptr<podio::CollectionBase>(arg);
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
      // return std::apply( [&]( const auto&... handle ) {
      // printType(*get( handle, algo, ctx )...);
      //   return algo( static_cast<const
      // edm4hep::MCParticleCollection&>(*get( handle, algo, ctx ))... ); }, handles );
      return std::apply(
          [&](const auto&... handle) { return algo(transformIfDerivedFromBase<In>(get(handle, algo, ctx))...); },
          handles);
    }
  };
} // namespace details
} // namespace k4FWCore
