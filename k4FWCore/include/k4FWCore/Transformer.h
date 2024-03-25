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

#include <type_traits>
#include <utility>

namespace k4FWCore {

namespace details {

  template <typename T, std::enable_if_t<!std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
  auto transformm(T&& arg) {
    // return arg;
    return std::shared_ptr<podio::CollectionBase>(std::make_shared<T>(std::move(arg)));
  }
  template <typename T, std::enable_if_t<std::is_same_v<std::shared_ptr<podio::CollectionBase>, T>, int> = 0>
  auto transformm(T&& arg) {
    // return arg;
    std::cout << "Calling static_cast<const T&>(*arg) (transformm)" << std::endl;
    return static_cast<const T&>(*arg);
  }


  template <typename Signature, typename Traits_>
  struct Transformer;

  template <typename Out, typename... In, typename Traits_>
  struct Transformer<Out(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<decltype(transformType(std::declval<Out>()))>,
                                                    Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<
        std::tuple<decltype(transformType(std::declval<Out>()))>, Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>, Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute(const EventContext& ctx) const override final {
      try {
        if constexpr (sizeof...(In) == 0) {
          Gaudi::Functional::details::put(std::get<0>(this->m_outputs), transformm((*this)()));
        } else if constexpr (std::tuple_size_v<Gaudi::Functional::details::filter_evtcontext<In...>> == 0) {
          Gaudi::Functional::details::put(std::get<0>(this->m_outputs), (*this)(ctx));
        } else {
          Gaudi::Functional::details::put(std::get<0>(this->m_outputs),
              transformm(std::move(filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs)))
                                              );
        }
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // instead they MUST implement this operator
    virtual Out operator()(const In&...) const = 0;
  };

  //
  // general N -> M algorithms
  //
  template <typename Signature, typename Traits_>
  struct MultiTransformer;

  template <typename... Out, typename... In, typename Traits_>
  struct MultiTransformer<std::tuple<Out...>(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<decltype(transformType(std::declval<Out>()))...>,
                                                    Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<
        std::tuple<decltype(transformType(std::declval<Out>()))...>, Gaudi::Functional::details::filter_evtcontext<decltype(transformType(std::declval<In>()))...>, Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute(const EventContext& ctx) const override final {
      try {
        GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
        std::apply(
            [this, &ctx](auto&... ohandle) {
              if constexpr (sizeof...(In) == 0) {
                std::apply(
                    [&ohandle...](auto&&... data) {
                      (Gaudi::Functional::details::put(ohandle, transformm(std::forward<decltype(data)>(data))), ...);
                    },
                    (*this)());
              } else if constexpr (std::tuple_size_v<Gaudi::Functional::details::filter_evtcontext<In...>> == 0) {
                std::apply(
                    [&ohandle...](auto&&... data) {
                      (Gaudi::Functional::details::put(ohandle, transformm(std::forward<decltype(data)>(data))), ...);
                    },
                    (*this)(ctx));
              } else {
                std::apply(
                    [&ohandle...](auto&&... data) {
                      // (Gaudi::Functional::details::put(ohandle, std::forward<decltype(data)>(data)), ...);
                      (Gaudi::Functional::details::put(ohandle, transformm(std::forward<decltype(data)>(data))), ...);
                    },
                    filter_evtcontext_tt<In...>::apply(*this, ctx, this->m_inputs));
              }
            },
            this->m_outputs);
        GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
        return Gaudi::Functional::FilterDecision::PASSED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // instead they MUST implement this operator
    virtual std::tuple<Out...> operator()(const In&...) const = 0;
  };

  //
  // general N -> M algorithms with filter functionality
  //
  template <typename Signature, typename Traits_, bool isLegacy>
  struct MultiTransformerFilter;

  template <typename... Out, typename... In, typename Traits_>
  struct MultiTransformerFilter<std::tuple<Out...>(const In&...), Traits_, true>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<Out...>,
                                                    Gaudi::Functional::details::filter_evtcontext<In...>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<
        std::tuple<Out...>, Gaudi::Functional::details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute() override final {
      try {
        return std::apply(
                   [&](auto&... ohandle) {
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
                     return std::apply(
                         [&ohandle...](bool passed, auto&&... data) {
                           (Gaudi::Functional::details::put(ohandle, std::forward<decltype(data)>(data)), ...);
                           return passed;
                         },
                         Gaudi::Functional::details::filter_evtcontext_t<In...>::apply(*this, this->m_inputs));
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
                   },
                   this->m_outputs)
            ? Gaudi::Functional::FilterDecision::PASSED
            : Gaudi::Functional::FilterDecision::FAILED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // instead they MUST implement this operator
    virtual std::tuple<bool, Out...> operator()(const In&...) const = 0;
  };

  template <typename... Out, typename... In, typename Traits_>
  struct MultiTransformerFilter<std::tuple<Out...>(const In&...), Traits_, false>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<Out...>,
                                                    Gaudi::Functional::details::filter_evtcontext<In...>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<
        std::tuple<Out...>, Gaudi::Functional::details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute(const EventContext& ctx) const override final {
      try {
        return std::apply(
                   GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN[&](auto&... ohandle) {
                     return std::apply(
                         [&ohandle...](bool passed, auto&&... data) {
                           (Gaudi::Functional::details::put(ohandle, std::forward<decltype(data)>(data)), ...);
                           return passed;
                         },
                         Gaudi::Functional::details::filter_evtcontext_t<In...>::apply(*this, ctx, this->m_inputs));
                   },
                   GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END

                   this->m_outputs)
            ? Gaudi::Functional::FilterDecision::PASSED
            : Gaudi::Functional::FilterDecision::FAILED;
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    // instead they MUST implement this operator
    virtual std::tuple<bool, Out...> operator()(const In&...) const = 0;
  };
} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using Transformer = details::Transformer<Signature, Traits_>;

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using MultiTransformer = details::MultiTransformer<Signature, Traits_>;

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using MultiTransformerFilter = details::MultiTransformerFilter<Signature, Traits_, false>;

} // namespace k4FWCore
