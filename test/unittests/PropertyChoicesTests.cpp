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

const k4FWCore::PropertyChoices<Mode> s_modes{{"Fast", Mode::Fast}, {"Precise", Mode::Precise}};

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
  STATIC_REQUIRE(std::is_same_v<decltype(choices), const k4FWCore::PropertyChoices<Color>>);

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
  const k4FWCore::PropertyChoices<double> driftVelocities{{"Argon", 0.0023}, {"ArgonIsobutane", 0.0075}};
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
