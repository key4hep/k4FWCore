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

// This example shows how to restrict Gaudi properties to a fixed set of allowed
// values with the verifiers from k4FWCore/PropertyVerifiers.h

#include "Gaudi/Property.h"

#include "k4FWCore/Consumer.h"
#include "k4FWCore/GaudiChecks.h"
#include "k4FWCore/PropertyVerifiers.h"

#include <string>

struct ExampleFunctionalPropertyVerifiers final : k4FWCore::Consumer<void()> {
  ExampleFunctionalPropertyVerifiers(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {}

  // A few allowed integers: anything but breakfast or no breakfast is forbidden
  using BreakfastChoices = k4FWCore::OneOf<0, 1>;
  Gaudi::Property<int, BreakfastChoices> m_numBreakfasts{this, "NumBreakfasts", 1,
                                                         "One of " + BreakfastChoices::list()};

  // A few allowed strings, where the string itself is what the code uses,
  // e.g. to look up a detector or to name a collection
  using MemberChoices =
      k4FWCore::OneOf<"Frodo", "Sam", "Merry", "Pippin", "Gandalf", "Aragorn", "Legolas", "Gimli", "Boromir">;
  Gaudi::Property<std::string, MemberChoices> m_fellowshipLeader{this, "FellowshipLeader", "Frodo",
                                                                 "One of " + MemberChoices::list()};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(Consumer::initialize());
    // Nothing to validate here, the properties cannot hold an invalid value
    info() << "NumBreakfasts = " << m_numBreakfasts.value() << ", FellowshipLeader = " << m_fellowshipLeader.value()
           << endmsg;
    return StatusCode::SUCCESS;
  }

  void operator()() const override {
    info() << m_fellowshipLeader.value() << " leads the Fellowship after " << m_numBreakfasts.value() << " breakfast(s)"
           << endmsg;
  }
};

DECLARE_COMPONENT(ExampleFunctionalPropertyVerifiers)
