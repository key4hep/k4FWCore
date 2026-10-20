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

#include <GaudiKernel/MsgStream.h>

#include "catch2/catch_test_macros.hpp"

#include <sstream>
#include <string>
#include <type_traits>

namespace {
enum class Mode { Fast, Precise };

const k4FWCore::PropertyChoices s_modes{std::pair{"Fast", Mode::Fast}, std::pair{"Precise", Mode::Precise}};

/// Stand-in for a Gaudi component, recording what is logged as an error
struct FakeComponent {
  struct Stream {
    std::ostringstream* out;
    template <typename T>
    Stream& operator<<(const T& v) {
      *out << v;
      return *this;
    }
    // swallow endmsg
    Stream& operator<<(MsgStream& (*)(MsgStream&)) { return *this; }
  };
  Stream error() const { return Stream{&log}; }
  mutable std::ostringstream log;
};
} // namespace

TEST_CASE("PropertyChoices parses valid values") {
  REQUIRE(s_modes.parse("Fast") == Mode::Fast);
  REQUIRE(s_modes.parse("Precise") == Mode::Precise);
}

TEST_CASE("PropertyChoices rejects unknown values") {
  REQUIRE_FALSE(s_modes.parse("fast").has_value());
  REQUIRE_FALSE(s_modes.parse("").has_value());
}

TEST_CASE("PropertyChoices lists the choices") { REQUIRE(s_modes.list() == "'Fast', 'Precise'"); }

TEST_CASE("PropertyChoices parses a Gaudi property") {
  Gaudi::Property<std::string> property{"Mode", "Precise"};
  Mode mode = Mode::Fast;
  FakeComponent comp;
  REQUIRE(s_modes.parse(property, mode, &comp).isSuccess());
  REQUIRE(mode == Mode::Precise);
}

TEST_CASE("PropertyChoices reports an invalid Gaudi property") {
  Gaudi::Property<std::string> property{"Mode", "Sloppy"};
  Mode mode = Mode::Fast;
  FakeComponent comp;
  REQUIRE(s_modes.parse(property, mode, &comp).isFailure());
  REQUIRE(mode == Mode::Fast);
  const auto message = comp.log.str();
  REQUIRE(message.find("Mode") != std::string::npos);
  REQUIRE(message.find("'Sloppy'") != std::string::npos);
  REQUIRE(message.find("'Fast', 'Precise'") != std::string::npos);
}

namespace {
enum class Color { Red, Green, Blue };
enum UnscopedLevel { Low, High };
} // namespace

namespace outer::inner {
enum class Nested { Alpha, Beta };
} // namespace outer::inner

TEST_CASE("enum_value_name extracts the unqualified enum value name") {
  STATIC_REQUIRE(k4FWCore::detail::enum_value_name<Mode::Fast>() == "Fast");
  STATIC_REQUIRE(k4FWCore::detail::enum_value_name<Color::Green>() == "Green");
  STATIC_REQUIRE(k4FWCore::detail::enum_value_name<Low>() == "Low");
  STATIC_REQUIRE(k4FWCore::detail::enum_value_name<outer::inner::Nested::Beta>() == "Beta");
}

TEST_CASE("makePropertyChoices uses the enumerator names") {
  const auto choices = k4FWCore::makePropertyChoices<Color::Red, Color::Green, Color::Blue>();
  STATIC_REQUIRE(std::is_same_v<decltype(choices), const k4FWCore::PropertyChoices<std::string, Color>>);

  REQUIRE(choices.parse("Red") == Color::Red);
  REQUIRE(choices.parse("Green") == Color::Green);
  REQUIRE(choices.parse("Blue") == Color::Blue);
  REQUIRE_FALSE(choices.parse("red").has_value());
  REQUIRE_FALSE(choices.parse("Color::Red").has_value());
}

TEST_CASE("makePropertyChoices keeps the order of the template arguments") {
  const auto choices = k4FWCore::makePropertyChoices<Color::Blue, Color::Red>();
  REQUIRE(choices.list() == "'Blue', 'Red'");
  REQUIRE_FALSE(choices.parse("Green").has_value());
}

TEST_CASE("makePropertyChoices works for unscoped and nested enums") {
  const auto levels = k4FWCore::makePropertyChoices<Low, High>();
  REQUIRE(levels.list() == "'Low', 'High'");
  REQUIRE(levels.parse("High") == High);

  const auto nested = k4FWCore::makePropertyChoices<outer::inner::Nested::Alpha, outer::inner::Nested::Beta>();
  REQUIRE(nested.list() == "'Alpha', 'Beta'");
  REQUIRE(nested.parse("Alpha") == outer::inner::Nested::Alpha);
}

TEST_CASE("PropertyChoices maps labels to a non-enum type") {
  const k4FWCore::PropertyChoices driftVelocities{std::pair{"Argon", 0.0023}, std::pair{"ArgonIsobutane", 0.0075}};
  REQUIRE(driftVelocities.parse("Argon") == 0.0023);
  REQUIRE(driftVelocities.parse("ArgonIsobutane") == 0.0075);
  REQUIRE_FALSE(driftVelocities.parse("Neon").has_value());
  REQUIRE(driftVelocities.list() == "'Argon', 'ArgonIsobutane'");
}

TEST_CASE("makePropertyChoices accepts only the given integers") {
  const auto layers = k4FWCore::makePropertyChoices<4, 8, 15, 16, 23, 42>();
  STATIC_REQUIRE(std::is_same_v<decltype(layers), const k4FWCore::PropertyChoices<int, int>>);

  REQUIRE(layers.parse(15) == 15);
  REQUIRE(layers.parse(42) == 42);
  REQUIRE_FALSE(layers.parse(5).has_value());
  REQUIRE(layers.list() == "4, 8, 15, 16, 23, 42");
}

TEST_CASE("makePropertyChoices works for a single value") {
  const auto one = k4FWCore::makePropertyChoices<7>();
  REQUIRE(one.parse(7) == 7);
  REQUIRE_FALSE(one.parse(8).has_value());
  REQUIRE(one.list() == "7");

  const auto mode = k4FWCore::makePropertyChoices<Mode::Fast>();
  REQUIRE(mode.parse("Fast") == Mode::Fast);
  REQUIRE(mode.list() == "'Fast'");
}

TEST_CASE("makePropertyChoices accepts only the given strings") {
  const auto formats = k4FWCore::makePropertyChoices<"root", "rntuple">();
  STATIC_REQUIRE(std::is_same_v<decltype(formats), const k4FWCore::PropertyChoices<std::string, std::string>>);

  REQUIRE(formats.parse("root") == "root");
  REQUIRE(formats.parse(std::string_view{"rntuple"}) == "rntuple");
  REQUIRE_FALSE(formats.parse("hdf5").has_value());
  REQUIRE_FALSE(formats.parse("").has_value());
  REQUIRE(formats.list() == "'root', 'rntuple'");
}

TEST_CASE("PropertyChoices parses an integer Gaudi property") {
  const auto layers = k4FWCore::makePropertyChoices<4, 8, 15>();
  FakeComponent comp;
  int value = 0;

  Gaudi::Property<int> valid{"NumLayers", 8};
  REQUIRE(layers.parse(valid, value, &comp).isSuccess());
  REQUIRE(value == 8);

  Gaudi::Property<int> invalid{"NumLayers", 9};
  REQUIRE(layers.parse(invalid, value, &comp).isFailure());
  REQUIRE(value == 8);
  const auto message = comp.log.str();
  REQUIRE(message.find("NumLayers") != std::string::npos);
  REQUIRE(message.find("9") != std::string::npos);
  REQUIRE(message.find("4, 8, 15") != std::string::npos);
}

TEST_CASE("PropertyChoices parses a string Gaudi property for allowed strings") {
  const auto formats = k4FWCore::makePropertyChoices<"root", "rntuple">();
  FakeComponent comp;
  std::string value;

  Gaudi::Property<std::string> property{"Format", "hdf5"};
  REQUIRE(formats.parse(property, value, &comp).isFailure());
  REQUIRE(comp.log.str().find("'hdf5'") != std::string::npos);

  property = "rntuple";
  REQUIRE(formats.parse(property, value, &comp).isSuccess());
  REQUIRE(value == "rntuple");
}

TEST_CASE("PropertyChoices with an integer key and an enum value") {
  const k4FWCore::PropertyChoices levels{std::pair{1, Mode::Fast}, std::pair{2, Mode::Precise}};
  REQUIRE(levels.parse(2) == Mode::Precise);
  REQUIRE_FALSE(levels.parse(3).has_value());
  REQUIRE(levels.list() == "1, 2");

  FakeComponent comp;
  Mode mode = Mode::Fast;
  Gaudi::Property<int> property{"Level", 2};
  REQUIRE(levels.parse(property, mode, &comp).isSuccess());
  REQUIRE(mode == Mode::Precise);

  property = 3;
  REQUIRE(levels.parse(property, mode, &comp).isFailure());
  REQUIRE(comp.log.str().find("Invalid Level 3, expected one of 1, 2.") != std::string::npos);
}

TEST_CASE("PropertyChoices lists nothing for an empty table") {
  const k4FWCore::PropertyChoices<int, int> empty{};
  REQUIRE(empty.list().empty());
  REQUIRE_FALSE(empty.parse(1).has_value());
}

TEST_CASE("PropertyChoices deduces its types from the pairs") {
  // String literals become std::string, other types are kept
  STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(s_modes)>, k4FWCore::PropertyChoices<std::string, Mode>>);

  const k4FWCore::PropertyChoices levels{std::pair{1, Mode::Fast}, std::pair{2, Mode::Precise}};
  STATIC_REQUIRE(std::is_same_v<decltype(levels), const k4FWCore::PropertyChoices<int, Mode>>);

  // String values are std::string too
  const k4FWCore::PropertyChoices names{std::pair{"fast", "Fast"}, std::pair{"precise", "Precise"}};
  STATIC_REQUIRE(std::is_same_v<decltype(names), const k4FWCore::PropertyChoices<std::string, std::string>>);
  REQUIRE(names.parse("fast") == "Fast");

  // A single pair is enough
  const k4FWCore::PropertyChoices single{std::pair{"Argon", 0.0023}};
  STATIC_REQUIRE(std::is_same_v<decltype(single), const k4FWCore::PropertyChoices<std::string, double>>);
  REQUIRE(single.parse("Argon") == 0.0023);
}

TEST_CASE("PropertyChoices deduces the common type of different values") {
  const k4FWCore::PropertyChoices mixed{std::pair{"a", 0.5}, std::pair{"b", 1}};
  STATIC_REQUIRE(std::is_same_v<decltype(mixed), const k4FWCore::PropertyChoices<std::string, double>>);
  REQUIRE(mixed.parse("b") == 1.0);

  // There is no common type of a number and an enum, so the types cannot be deduced
  const auto deducible = []<typename V1, typename V2>(V1 v1, V2 v2) {
    return requires { k4FWCore::PropertyChoices{std::pair{"a", v1}, std::pair{"b", v2}}; };
  };
  REQUIRE(deducible(1, 2.5));
  REQUIRE_FALSE(deducible(1, Mode::Fast));
}

TEST_CASE("PropertyChoices converts the pairs to explicitly given types") {
  const k4FWCore::PropertyChoices<std::string, double> choices{std::pair{"a", 1}, std::pair{"b", 2}};
  REQUIRE(choices.parse("b") == 2.0);
  REQUIRE(choices.list() == "'a', 'b'");
}
