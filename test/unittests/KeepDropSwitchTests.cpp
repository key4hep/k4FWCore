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

#include "k4FWCore/KeepDropSwitch.h"

#include "catch2/catch_test_macros.hpp"

using k4FWCore::KeepDropSwitch;

TEST_CASE("KeepDropSwitch default constructor accepts all") {
  const auto keepDropSwitch = KeepDropSwitch();
  REQUIRE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE(keepDropSwitch.isOn(""));
  REQUIRE(keepDropSwitch.isOn("arbitrary strings are still accepted"));
}

TEST_CASE("KeepDropSwitch drop * drops all") {
  const auto keepDropSwitch = KeepDropSwitch({"drop *"});
  REQUIRE_FALSE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE_FALSE(keepDropSwitch.isOn(""));
  REQUIRE_FALSE(keepDropSwitch.isOn("aribtrary strings are not accepted either"));
}

TEST_CASE("KeepDropSwitch keep * keeps all") {
  const auto keepDropSwitch = KeepDropSwitch({"keep *"});
  REQUIRE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE(keepDropSwitch.isOn(""));
  REQUIRE(keepDropSwitch.isOn("aribtrary strings are also accepted"));
}

TEST_CASE("KeepDropSwitch cmd order is respected for *") {
  const auto dropAllFirst = KeepDropSwitch({"drop *", "keep *"});
  REQUIRE(dropAllFirst.isOn("MCParticles"));
  REQUIRE(dropAllFirst.isOn(""));
  REQUIRE(dropAllFirst.isOn("aribtrary strings are also accepted"));

  const auto keepAllFirst = KeepDropSwitch({"keep *", "drop *"});
  REQUIRE_FALSE(keepAllFirst.isOn("MCParticles"));
  REQUIRE_FALSE(keepAllFirst.isOn(""));
  REQUIRE_FALSE(keepAllFirst.isOn("aribtrary strings are not accepted either"));
}

TEST_CASE("KeepDropSwitch keep and drop without wildcards") {
  const auto keepDropSwitch = KeepDropSwitch({"keep MCParticles", "drop RecoParticles"});
  REQUIRE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE_FALSE(keepDropSwitch.isOn("RecoParticles"));
  // NOTE: Things without keep or dropped are kept by default!
  REQUIRE(keepDropSwitch.isOn("OtherNames"));
}

TEST_CASE("KeepDropSwitch keep and drop with ? wildcard") {
  const auto keepDropSwitch = KeepDropSwitch({"keep *", "drop Colle?tion"});
  REQUIRE_FALSE(keepDropSwitch.isOn("ColleCtion"));
  REQUIRE_FALSE(keepDropSwitch.isOn("Collection"));
  REQUIRE_FALSE(keepDropSwitch.isOn("Colle7tion"));
  REQUIRE(keepDropSwitch.isOn("CollectionXYZ"));
}

TEST_CASE("KeepDropSwitch malformed cmd inputs") {
  REQUIRE_THROWS_AS(KeepDropSwitch({"abc"}), std::invalid_argument);
  REQUIRE_THROWS_AS(KeepDropSwitch({"foobar MCParticles"}), std::invalid_argument);
  REQUIRE_THROWS_AS(KeepDropSwitch({"keep anything-but-type collType"}), std::invalid_argument);
}

TEST_CASE("KeepDropSwitch drops types") {
  const auto keepDropSwitch = KeepDropSwitch({"drop type MCParticleCollection"});
  REQUIRE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE_FALSE(keepDropSwitch.isOn("MCParticles", "MCParticleCollection"));
  REQUIRE(keepDropSwitch.isOn("MCParticles", "AnotherCollection"));
}

TEST_CASE("KeepDropSwitch types respect command order") {
  auto keepDropSwitch = KeepDropSwitch({"drop *", "keep type ABCollection"});
  REQUIRE(keepDropSwitch.isOn("collName", "ABCollection"));
  REQUIRE_FALSE(keepDropSwitch.isOn("collName", "NonKeptType"));
  REQUIRE_FALSE(keepDropSwitch.isOn("collName"));

  keepDropSwitch = KeepDropSwitch({"keep type ABCollection", "drop *"});
  REQUIRE_FALSE(keepDropSwitch.isOn("collName", "ABCollection"));

  keepDropSwitch = KeepDropSwitch({"drop *", "keep MCParticles", "drop type ABCType", "keep type FooType"});
  REQUIRE(keepDropSwitch.isOn("Anything", "FooType"));
  REQUIRE_FALSE(keepDropSwitch.isOn("Anything", "ABCType"));
  REQUIRE_FALSE(keepDropSwitch.isOn("MCParticles", "ABCType"));
  REQUIRE(keepDropSwitch.isOn("MCParticles"));
  REQUIRE_FALSE(keepDropSwitch.isOn("Anything"));
}
