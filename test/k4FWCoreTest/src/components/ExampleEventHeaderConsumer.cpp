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
#include <GaudiKernel/ISvcLocator.h>
#include "GaudiAlg/Consumer.h"

#include "edm4hep/Constants.h"
#include "edm4hep/EventHeaderCollection.h"

#include "k4FWCore/BaseClass.h"

#include <optional>
#include <stdexcept>

struct ExampleEventHeaderConsumer final
    : Gaudi::Functional::Consumer<void(const edm4hep::EventHeaderCollection&), BaseClass_t> {
  ExampleEventHeaderConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, {KeyValue("EventHeaderName", edm4hep::EventHeaderName)}) {}

  void operator()(const edm4hep::EventHeaderCollection& evtHeaderColl) const {
    if (evtHeaderColl.empty()) {
      throw std::runtime_error("EventHeader collection is empty");
    }
    if (!evtHeaderColl[0].isAvailable()) {
      throw std::runtime_error("Cannot get a valid EventHeader");
    }
  }
};

DECLARE_COMPONENT(ExampleEventHeaderConsumer)
