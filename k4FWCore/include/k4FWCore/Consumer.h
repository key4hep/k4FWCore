/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include <GaudiKernel/FunctionalFilterDecision.h>

#include "podio/CollectionBase.h"

#include "k4FWCore/FunctionalUtils.h"

#include <type_traits>
#include <utility>

namespace k4FWCore {

namespace details {

  template <typename Signature, typename Traits_>
  struct Consumer;

  template <typename... In, typename Traits_>
  struct Consumer<void(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<
            std::tuple<>, Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>,
            Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<
        std::tuple<>, Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>,
        Traits_>::DataHandleMixin;

    // derived classes are NOT allowed to implement execute ...
    StatusCode execute(const EventContext& ctx) const override final {
      try {
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

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Consumer = details::Consumer<Signature, Traits_>;

} // namespace k4FWCore
