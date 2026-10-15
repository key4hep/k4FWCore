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
