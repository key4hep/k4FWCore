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

#include "k4FWCore/IMetadataSvc.h"

#include <Gaudi/Algorithm.h>
#include <GaudiKernel/Service.h>

#include <podio/FrameCategories.h>

#include <edm4hep/Constants.h>

#include <optional>
#include <ostream>

namespace k4FWCore {

template <typename T>
concept Streamable = requires(std::ostream& os, const T& v) {
  { os << v } -> std::convertible_to<std::ostream&>;
};

/// @brief Save a metadata parameter in the metadata frame
/// @param name The name of the parameter
/// @param value The value of the parameter
/// @param comp The Gaudi component (algorithm, tool) that is saving the
///             parameter, typically "this"
/// @tparam GaudiComp The type of the component. This will be deduced in
///                   pretty much all of the use cases
template <typename T, typename GaudiComp>
void putParameter(const std::string& name, const T& value, const GaudiComp* comp) {
  comp->debug() << "Trying to put parameter '" << name << "'";
  if constexpr (Streamable<T>) {
    comp->debug() << " (value = " << value << ")";
  }
  comp->debug() << endmsg;
  auto metadataSvc = comp->template service<IMetadataSvc>("MetadataSvc", false);
  if (!metadataSvc) {
    comp->error() << "MetadataSvc not found" << endmsg;
    return;
  }
  metadataSvc->template put<T>(name, value);
}

/// @brief Save a metadata parameter in the metadata frame. Overload for compatibility
/// with the MetadataHandle, don't use!
/// @deprecated Use the overload taking a Gaudi::Algorithm* instead
template <typename T>
[[deprecated("Use the overload taking a Gaudi::Algorithm* as third argument instead")]] void
putParameter(const std::string& name, const T& value) {
  auto metadataSvc = Gaudi::svcLocator()->service<IMetadataSvc>("MetadataSvc", false);
  if (!metadataSvc) {
    std::cout << "MetadataSvc not found" << std::endl;
    return;
  }
  return metadataSvc->put<T>(name, value);
}

/// @brief Get a metadata parameter from the metadata frame
/// @param name The name of the parameter
/// @param comp The Gaudi component (algorithm, tool) that is retrieving the
///             parameter, typically "this"
/// @tparam GaudiComp The type of the component. This will be deduced in
///                   pretty much all of the use cases
/// @return std::optional<T> The value of the parameter, if it exists or std::nullopt
template <typename T, typename GaudiComp>
std::optional<T> getParameter(const std::string& name, const GaudiComp* comp) {
  comp->debug() << "Trying to get parameter '" << name << "'" << endmsg;
  auto metadataSvc = comp->template service<IMetadataSvc>("MetadataSvc", false);
  if (!metadataSvc) {
    comp->error() << "MetadataSvc not found" << endmsg;
    return std::nullopt;
  }
  return metadataSvc->template get<T>(name);
}

/// @brief Get a metadata parameter from the metadata frame. Overload for compatibility
/// with the MetadataHandle, don't use!
/// @deprecated Use the overload taking a Gaudi::Algorithm* instead
template <typename T>
[[deprecated("Use the overload taking a Gaudi::Algorithm* as second argument instead")]] std::optional<T>
getParameter(const std::string& name) {
  auto metadataSvc = Gaudi::svcLocator()->service<IMetadataSvc>("MetadataSvc", false);
  if (!metadataSvc) {
    return std::nullopt;
  }
  return metadataSvc->get<T>(name);
}

/// @brief Put the CellID encoding string for a collection into the metadata
///
/// Internally builds the correct parameter name and then stores the encoding
/// into the metadata frame via the MetadataSvc
///
/// @param collName The name of the collection to which the encoding should be
///        associated
/// @param encoding The cell ID encoding string
/// @param comp The Gaudi component (algorithm, tool) that is saving the
///             parameter, typically "this"
/// @tparam GaudiComp The type of the component. This will be deduced in
///                   pretty much all of the use cases
template <typename GaudiComp = Gaudi::Algorithm>
void putCellIDEncoding(const std::string& collName, const std::string& encoding, const GaudiComp* comp) {
  putParameter(podio::collMetadataParamName(collName, edm4hep::labels::CellIDEncoding), encoding, comp);
}

/// @brief Get the CellID encoding string for a collection from the metadata
///
/// Internally builds the correct parameter name and then retrieves the encoding
/// from the metadata frame via the MetadataSvc
///
/// @param collName The name of the collection for which the encoding should be
///                 retrieved
/// @param comp The Gaudi component (algorithm, tool) that is retrieving the
///             encoding, typically "this"
/// @tparam GaudiComp The type of the component. This will be deduced in
///                   pretty much all of the use cases
/// @return  The encoding string if it has been found or std::nullopt if not
template <typename GaudiComp = Gaudi::Algorithm>
std::optional<std::string> getCellIDEncoding(const std::string& collName, const GaudiComp* comp) {
  return getParameter<std::string>(podio::collMetadataParamName(collName, edm4hep::labels::CellIDEncoding), comp);
}

} // namespace k4FWCore

#endif // FWCORE_METADATAUTILS_H
