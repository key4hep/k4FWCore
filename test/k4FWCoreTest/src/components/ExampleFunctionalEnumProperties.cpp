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

// This example shows how to use enums as the type of Gaudi properties, so that
// they can be set by name from the steering file. See doc/enumProperties.md

#include "Gaudi/Property.h"

#include "k4FWCore/Consumer.h"
#include "k4FWCore/EnumProperty.h"
#include "k4FWCore/GaudiChecks.h"

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

namespace {
enum class Gandalf { TheGrey, TheWhite };
enum class Member { Frodo, Sam, Merry, Pippin, Gandalf, Aragorn, Legolas, Gimli, Boromir };
} // namespace

// (1) Register the enums with K4FWCORE_ENUM_PROPERTY, before they are used as
//     the type of a property. The names are the enumerators: the user writes
//     "TheGrey" or "TheWhite"
K4FWCORE_ENUM_PROPERTY(Gandalf, TheGrey, TheWhite);
K4FWCORE_ENUM_PROPERTY(Member, Frodo, Sam, Merry, Pippin, Gandalf, Aragorn, Legolas, Gimli, Boromir);

struct ExampleFunctionalEnumProperties final : k4FWCore::Consumer<void()> {
  // (2) An enum declared inside the algorithm is registered with
  //     K4FWCORE_ENUM_PROPERTY_FRIEND inside the class (found by ADL); the
  //     enumerators have to be given with the name of the enum
  enum class Resurrection { NoResurrection, WithResurrection };
  K4FWCORE_ENUM_PROPERTY_FRIEND(Resurrection, Resurrection::NoResurrection, Resurrection::WithResurrection);

  ExampleFunctionalEnumProperties(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {}

  // Enum properties: set by name from the steering file, the code works with the enumerator
  Gaudi::Property<Gandalf> m_gandalf{this, "Gandalf", Gandalf::TheGrey, "One of " + k4FWCore::enumNameList<Gandalf>()};
  Gaudi::Property<Resurrection> m_resurrection{this, "SurvivalRate", Resurrection::WithResurrection,
                                               "One of " + k4FWCore::enumNameList<Resurrection>()};
  // (3) A list of enums
  Gaudi::Property<std::vector<Member>> m_fellowship{this,
                                                    "Fellowship",
                                                    {Member::Frodo, Member::Sam, Member::Merry, Member::Pippin,
                                                     Member::Gandalf, Member::Aragorn, Member::Legolas, Member::Gimli,
                                                     Member::Boromir},
                                                    "Any of " + k4FWCore::enumNameList<Member>()};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(Consumer::initialize());
    // Nothing to validate here, the properties cannot hold an invalid value
    info() << "Gandalf = " << m_gandalf.toString() << ", SurvivalRate = " << m_resurrection.toString()
           << ", Fellowship = " << m_fellowship.toString() << endmsg;
    return StatusCode::SUCCESS;
  }

  void operator()() const override {
    // The enums are used to branch
    const auto* wizard = m_gandalf == Gandalf::TheWhite ? "the White" : "the Grey";
    const auto& fellowship = m_fellowship.value();
    const auto size = std::ranges::ssize(fellowship);
    const bool withGandalf = std::ranges::find(fellowship, Member::Gandalf) != fellowship.end();
    // Boromir does not survive, Gandalf only with resurrection
    auto survivors = size - std::ranges::count(fellowship, Member::Boromir);
    if (withGandalf && m_resurrection == Resurrection::NoResurrection) {
      --survivors;
    }
    info() << "Gandalf " << wizard << (withGandalf ? " leads" : " watches") << " a Fellowship of " << size << ", and "
           << survivors << " are expected to survive" << endmsg;
  }
};

DECLARE_COMPONENT(ExampleFunctionalEnumProperties)
