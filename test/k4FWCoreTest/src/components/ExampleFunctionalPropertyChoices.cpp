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
// values with the helpers from k4FWCore/PropertyChoices.h, for the different kinds of properties.

#include "Gaudi/Property.h"

#include "k4FWCore/Consumer.h"
#include "k4FWCore/GaudiChecks.h"
#include "k4FWCore/PropertyChoices.h"

#include <string>

namespace {
enum class Gandalf { TheGrey, TheWhite };
enum class Resurrection { None, Gandalf };
} // namespace

// Register the enums, so that they can be the type of a Gaudi::Property.
// (1) By the names of the enumerators: the user writes "TheGrey" or "TheWhite"
K4FWCORE_ENUM_PROPERTY(Gandalf, Gandalf::TheGrey, Gandalf::TheWhite);
// (2) By custom names: the user writes "NoResurrection" or "WithResurrection"
K4FWCORE_ENUM_PROPERTY_NAMES(Resurrection, {Resurrection::None, "NoResurrection"},
                             {Resurrection::Gandalf, "WithResurrection"});

struct ExampleFunctionalPropertyChoices final : k4FWCore::Consumer<void()> {
  ExampleFunctionalPropertyChoices(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {}

  // Enum properties: set by name from the steering file, the code works with the enumerator
  Gaudi::Property<Gandalf> m_gandalf{this, "Gandalf", Gandalf::TheGrey, "One of " + k4FWCore::enumNameList<Gandalf>()};
  Gaudi::Property<Resurrection> m_resurrection{this, "SurvivalRate", Resurrection::Gandalf,
                                               "One of " + k4FWCore::enumNameList<Resurrection>()};

  // (3) A few allowed integers: anything but breakfast or no breakfast is forbidden
  using BreakfastChoices = k4FWCore::OneOf<0, 1>;
  Gaudi::Property<int, BreakfastChoices> m_numBreakfasts{this, "NumBreakfasts", 1,
                                                         "One of " + BreakfastChoices::list()};

  // (4) A few allowed strings, where the string itself is what the code uses,
  //     e.g. to look up a detector or to name a collection
  using MemberChoices =
      k4FWCore::OneOfStrings<"Frodo", "Sam", "Merry", "Pippin", "Gandalf", "Aragorn", "Legolas", "Gimli", "Boromir">;
  Gaudi::Property<std::string, MemberChoices> m_fellowshipLeader{this, "FellowshipLeader", "Frodo",
                                                                 "One of " + MemberChoices::list()};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(Consumer::initialize());
    // Nothing to validate here, the properties cannot hold an invalid value
    info() << "Gandalf = " << m_gandalf.toString() << ", NumBreakfasts = " << m_numBreakfasts.value()
           << ", FellowshipLeader = " << m_fellowshipLeader.value() << ", SurvivalRate = " << m_resurrection.toString()
           << endmsg;
    return StatusCode::SUCCESS;
  }

  void operator()() const override {
    // The enums are used to branch, the other values are used directly
    const auto* wizard = m_gandalf == Gandalf::TheWhite ? "the White" : "the Grey";
    const int survivors = m_resurrection == Resurrection::Gandalf ? 8 : 7;
    info() << m_fellowshipLeader.value() << " leads the Fellowship with Gandalf " << wizard << " after "
           << m_numBreakfasts.value() << " breakfast(s), and " << survivors
           << " of the 9 members are expected to survive" << endmsg;
  }
};

DECLARE_COMPONENT(ExampleFunctionalPropertyChoices)
