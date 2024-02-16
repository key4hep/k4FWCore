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

#ifndef K4FWCORE_FUNCTIONALUTILS_H
#define K4FWCORE_FUNCTIONALUTILS_H

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "k4FWCore/DataWrapper.h"

// Base class used for the Traits template argument of the
// Gaudi::Functional algorithms
struct [[deprecated(
    "Functional algorithms using the BaseClass.h header are deprecated and will be removed in the "
    "future")]] BaseClass_t {
  template <typename T> using InputHandle  = DataObjectReadHandle<DataWrapper<T>>;
  template <typename T> using OutputHandle = DataObjectWriteHandle<DataWrapper<T>>;

  using BaseClass = Gaudi::Algorithm;
};

#endif
