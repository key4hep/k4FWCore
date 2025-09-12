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
/**
 * @file k4FWCore/k4FWCore/include/k4FWCore/check.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2025
 * @brief Helper to check return codes in a Gaudi component.
 */

#ifndef K4FWCORE_K4_CHECK_H
#define K4FWCORE_K4_CHECK_H

#include "GaudiKernel/StatusCode.h"

/**
 * @brief Helper to check return codes in a Gaudi component.
 *
 * <code>K4_GAUDI_CHECK( EXP )</code> will evaluate @c EXP.  If this results
 * in something that evaluates to @c false (including @c StatusCode::FAILURE),
 * we emit an error message and immediately return a @c StatusCode::FAILURE.
 * This can be used to make checking the results of calls from Gaudi components
 * easier to read.
 */
#define K4_GAUDI_CHECK(EXP)                                                                                            \
  do {                                                                                                                 \
    if (!(EXP)) {                                                                                                      \
      error() << __FILE__ << ":" << __LINE__ << " error returned from: " << #EXP << endmsg;                            \
      return StatusCode::FAILURE;                                                                                      \
    }                                                                                                                  \
  } while (0)

#endif // not K4FWCORE_K4_CHECK_H
