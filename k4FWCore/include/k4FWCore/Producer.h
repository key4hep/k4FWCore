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
#ifndef FWCORE_PRODUCER_H
#define FWCORE_PRODUCER_H

#include "k4FWCore/Transformer.h"

#include <tuple>


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

#endif // FWCORE_PRODUCER_H
