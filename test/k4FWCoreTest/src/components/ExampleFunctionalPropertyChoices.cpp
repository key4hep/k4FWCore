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
// values with k4FWCore::PropertyChoices, for the different kinds of properties.

#include "Gaudi/Property.h"

#include "k4FWCore/Consumer.h"
#include "k4FWCore/GaudiChecks.h"
#include "k4FWCore/PropertyChoices.h"

#include <string>

namespace {
enum class Gandalf { TheGrey, TheWhite };
}

struct ExampleFunctionalPropertyChoices final : k4FWCore::Consumer<void()> {
  ExampleFunctionalPropertyChoices(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {}

  // (1) An enum selected by the name of the enumerators: the allowed strings are
  //     "TheGrey" and "TheWhite", the table is built from the enumerator names
  static inline const auto s_gandalfs = k4FWCore::makePropertyChoices<Gandalf::TheGrey, Gandalf::TheWhite>();

  // (2) A few allowed integers: the user writes the number, no label is involved.
  //     Anything but breakfast or no breakfast is forbidden
  static inline const auto s_numBreakfasts = k4FWCore::makePropertyChoices<0, 1>();

  // (3) A few allowed strings, where the string itself is what the code uses,
  //     e.g. to look up a detector or to name a collection
  static inline const auto s_members = k4FWCore::makePropertyChoices<"Frodo", "Sam", "Merry", "Pippin", "Gandalf",
                                                                     "Aragorn", "Legolas", "Gimli", "Boromir">();

  // (4) Labels for values of a type that cannot be derived from a name. The user selects
  //     the survival rate by name and the code gets the fraction of the Fellowship that survives
  static inline const k4FWCore::PropertyChoices<double> s_survivalRates{{"NoResurrection", 7. / 9.},
                                                                        {"WithResurrection", 8. / 9.}};

  // The properties. All of them document their allowed values with list()
  Gaudi::Property<std::string> m_gandalf{this, "Gandalf", "TheGrey", "One of " + s_gandalfs.list()};
  Gaudi::Property<int> m_numBreakfasts{this, "NumBreakfasts", 1, "One of " + s_numBreakfasts.list()};
  Gaudi::Property<std::string> m_fellowshipLeader{this, "FellowshipLeader", "Frodo", "One of " + s_members.list()};
  Gaudi::Property<std::string> m_survivalRate{this, "SurvivalRate", "WithResurrection",
                                              "One of " + s_survivalRates.list()};

  // What the code works with after the validation, filled in initialize()
  Gandalf m_gandalfMode{};
  int m_numBreakfastsValue{};
  std::string m_fellowshipLeaderValue;
  double m_survivalRateValue{};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(Consumer::initialize());
    // Validate the property and translate it. On an invalid value an error naming the
    // property and the allowed values is logged and the job fails
    K4_GAUDI_CHECK(s_gandalfs.parse(m_gandalf, m_gandalfMode, this));
    K4_GAUDI_CHECK(s_numBreakfasts.parse(m_numBreakfasts, m_numBreakfastsValue, this));
    K4_GAUDI_CHECK(s_members.parse(m_fellowshipLeader, m_fellowshipLeaderValue, this));
    K4_GAUDI_CHECK(s_survivalRates.parse(m_survivalRate, m_survivalRateValue, this));

    info() << "Gandalf = " << m_gandalf.value() << ", NumBreakfasts = " << m_numBreakfastsValue
           << ", FellowshipLeader = " << m_fellowshipLeaderValue << ", SurvivalRate = " << m_survivalRate.value()
           << ", value " << m_survivalRateValue << endmsg;
    return StatusCode::SUCCESS;
  }

  void operator()() const override {
    // The enum is used to branch, the other values are used directly
    const auto* wizard = m_gandalfMode == Gandalf::TheWhite ? "the White" : "the Grey";
    info() << m_fellowshipLeaderValue << " leads the Fellowship with Gandalf " << wizard << " after "
           << m_numBreakfastsValue << " breakfast(s), and " << 9 * m_survivalRateValue
           << " of the 9 members are expected to survive" << endmsg;
  }
};

DECLARE_COMPONENT(ExampleFunctionalPropertyChoices)
