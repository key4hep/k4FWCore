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
#ifndef FWCORE_METADATAUTILS_H
#define FWCORE_METADATAUTILS_H

#include <GaudiKernel/Service.h>
#include "Gaudi/Algorithm.h"

#include "k4FWCore/IMetadataSvc.h"

// #include "GaudiKernel/CommonMessaging.h"

namespace k4FWCore {

  template <typename T> void putParameter(const std::string& name, const T& value, const Gaudi::Algorithm* alg) {
    auto metadataSvc = alg->service<IMetadataSvc>("MetadataSvc", false);
    if (!metadataSvc) {
      alg->error() << "MetadataSvc not found" << endmsg;
      return;
    }
    metadataSvc->put<T>(name, value);
  }
  template <typename T> std::optional<T> getParameter(const std::string& name, const Gaudi::Algorithm* alg) {
    auto metadataSvc = alg->service<IMetadataSvc>("MetadataSvc", false);
    if (!metadataSvc) {
      alg->error() << "MetadataSvc not found" << endmsg;
      return std::nullopt;
    }
    return metadataSvc->get<T>(name);
  }
}  // namespace k4FWCore

#endif  // CORE_FUNCTIONALUTILS_H
