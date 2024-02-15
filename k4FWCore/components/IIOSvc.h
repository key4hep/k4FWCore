/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#include "podio/ROOTWriter.h"

#include <exception>
#include <memory>
#include <vector>

/**
 * The interface implemented by any class making IO and reading RawEvent Data
 */
class IIOSvc : virtual public IInterface {
public:
  struct EndOfInput : std::logic_error {
    EndOfInput() : logic_error("Reached end of input while more data were expected"){};
  };

public:
  /// InterfaceID
  DeclareInterfaceID(IIOSvc, 1, 0);

  virtual std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame>
                                                    next()                     = 0;
  virtual std::shared_ptr<std::vector<std::string>> getCollectionNames() const = 0;

  virtual std::shared_ptr<podio::ROOTWriter> getWriter()                                  = 0;
  virtual void                               deleteWriter()                               = 0;
  virtual void                               deleteReader()                               = 0;
  virtual bool                               writeCollection(const std::string& collName) = 0;
};

#endif
