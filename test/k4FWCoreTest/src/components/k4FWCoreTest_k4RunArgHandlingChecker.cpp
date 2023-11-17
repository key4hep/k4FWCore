/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#include "k4FWCore/BaseClass.h"

#include "Gaudi/Property.h"
#include "GaudiAlg/Producer.h"

#include <Gaudi/PluginServiceV2.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/StatusCode.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <bitset>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Simple algorithm that doesn't do anything useful other than declaring a bunch
 * of different properties that are then configured from the python side, resp.
 * via command line arguments of k4run. This algorithm does nothing else than
 * verifying that the full chain works as expected.
 *
 * In order to only have one Algorithm instead of multiple this algorithm is
 * called with multiple different invocations of arguments. The individual
 * checks are split into several different functions which are simply delegated
 * to according to the value of the testCase property.
 */
struct k4FWCoreTest_k4RunArgHandlingChecker final : Gaudi::Functional::Producer<int(), BaseClass_t> {
  k4FWCoreTest_k4RunArgHandlingChecker(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {KeyValue("AnInteger", "AnInteger")}) {}

  /// Dummy implementation for fulfilling the interface
  int operator()() const { return 42; }

  /// The initialize method is where all the checks happen as here we can easily
  /// return a StatusCode to report on failure and success.
  StatusCode initialize() final;

  /// Dummy check to make sure that the property defaults remain untouched
  int check_defaults() const;
  /// Check if setting single values from the command line works
  int check_singleValues() const;
  /// Check if setting vector properties works as expected, i.e. empty
  /// defaults
  int check_vectorValues() const;

  Gaudi::Property<std::string>              m_testCase{this, "testCase", "defaults", "The test case that is run"};
  Gaudi::Property<int>                      m_int{this, "integer", 42, "An integer for testing"};
  Gaudi::Property<float>                    m_float{this, "float", 3.14f, "An float for testing"};
  Gaudi::Property<std::string>              m_string{this, "string", "fancy string", "A string for testing"};
  Gaudi::Property<std::vector<std::string>> m_stringVec{this, "stringVec", {}, "A string vector for testing"};
  Gaudi::Property<std::vector<int>>         m_intVec{this, "intVec", {42}, "An int vector for testing"};
};

// Helper macro to take away a bit of boilerplate below.
// Checks if m_testCase is equal to the passed name and then runs the
// check_<name> method and if the return values is not 0 returns a failure
// status code.
#define TEST_CASE(name)                                                       \
  if (m_testCase == #name) {                                                  \
    if (check_##name()) {                                                     \
      fatal() << "Checks for " #name " test case did not succeed!" << endmsg; \
      return StatusCode::FAILURE;                                             \
    }                                                                         \
  }

StatusCode k4FWCoreTest_k4RunArgHandlingChecker::initialize() {
  TEST_CASE(defaults);
  TEST_CASE(singleValues);
  TEST_CASE(vectorValues);

  return StatusCode::SUCCESS;
}

#undef TEST_CASE

// Helper macro to reduce boiler plate in the actual check functions. Assumes
// that there is an integer results and that property is a Gaudi::Property that
// can compare against the passed expected value
#define ASSERT_EQUAL(expected, property)                                                                          \
  if (property != expected) {                                                                                     \
    error() << fmt::format("{} does not have expected value (expected {}, actual {})", property.name(), expected, \
                           property.value())                                                                      \
            << endmsg;                                                                                            \
    result++;                                                                                                     \
  }

int k4FWCoreTest_k4RunArgHandlingChecker::check_defaults() const {
  int result = 0;

  ASSERT_EQUAL(42, m_int);
  ASSERT_EQUAL(3.14f, m_float);
  ASSERT_EQUAL("fancy string", m_string);
  ASSERT_EQUAL(std::vector<std::string>{}, m_stringVec);
  ASSERT_EQUAL(std::vector<int>{42}, m_intVec);

  return result;
}

int k4FWCoreTest_k4RunArgHandlingChecker::check_singleValues() const {
  int result = 0;
  ASSERT_EQUAL(123, m_int);
  ASSERT_EQUAL(1.23f, m_float);
  ASSERT_EQUAL("some string with spaces even", m_string)
  // Vector values remain unchanged
  ASSERT_EQUAL(std::vector<std::string>{}, m_stringVec);
  ASSERT_EQUAL(std::vector<int>{42}, m_intVec);

  return result;
}

int k4FWCoreTest_k4RunArgHandlingChecker::check_vectorValues() const {
  int result = 0;

  const auto expectedStrings = std::vector<std::string>{"one", "two", "three"};
  ASSERT_EQUAL(expectedStrings, m_stringVec);
  const auto expectedInts = std::vector<int>{42, 84, 123};
  ASSERT_EQUAL(expectedInts, m_intVec);

  // single value properties remain unchanged
  ASSERT_EQUAL(42, m_int);
  ASSERT_EQUAL(3.14f, m_float);
  ASSERT_EQUAL("fancy string", m_string);

  return result;
}

#undef ASSERT_EQUAL

DECLARE_COMPONENT(k4FWCoreTest_k4RunArgHandlingChecker)
