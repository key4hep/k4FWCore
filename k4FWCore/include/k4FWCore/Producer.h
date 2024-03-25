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

#include "FunctionalUtils.h"
#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include <GaudiKernel/CommonMessaging.h>
#include <GaudiKernel/FunctionalFilterDecision.h>

#include "podio/CollectionBase.h"

#include "k4FWCore/FunctionalUtils.h"
#include "k4FWCore/Transformer.h"

#include <type_traits>
#include <utility>



namespace k4FWCore {

  namespace details {

    template <typename Signature, typename Traits_>
    struct Producer;

    template <typename... Out, typename Traits_>
    struct Producer<std::tuple<Out...>(), Traits_> : MultiTransformer<std::tuple<Out...>(), Traits_> {
      using MultiTransformer<std::tuple<Out...>(), Traits_>::MultiTransformer;
    };

    template <typename Out, typename Traits_>
    struct Producer<Out(), Traits_> : Transformer<Out(), Traits_> {
      using Transformer<Out(), Traits_>::Transformer;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
  using Producer = details::Producer<Signature, Traits_>;

} // namespace k4FWCore
