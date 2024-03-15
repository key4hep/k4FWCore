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
#ifndef FWCORE_PODIOINPUT_H
#define FWCORE_PODIOINPUT_H
// Gaudi
#include "Gaudi/Property.h"
#include "Gaudi/Functional/Consumer.h"

// STL
#include <string>
#include <vector>

class PodioDataSvc;

/** @class PodioInput
 *
 *  Class that allows to read ROOT files written with PodioOutput
 *
 *  @author J. Lingemann
 */

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

class PodioInput final : public Gaudi::Functional::Consumer<void(), BaseClass_t> {
public:
  PodioInput(const std::string& name, ISvcLocator* svcLoc);
  void operator()() const override;

  StatusCode initialize() final;

private:
  template <typename T> void maybeRead(std::string_view collName) const;
  void                       fillReaders();
  // Name of collections to read. Set by option collections (this is temporary)
  Gaudi::Property<std::vector<std::string>> m_collectionNames{
      this, "collections", {}, "Collections that should be read (default all)"};
  // Data service: needed to register objects and get collection IDs. Just an observing pointer.
  PodioDataSvc*                                                             m_podioDataSvc;
  mutable std::map<std::string_view, std::function<void(std::string_view)>> m_readers;
};

#endif
