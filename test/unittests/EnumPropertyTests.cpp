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

#include "k4FWCore/EnumProperty.h"

#include <Gaudi/Property.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/ToStream.h>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <exception>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
/// Set a property from a string as the job options do, and return whether that worked, independent of the
/// parsing error policy
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

std::string filled(const Gaudi::Details::PropertyBase& property) {
  std::ostringstream os;
  property.fillStream(os);
  return os.str();
}

/// Set the parsing error policy for the lifetime of the object
struct ScopedParsingErrorPolicy {
  using Policy = Gaudi::Details::Property::ParsingErrorPolicy;
  explicit ScopedParsingErrorPolicy(Policy p) : m_previous{Gaudi::Details::Property::setParsingErrorPolicy(p)} {}
  ~ScopedParsingErrorPolicy() { Gaudi::Details::Property::setParsingErrorPolicy(m_previous); }
  ScopedParsingErrorPolicy(const ScopedParsingErrorPolicy&) = delete;
  ScopedParsingErrorPolicy& operator=(const ScopedParsingErrorPolicy&) = delete;
  ScopedParsingErrorPolicy(ScopedParsingErrorPolicy&&) = delete;
  ScopedParsingErrorPolicy& operator=(ScopedParsingErrorPolicy&&) = delete;
  Policy m_previous;
};
} // namespace

// The enum used in most of the tests below
namespace {
enum class Mode { Fast, Precise, Experimental };
} // namespace
// Experimental is deliberately not registered
template <>
struct k4FWCore::EnumNames<Mode> : k4FWCore::Enumerators<Mode::Fast, Mode::Precise> {};
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Mode);

TEST_CASE("EnumProperty: enumValueName gives the unqualified enumerator name") {
  using k4FWCore::details::enumValueName;
  STATIC_REQUIRE(enumValueName<Mode::Fast>() == "Fast");
  STATIC_REQUIRE(enumValueName<Mode::Experimental>() == "Experimental");
  // A value without an enumerator name does not give an identifier
  STATIC_REQUIRE_FALSE(k4FWCore::details::isIdentifier(enumValueName<static_cast<Mode>(7)>()));
}

// The following must not compile, with a readable static_assert message
// (checked by hand with GCC and Clang)
//
// enum class Bad { A = 0, B = 0 };
// // a value without an enumerator name
// template <>
// struct k4FWCore::EnumNames<Bad> : k4FWCore::Enumerators<Bad::A, static_cast<Bad>(7)> {};
// // two enumerators with the same value, both are named "A"
// template <>
// struct k4FWCore::EnumNames<Bad> : k4FWCore::Enumerators<Bad::A, Bad::B> {};
// static_assert(k4FWCore::NamedEnum<Bad>);

TEST_CASE("EnumProperty: name validation") {
  using k4FWCore::details::hasUniqueNames;
  using k4FWCore::details::isIdentifier;
  STATIC_REQUIRE(isIdentifier("Fast"));
  STATIC_REQUIRE(isIdentifier("_x1"));
  STATIC_REQUIRE_FALSE(isIdentifier(""));
  STATIC_REQUIRE_FALSE(isIdentifier("1x"));
  STATIC_REQUIRE_FALSE(isIdentifier("Mode)7"));
  STATIC_REQUIRE_FALSE(isIdentifier("a b"));

  using Entry = std::pair<Mode, std::string_view>;
  using enum Mode;
  STATIC_REQUIRE(hasUniqueNames(std::array{Entry{Fast, "Fast"}, Entry{Precise, "Precise"}}));
  // Same name with different values is not allowed!
  STATIC_REQUIRE_FALSE(hasUniqueNames(std::array{Entry{Fast, "Fast"}, Entry{Precise, "Fast"}}));
  // Same value with different name works though!
  STATIC_REQUIRE(hasUniqueNames(std::array{Entry{Fast, "Fast"}, Entry{Fast, "Precise"}}));
}

TEST_CASE("EnumProperty: names can be looked up in both directions") {
  STATIC_REQUIRE(k4FWCore::NamedEnum<Mode>);
  STATIC_REQUIRE(k4FWCore::enumFromName<Mode>("Precise") == Mode::Precise);
  STATIC_REQUIRE(k4FWCore::enumToName(Mode::Fast) == "Fast");
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("fast").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("Mode::Fast").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Mode>("Experimental").has_value());
  STATIC_REQUIRE_FALSE(k4FWCore::enumToName(Mode::Experimental).has_value());
}

TEST_CASE("EnumProperty: enumNameList lists the names in the order of registration") {
  REQUIRE(k4FWCore::enumNameList<Mode>() == "'Fast', 'Precise'");
}

TEST_CASE("EnumProperty: set by the name of the enumerator") {
  Gaudi::Property<Mode> property{"Mode", Mode::Fast};
  // Unquoted as from the command line, quoted as from Python
  REQUIRE(setFromString(property, "Precise"));
  REQUIRE(property.value() == Mode::Precise);
  REQUIRE(setFromString(property, "'Fast'"));
  REQUIRE(property.value() == Mode::Fast);
  REQUIRE(setFromString(property, "\"Precise\""));
  REQUIRE(property.value() == Mode::Precise);
}

TEST_CASE("EnumProperty: other names are rejected and the value is kept") {
  Gaudi::Property<Mode> property{"Mode", Mode::Precise};
  REQUIRE_FALSE(setFromString(property, "Sloppy"));
  REQUIRE_FALSE(setFromString(property, "fast"));
  REQUIRE_FALSE(setFromString(property, "Experimental"));
  REQUIRE_FALSE(setFromString(property, "1"));
  REQUIRE_FALSE(setFromString(property, ""));
  REQUIRE(property.value() == Mode::Precise);
}

TEST_CASE("EnumProperty: the error for an invalid name lists the allowed ones") {
  using Catch::Matchers::ContainsSubstring;
  using Catch::Matchers::MessageMatches;
  const auto convert = [] { Gaudi::Details::Property::StringConverter<Mode>().fromString(Mode::Fast, "'Sloppy'"); };
  REQUIRE_THROWS_MATCHES(convert(), std::invalid_argument,
                         MessageMatches(ContainsSubstring("cannot parse ''Sloppy'' to ") &&
                                        ContainsSubstring("Mode, expected one of 'Fast', 'Precise'")));

  ScopedParsingErrorPolicy policy{Gaudi::Details::Property::ParsingErrorPolicy::Exception};
  Gaudi::Property<Mode> property{"Mode", Mode::Precise};
  REQUIRE_THROWS_MATCHES(property.fromString("'Sloppy'").ignore(), GaudiException,
                         MessageMatches(ContainsSubstring("Cannot convert ''Sloppy'' for property 'Mode'") &&
                                        ContainsSubstring("expected one of 'Fast', 'Precise'")));
  REQUIRE(property.value() == Mode::Precise);
}

TEST_CASE("EnumProperty: printed by the name of the enumerator") {
  Gaudi::Property<Mode> property{"Mode", Mode::Precise};
  REQUIRE(property.toString() == "Precise");
  // toStream gives the Python representation, i.e. a quoted string
  REQUIRE(streamed(property) == "'Precise'");
  REQUIRE(filled(property) == " 'Mode':Precise");

  // Both representations can be read back
  Gaudi::Property<Mode> other{"Other", Mode::Fast};
  REQUIRE(setFromString(other, property.toString()));
  REQUIRE(other.value() == Mode::Precise);
  other = Mode::Fast;
  REQUIRE(setFromString(other, streamed(property)));
  REQUIRE(other.value() == Mode::Precise);
}

TEST_CASE("EnumProperty: an unregistered enumerator is printed as its value") {
  // An enumerator without a name can only be assigned from C++
  Gaudi::Property<Mode> property{"Mode", Mode::Experimental};
  REQUIRE(property.toString() == "2");
  REQUIRE(streamed(property) == "2");
  // and cannot be read back
  REQUIRE_FALSE(setFromString(property, property.toString()));
}

TEMPLATE_TEST_CASE("EnumProperty: container property", "", std::vector<Mode>, std::set<Mode>) {
  using enum Mode;
  // The elements are printed by name
  REQUIRE(Gaudi::Utils::toString(TestType{Fast, Precise}) == "[ 'Fast' , 'Precise' ]");

  Gaudi::Property<TestType> property{"Modes", {Fast}};
  REQUIRE(setFromString(property, "['Fast', 'Precise']"));
  REQUIRE(property.value() == TestType{Fast, Precise});
  REQUIRE(setFromString(property, "[\"Precise\"]"));
  REQUIRE(property.value() == TestType{Precise});
  REQUIRE(setFromString(property, "['Precise', 'Fast', 'Precise']"));
  if constexpr (std::is_same_v<TestType, std::set<Mode>>) {
    // Duplicates collapse, and the Python set syntax works
    REQUIRE(property.value() == TestType{Fast, Precise});
    REQUIRE(setFromString(property, "{\"Precise\"}"));
    REQUIRE(property.value() == TestType{Precise});
  } else {
    REQUIRE(property.value() == TestType{Precise, Fast, Precise});
  }
  REQUIRE(setFromString(property, "[]"));
  REQUIRE(property.value().empty());

  // Nothing is changed if one of the elements is invalid
  property = TestType{Precise, Fast};
  REQUIRE_FALSE(setFromString(property, "['Fast', 'Sloppy']"));
  REQUIRE_FALSE(setFromString(property, "'Fast'"));
  REQUIRE(property.value() == TestType{Precise, Fast});

  // round trip
  Gaudi::Property<TestType> other{"Other", {}};
  REQUIRE(setFromString(other, property.toString()));
  REQUIRE(other.value() == property.value());
}

namespace {
enum class Unregistered { X };

// k4FWCoreEnumNames does not give a table. [[maybe_unused]] avoids a Clang warning because of the anonymous namespace
enum class NoTable { X };
constexpr int k4FWCoreEnumNames [[maybe_unused]] (NoTable) { return 0; }
} // namespace

TEST_CASE("EnumProperty: only registered enums are NamedEnums") {
  STATIC_REQUIRE(k4FWCore::details::enumValueName<Unregistered::X>() == "X");
  STATIC_REQUIRE_FALSE(k4FWCore::NamedEnum<Unregistered>);
  STATIC_REQUIRE_FALSE(k4FWCore::NamedEnum<NoTable>);
  STATIC_REQUIRE_FALSE(k4FWCore::NamedEnum<int>);
}

namespace {
enum Level { Low, High };
} // namespace
template <>
struct k4FWCore::EnumNames<Level> : k4FWCore::Enumerators<Low, High> {};
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Level);

TEST_CASE("EnumProperty: unscoped enum") {
  STATIC_REQUIRE(k4FWCore::details::enumValueName<Low>() == "Low");
  STATIC_REQUIRE(k4FWCore::NamedEnum<Level>);
  STATIC_REQUIRE(k4FWCore::enumFromName<Level>("High") == High);

  Gaudi::Property<Level> level{"Level", Low};
  REQUIRE(setFromString(level, "'High'"));
  REQUIRE(level.value() == High);
}

namespace {
enum class Small : unsigned char { X, Y, Z };
} // namespace
// Z is deliberately not registered
template <>
struct k4FWCore::EnumNames<Small> : k4FWCore::Enumerators<Small::X, Small::Y> {};
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Small);

TEST_CASE("EnumProperty: enum with a character as underlying type") {
  STATIC_REQUIRE(k4FWCore::details::enumValueName<Small::Y>() == "Y");
  STATIC_REQUIRE(k4FWCore::NamedEnum<Small>);

  Gaudi::Property<Small> small{"Small", Small::Y};
  REQUIRE(small.toString() == "Y");
  // An unregistered value is printed as a number, not as a character
  small = Small::Z;
  REQUIRE(small.toString() == "2");
  REQUIRE(streamed(small) == "2");
}

namespace {
struct Holder {
  enum class Kind { A, B };
};
} // namespace
template <>
struct k4FWCore::EnumNames<Holder::Kind> : k4FWCore::Enumerators<Holder::Kind::A, Holder::Kind::B> {};
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Holder::Kind);

TEST_CASE("EnumProperty: enum nested in a class") {
  STATIC_REQUIRE(k4FWCore::details::enumValueName<Holder::Kind::B>() == "B");
  STATIC_REQUIRE(k4FWCore::NamedEnum<Holder::Kind>);
  STATIC_REQUIRE(k4FWCore::enumFromName<Holder::Kind>("B") == Holder::Kind::B);

  Gaudi::Property<Holder::Kind> kind{"Kind", Holder::Kind::A};
  REQUIRE(setFromString(kind, "'B'"));
  REQUIRE(kind.value() == Holder::Kind::B);
  REQUIRE(kind.toString() == "B");
}

namespace outer::inner {
enum class Nested { Alpha, Beta };
} // namespace outer::inner
template <>
struct k4FWCore::EnumNames<outer::inner::Nested>
    : k4FWCore::Enumerators<outer::inner::Nested::Alpha, outer::inner::Nested::Beta> {};
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(outer::inner::Nested);

TEST_CASE("EnumProperty: enum in a nested namespace") {
  STATIC_REQUIRE(k4FWCore::details::enumValueName<outer::inner::Nested::Beta>() == "Beta");
  STATIC_REQUIRE(k4FWCore::NamedEnum<outer::inner::Nested>);
  STATIC_REQUIRE(k4FWCore::enumFromName<outer::inner::Nested>("Alpha") == outer::inner::Nested::Alpha);
  REQUIRE(k4FWCore::enumNameList<outer::inner::Nested>() == "'Alpha', 'Beta'");

  Gaudi::Property<outer::inner::Nested> nested{"Nested", outer::inner::Nested::Alpha};
  REQUIRE(setFromString(nested, "Beta"));
  REQUIRE(nested.value() == outer::inner::Nested::Beta);
  REQUIRE(streamed(nested) == "'Beta'");
}

namespace {
// Registration with k4FWCoreEnumNames, found by argument-dependent lookup. [[maybe_unused]] avoids a Clang warning
// because of the anonymous namespace
enum class Colour { Red, Green, Blue };
constexpr k4FWCore::Enumerators<Colour::Red, Colour::Green> k4FWCoreEnumNames [[maybe_unused]] (Colour) { return {}; }
} // namespace
K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Colour);

TEST_CASE("EnumProperty: registration with k4FWCoreEnumNames") {
  STATIC_REQUIRE(k4FWCore::NamedEnum<Colour>);
  REQUIRE(k4FWCore::enumNameList<Colour>() == "'Red', 'Green'");

  Gaudi::Property<Colour> colour{"Colour", Colour::Red};
  REQUIRE(setFromString(colour, "'Green'"));
  REQUIRE(colour.value() == Colour::Green);
  REQUIRE_FALSE(setFromString(colour, "Blue"));
  REQUIRE(colour.toString() == "Green");
  REQUIRE(streamed(colour) == "'Green'");
}

namespace {
// The enum is used as the type of a property of the class that declares it.
// This is the case which requires k4FWCoreEnumNames and K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY
struct Owner {
  enum class Mode { On, Off, Standby };
  // Standby is deliberately not registered
  // [[maybe_unused]] avoids a Clang warning because of the anonymous namespace
  friend constexpr k4FWCore::Enumerators<Mode::On, Mode::Off> k4FWCoreEnumNames [[maybe_unused]] (Mode) { return {}; }
  K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY(Mode);

  Gaudi::Property<Mode> mode{"Mode", Mode::Off, "One of " + k4FWCore::enumNameList<Mode>()};
  Gaudi::Property<std::vector<Mode>> modes{"Modes", {Mode::On}};
};
} // namespace

TEST_CASE("EnumProperty: an enum declared in the class that has the property") {
  STATIC_REQUIRE(k4FWCore::NamedEnum<Owner::Mode>);

  Owner owner;
  REQUIRE(owner.mode.documentation() == "One of 'On', 'Off'");
  REQUIRE(setFromString(owner.mode, "On"));
  REQUIRE(owner.mode.value() == Owner::Mode::On);
  REQUIRE(owner.mode.toString() == "On");
  REQUIRE(streamed(owner.mode) == "'On'");
  REQUIRE_FALSE(setFromString(owner.mode, "Standby"));
  REQUIRE(setFromString(owner.modes, "['Off', 'On']"));
  REQUIRE(owner.modes.value() == std::vector{Owner::Mode::Off, Owner::Mode::On});
  REQUIRE(owner.modes.toString() == "[ 'Off' , 'On' ]");
  REQUIRE_FALSE(setFromString(owner.modes, "['Off', 'Standby']"));

  // An unregistered enumerator is printed as its value
  owner.mode = Owner::Mode::Standby;
  REQUIRE(owner.mode.toString() == "2");
  REQUIRE(streamed(owner.mode) == "2");
}

namespace {
// Registered in both ways
enum class Both { A, B };
constexpr k4FWCore::Enumerators<Both::A, Both::B> k4FWCoreEnumNames [[maybe_unused]] (Both) { return {}; }
} // namespace
template <>
struct k4FWCore::EnumNames<Both> : k4FWCore::Enumerators<Both::B> {};

TEST_CASE("EnumProperty: a specialization of EnumNames takes precedence over k4FWCoreEnumNames") {
  REQUIRE(k4FWCore::enumNameList<Both>() == "'B'");
  STATIC_REQUIRE_FALSE(k4FWCore::enumFromName<Both>("A").has_value());
}
