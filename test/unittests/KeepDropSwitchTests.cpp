#include "k4FWCore/KeepDropSwitch.h"

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

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
  REQUIRE(keepDropSwitch.isOn("aribtrary strings are not accepted either"));
}

TEST_CASE("KeepDropSwitch cmd order is respected for *") {
  const auto dropAllFirst = KeepDropSwitch({"drop *", "keep *"});
  REQUIRE(dropAllFirst.isOn("MCParticles"));
  REQUIRE(dropAllFirst.isOn(""));
  REQUIRE(dropAllFirst.isOn("aribtrary strings are not accepted either"));

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
  const auto globalMalformed = KeepDropSwitch({"abc"});
  REQUIRE_THROWS_AS(globalMalformed.isOn("Any string"), std::invalid_argument);
  const auto malformedCmd = KeepDropSwitch({"foobar MCParticles"});
  REQUIRE_THROWS_AS(malformedCmd.isOn("whatever"), std::invalid_argument);
}
