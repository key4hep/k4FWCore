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

#include "k4FWCore/PropertyChoices.h"

#include <Gaudi/Property.h>

#include "catch2/catch_test_macros.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace {
enum class Mode { Fast, Precise, Experimental };
enum class Label { First, Second };
enum Level { Low, High };
struct Holder {
  enum class Kind { A, B };
};
} // namespace

namespace outer::inner {
enum class Nested { Alpha, Beta };
} // namespace outer::inner

// Experimental is deliberately not registered. The enumerators are given with and without the name of the enum
K4FWCORE_ENUM_PROPERTY(Mode, Fast, Precise);
K4FWCORE_ENUM_PROPERTY_NAMES(Label, {Label::First, "first"}, {Second, "second"});
K4FWCORE_ENUM_PROPERTY(Level, Low, High);
K4FWCORE_ENUM_PROPERTY(Holder::Kind, A, Holder::Kind::B);
K4FWCORE_ENUM_PROPERTY(outer::inner::Nested, outer::inner::Nested::Alpha, outer::inner::Nested::Beta);

namespace {
/// Set a property from a string as the steering file does, and return whether that worked, independent of the
/// parsing error policy of Gaudi
bool setFromString(Gaudi::Details::PropertyBase& property, const std::string& value) {
  try {
    return property.fromString(value).isSuccess();
  } catch (const std::exception&) {
    return false;
  }
}

std::string streamed(const Gaudi::Details::PropertyBase& property) {
  std::ostringstream os;
  property.toStream(os);
  return os.str();
}

enum class Unregistered { X };
} // namespace

TEST_CASE("enumValueName extracts the unqualified enum value name") {
  STATIC_REQUIRE(k4FWCore::detail::enumValueName<Mode::Fast>() == "Fast");
  STATIC_REQUIRE(k4FWCore::detail::enumValueName<Low>() == "Low");
  STATIC_REQUIRE(k4FWCore::detail::enumValueName<Holder::Kind::B>() == "B");
  STATIC_REQUIRE(k4FWCore::detail::enumValueName<outer::inner::Nested::Beta>() == "Beta");
}

TEST_CASE("Only registered enums are NamedEnums") {
  STATIC_REQUIRE(k4FWCore::NamedEnum<Mode>);
  STATIC_REQUIRE(k4FWCore::NamedEnum<Label>);
  STATIC_REQUIRE_FALSE(k4FWCore::NamedEnum<Unregistered>);
  STATIC_REQUIRE_FALSE(k4FWCore::NamedEnum<int>);
}

TEST_CASE("Enum names can be looked up in both directions") {
  STATIC_REQUIRE(k4FWCore::enumFromName<Mode>("Precise") == Mode::Precise);
  STATIC_REQUIRE(k4FWCore::enumToName(Mode::Fast) == "Fast");
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("fast").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("Mode::Fast").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("Experimental").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumToName(Mode::Experimental).has_value());

  STATIC_REQUIRE(k4FWCore::enumFromName<Label>("second") == Label::Second);
  STATIC_REQUIRE(k4FWCore::enumToName(Label::First) == "first");
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Label>("First").has_value());

  STATIC_REQUIRE(k4FWCore::enumFromName<Level>("High") == High);
  STATIC_REQUIRE(k4FWCore::enumFromName<Holder::Kind>("B") == Holder::Kind::B);
  STATIC_REQUIRE(k4FWCore::enumFromName<outer::inner::Nested>("Alpha") == outer::inner::Nested::Alpha);
}

TEST_CASE("enumNameList lists the names in the order of registration") {
  REQUIRE(k4FWCore::enumNameList<Mode>() == "'Fast', 'Precise'");
  REQUIRE(k4FWCore::enumNameList<Label>() == "'first', 'second'");
}

TEST_CASE("An enum property is set by the name of the enumerator") {
  Gaudi::Property<Mode> property{"Mode", Mode::Fast};
  // Unquoted as from the command line, quoted as from Python
  REQUIRE(setFromString(property, "Precise"));
  REQUIRE(property.value() == Mode::Precise);
  REQUIRE(setFromString(property, "'Fast'"));
  REQUIRE(property.value() == Mode::Fast);
  REQUIRE(setFromString(property, "\"Precise\""));
  REQUIRE(property.value() == Mode::Precise);

  Gaudi::Property<Label> label{"Label", Label::First};
  REQUIRE(setFromString(label, "second"));
  REQUIRE(label.value() == Label::Second);
}

TEST_CASE("An enum property rejects other names and keeps its value") {
  Gaudi::Property<Mode> property{"Mode", Mode::Precise};
  REQUIRE_FALSE(setFromString(property, "Sloppy"));
  REQUIRE_FALSE(setFromString(property, "fast"));
  REQUIRE_FALSE(setFromString(property, "Experimental"));
  REQUIRE_FALSE(setFromString(property, "1"));
  REQUIRE_FALSE(setFromString(property, ""));
  REQUIRE(property.value() == Mode::Precise);
}

TEST_CASE("The error for an invalid name lists the allowed ones") {
  const Gaudi::Details::Property::StringConverter<Mode> converter;
  try {
    converter.fromString(Mode::Fast, "'Sloppy'");
    FAIL("No exception thrown");
  } catch (const std::invalid_argument& e) {
    REQUIRE(std::string{e.what()} == "'Sloppy' is not one of 'Fast', 'Precise'");
  }
}

TEST_CASE("An enum property is printed by the name of the enumerator") {
  Gaudi::Property<Mode> property{"Mode", Mode::Precise};
  REQUIRE(property.toString() == "Precise");
  // toStream gives the Python representation, i.e. a quoted string
  REQUIRE(streamed(property) == "'Precise'");

  // The string representation can be read back
  Gaudi::Property<Mode> other{"Other", Mode::Fast};
  REQUIRE(setFromString(other, property.toString()));
  REQUIRE(other.value() == Mode::Precise);
  REQUIRE(setFromString(other, streamed(property)));
  REQUIRE(other.value() == Mode::Precise);

  // An enumerator without a name can only be assigned from C++ and is printed by its value
  property = Mode::Experimental;
  REQUIRE(property.toString() == "2");
  REQUIRE(streamed(property) == "2");
}

TEST_CASE("OneOf accepts only the given integers") {
  Gaudi::Property<int, k4FWCore::OneOf<4, 8, 15>> property{"NumLayers", 8};
  REQUIRE(setFromString(property, "15"));
  REQUIRE(property.value() == 15);

  REQUIRE_FALSE(setFromString(property, "16"));
  REQUIRE(property.value() == 15);
  REQUIRE_THROWS_AS(property = 16, std::invalid_argument);
  REQUIRE(property.value() == 15);
  REQUIRE_THROWS_AS((Gaudi::Property<int, k4FWCore::OneOf<4, 8, 15>>{"NumLayers", 0}), std::invalid_argument);
}

TEST_CASE("OneOf lists the allowed values") {
  REQUIRE(k4FWCore::OneOf<4, 8, 15>::list() == "4, 8, 15");
  REQUIRE(k4FWCore::OneOf<1>::list() == "1");
  REQUIRE(k4FWCore::OneOf<Mode::Precise, Mode::Fast>::list() == "'Precise', 'Fast'");
  REQUIRE(k4FWCore::OneOf<Unregistered::X>::list() == "0");
  try {
    k4FWCore::OneOf<0, 1>{}(2);
    FAIL("No exception thrown");
  } catch (const std::invalid_argument& e) {
    REQUIRE(std::string{e.what()} == "2 is not one of 0, 1");
  }
}

TEST_CASE("OneOf restricts an enum property to a subset of the enumerators") {
  using Stable = k4FWCore::OneOf<Mode::Fast>;
  Gaudi::Property<Mode, Stable> property{"Mode", Mode::Fast};
  REQUIRE(setFromString(property, "Fast"));
  REQUIRE_FALSE(setFromString(property, "Precise"));
  REQUIRE(property.value() == Mode::Fast);
  REQUIRE_THROWS_AS((Gaudi::Property<Mode, Stable>{"Mode", Mode::Precise}), std::invalid_argument);
}

TEST_CASE("OneOfStrings accepts only the given strings") {
  using Trackers = k4FWCore::OneOfStrings<"DCH", "STT">;
  REQUIRE(Trackers::list() == "'DCH', 'STT'");

  Gaudi::Property<std::string, Trackers> property{"Tracker", "DCH"};
  REQUIRE(setFromString(property, "'STT'"));
  REQUIRE(property.value() == "STT");
  REQUIRE_FALSE(setFromString(property, "dch"));
  REQUIRE_FALSE(setFromString(property, ""));
  REQUIRE(property.value() == "STT");
  REQUIRE_THROWS_AS((Gaudi::Property<std::string, Trackers>{"Tracker", "TPC"}), std::invalid_argument);
}
