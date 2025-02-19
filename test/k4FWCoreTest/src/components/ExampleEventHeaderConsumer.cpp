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

#include "edm4hep/Constants.h"
#include "edm4hep/EventHeaderCollection.h"

#include "k4FWCore/Consumer.h"

#include <Gaudi/Property.h>

#include <fmt/core.h>

#include <atomic>
#include <stdexcept>
#include <string>

struct ExampleEventHeaderConsumer final : k4FWCore::Consumer<void(const edm4hep::EventHeaderCollection&)> {
  ExampleEventHeaderConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, {KeyValues("EventHeaderName", {edm4hep::labels::EventHeader})}) {}

  void operator()(const edm4hep::EventHeaderCollection& evtHeaderColl) const override {
    if (evtHeaderColl.empty()) {
      throw std::runtime_error("EventHeader collection is empty");
    }
    const auto evtHeader = evtHeaderColl[0];
    if (!evtHeader.isAvailable()) {
      throw std::runtime_error("Cannot get a valid EventHeader");
    }

    if (evtHeader.getRunNumber() != m_runNumber) {
      throw std::runtime_error(fmt::format("Run number is not set correctly (expected {}, actual {})",
                                           m_runNumber.value(), evtHeader.getRunNumber()));
    }

    const auto expectedEvent = m_evtCounter++ + m_eventNumberOffset;
    if (evtHeader.getEventNumber() != expectedEvent) {
      throw std::runtime_error(fmt::format("Event number is not set correctly (expected {}, actual {})", expectedEvent,
                                           evtHeader.getEventNumber()));
    }
  }

  // Since the type of the runNumber and eventNumber changed recently in EDM4hep, deduce their type
  // instead of hardcoding it to avoid warnings
  Gaudi::Property<decltype(std::declval<edm4hep::EventHeader>().getEventNumber())> m_runNumber{
      this, "runNumber", 0, "The expected run number"};
  Gaudi::Property<decltype(std::declval<edm4hep::EventHeader>().getEventNumber())> m_eventNumberOffset{
      this, "eventNumberOffset", 0, "The event number offset where events will start counting from"};
  mutable std::atomic<decltype(std::declval<edm4hep::EventHeader>().getRunNumber())> m_evtCounter{0};
};

DECLARE_COMPONENT(ExampleEventHeaderConsumer)
