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

#include "k4FWCore/PropertyVerifiers.h"

#include <Gaudi/Property.h>

#include "catch2/catch_test_macros.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

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

/// Whether OneOf can be instantiated with the given allowed values
template <k4FWCore::detail::Choices... Allowed>
concept ValidOneOf = requires { typename k4FWCore::OneOf<Allowed...>; };

constexpr std::array layerCounts{4, 8, 15};
} // namespace

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
  REQUIRE(k4FWCore::OneOf<-1, 1>::list() == "-1, 1");
  try {
    k4FWCore::OneOf<0, 1>{}(2);
    FAIL("No exception thrown");
  } catch (const std::invalid_argument& e) {
    REQUIRE(std::string{e.what()} == "2 is not one of 0, 1");
  }
}

TEST_CASE("OneOf accepts only the given strings") {
  using Trackers = k4FWCore::OneOf<"DCH", "STT">;
  REQUIRE(Trackers::list() == "'DCH', 'STT'");
  REQUIRE(k4FWCore::OneOf<"DCH">::list() == "'DCH'");

  Gaudi::Property<std::string, Trackers> property{"Tracker", "DCH"};
  REQUIRE(setFromString(property, "'STT'"));
  REQUIRE(property.value() == "STT");
  REQUIRE_FALSE(setFromString(property, "dch"));
  REQUIRE_FALSE(setFromString(property, ""));
  REQUIRE(property.value() == "STT");
  REQUIRE_THROWS_AS((Gaudi::Property<std::string, Trackers>{"Tracker", "TPC"}), std::invalid_argument);
  try {
    Trackers{}("TPC");
    FAIL("No exception thrown");
  } catch (const std::invalid_argument& e) {
    REQUIRE(std::string{e.what()} == "'TPC' is not one of 'DCH', 'STT'");
  }
}

TEST_CASE("OneOf takes either integers or strings") {
  STATIC_REQUIRE(ValidOneOf<1, 2>);
  STATIC_REQUIRE(ValidOneOf<"a", "b">);
  STATIC_REQUIRE(ValidOneOf<{1, 2}>);
  STATIC_REQUIRE(ValidOneOf<{"a", "b"}>);
  STATIC_REQUIRE(ValidOneOf<layerCounts>);
  STATIC_REQUIRE_FALSE(ValidOneOf<1, "b">);
  STATIC_REQUIRE_FALSE(ValidOneOf<{1, 2}, "b">);
}

TEST_CASE("OneOf does not take booleans or characters") {
  STATIC_REQUIRE(ValidOneOf<std::int8_t{1}>);
  STATIC_REQUIRE(ValidOneOf<std::uint8_t{1}>);
  STATIC_REQUIRE(ValidOneOf<1ull>);
  STATIC_REQUIRE_FALSE(ValidOneOf<true>);
  STATIC_REQUIRE_FALSE(ValidOneOf<{true, false}>);
  STATIC_REQUIRE_FALSE(ValidOneOf<'a'>);
  STATIC_REQUIRE_FALSE(ValidOneOf<{'a', 'b'}>);
  STATIC_REQUIRE_FALSE(ValidOneOf<std::array{'a', 'b'}>);
  STATIC_REQUIRE_FALSE(ValidOneOf<u8'a'>);
}

TEST_CASE("OneOf takes the allowed values as a braced list") {
  using Layers = k4FWCore::OneOf<{4, 8, 15}>;
  REQUIRE(Layers::list() == "4, 8, 15");
  Gaudi::Property<int, Layers> property{"NumLayers", 8};
  REQUIRE(setFromString(property, "15"));
  REQUIRE_FALSE(setFromString(property, "16"));
  REQUIRE(property.value() == 15);

  using Trackers = k4FWCore::OneOf<{"DCH", "STT"}>;
  REQUIRE(Trackers::list() == "'DCH', 'STT'");
  Gaudi::Property<std::string, Trackers> tracker{"Tracker", "DCH"};
  REQUIRE(setFromString(tracker, "'STT'"));
  REQUIRE_FALSE(setFromString(tracker, "'TPC'"));
  REQUIRE(tracker.value() == "STT");
}

TEST_CASE("OneOf takes the allowed values as a std::array") {
  using Layers = k4FWCore::OneOf<layerCounts>;
  REQUIRE(Layers::list() == "4, 8, 15");
  Gaudi::Property<int, Layers> property{"NumLayers", 8};
  REQUIRE(setFromString(property, "15"));
  REQUIRE_FALSE(setFromString(property, "16"));
  REQUIRE(property.value() == 15);
  REQUIRE(k4FWCore::OneOf<std::array{1, 2}>::list() == "1, 2");
}

TEST_CASE("OneOf combines the different ways of giving the allowed values") {
  using Layers = k4FWCore::OneOf<1, {2, 3}, layerCounts>;
  REQUIRE(Layers::list() == "1, 2, 3, 4, 8, 15");
  REQUIRE_NOTHROW(Layers{}(3));
  REQUIRE_NOTHROW(Layers{}(15));
  REQUIRE_THROWS_AS(Layers{}(5), std::invalid_argument);

  REQUIRE(k4FWCore::OneOf<"DCH", {"STT", "TPC"}>::list() == "'DCH', 'STT', 'TPC'");
}

TEST_CASE("OneOf compares strings of different lengths exactly") {
  using Names = k4FWCore::OneOf<{"A", "ABC", ""}>;
  REQUIRE(Names::list() == "'A', 'ABC', ''");
  REQUIRE_NOTHROW(Names{}("A"));
  REQUIRE_NOTHROW(Names{}("ABC"));
  REQUIRE_NOTHROW(Names{}(""));
  REQUIRE_THROWS_AS(Names{}("AB"), std::invalid_argument);
  REQUIRE_THROWS_AS(Names{}("ABCD"), std::invalid_argument);
  REQUIRE_THROWS_AS(Names{}(std::string("A\0", 2)), std::invalid_argument);
}
