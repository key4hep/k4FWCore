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
#ifndef FWCORE_IIOSVC_H
#define FWCORE_IIOSVC_H

#include "GaudiKernel/IInterface.h"

#include "podio/CollectionBase.h"
#include "podio/Writer.h"

#include <string>
#include <string_view>
#include <vector>

/**
 * The interface implemented by any class making IO with functional algorithms
 */
class IIOSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IIOSvc, 1, 0);

  /**
   * @brief Read the next event from the input file
   * @return A tuple containing the collections read, the collection names and the frame that owns the collections
   */
  virtual std::tuple<std::vector<podio::CollectionBase*>, std::vector<std::string>, podio::Frame> next() = 0;
  virtual std::vector<std::string> getAvailableCollections() = 0;

  virtual podio::Writer& getWriter() = 0;
  virtual void deleteWriter() = 0;
  virtual void deleteReader() = 0;
  virtual bool checkIfWriteCollection(const std::string& collName, const std::string_view collType) const = 0;
};

#endif
