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
