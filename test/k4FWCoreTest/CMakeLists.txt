#[[
Copyright (c) 2014-2023 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

file(GLOB k4fwcoretest_plugin_sources src/components/*.cpp)

gaudi_add_module(k4FWCoreTestPlugins
                 SOURCES ${k4fwcoretest_plugin_sources}
                 LINK Gaudi::GaudiAlgLib Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hepDict EDM4HEP::edm4hep)

include(CTest)

set(K4RUN ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run)

function(set_test_env testname)
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "ROOT_INCLUDE_PATH=$<$<TARGET_EXISTS:podio::podio>:$<TARGET_FILE_DIR:podio::podio>/../include>:$<$<TARGET_EXISTS:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>/../include>:$ENV{ROOT_INCLUDE_PATH}")
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}:${PROJECT_BINARY_DIR}/test/k4FWCoreTest:$<$<TARGET_EXISTS:ROOT::Core>:$<TARGET_FILE_DIR:ROOT::Core>>:$<$<TARGET_EXISTS:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>>:$<$<TARGET_EXISTS:podio::podio>:$<TARGET_FILE_DIR:podio::podio>>:$ENV{LD_LIBRARY_PATH}")
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}/genConfDir:${PROJECT_BINARY_DIR}/test/k4FWCoreTest/genConfDir:$ENV{PYTHONPATH}")
endfunction()

function(add_test_with_env testname)
  foreach(arg ${ARGN})
    if(arg STREQUAL "PROPERTIES")
      set(TEST_PROPERTIES_FOUND TRUE)
    else()
      # For tests that depend on multiple tests and the original cmake way is to
      # add a colon between them this should make it work
      string(REPLACE ";" "\\\\;" arg ${arg})
      if(TEST_PROPERTIES_FOUND)
        list(APPEND TEST_PROPERTIES ${arg})
      else()
        list(APPEND TEST_ARGUMENTS ${arg})
      endif()
    endif()
  endforeach()
  add_test(NAME ${testname}
           WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
           COMMAND ${K4RUN} ${TEST_ARGUMENTS})
  if(TEST_PROPERTIES_FOUND)
    set_tests_properties(${testname} PROPERTIES ${TEST_PROPERTIES})
  endif()
  set_test_env(${testname})
endfunction()

add_test_with_env(CreateExampleEventData options/createExampleEventData.py)

add_test_with_env(CheckExampleEventData options/checkExampleEventData.py PROPERTIES DEPENDS CreateExampleEventData)
add_test_with_env(CheckExampleEventData_toolong -n 999 options/checkExampleEventData.py PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" DEPENDS CreateExampleEventData)
add_test_with_env(CheckExampleEventData_unbounded options/checkExampleEventData.py -n 999 PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" DEPENDS CreateExampleEventData)
add_test_with_env(ReadExampleEventData options/readExampleEventData.py)
set_property(TEST ReadExampleEventData APPEND PROPERTY DEPENDS CreateExampleEventData)
add_test_with_env(ReadExampleDataFromNthEvent options/readExampleDataFromNthEvent.py PROPERTIES DEPENDS CreateExampleEventData)

add_test_with_env(AlgorithmWithTFile options/TestAlgorithmWithTFile.py)
add_test(NAME AlgorithmWithTFileCheckFrameworkOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND  python scripts/check_TestAlgorithmWithTFile_framework_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckFrameworkOutput APPEND PROPERTY DEPENDS AlgorithmWithTFile)
set_test_env(AlgorithmWithTFileCheckFrameworkOutput)
add_test(NAME AlgorithmWithTFileCheckMyTFileOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND  python scripts/check_TestAlgorithmWithTFile_myTFile_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckMyTFileOutput APPEND PROPERTY DEPENDS AlgorithmWithTFile)
set_test_env(AlgorithmWithTFileCheckMyTFileOutput)

add_test_with_env(CreateExampleEventData_cellID options/createExampleEventData_cellID.py)
add_test_with_env(TwoProducers options/TwoProducers.py --filename output_k4fwcore_test_twoproducer.root
                  --magicNumberOffset.Producer2 12345 --Producer1.magicNumberOffset 54321)
add_test_with_env(CheckCommandLineArguments options/createHelloWorld.py --HelloWorldAlg1.PerEventPrintMessage TwasBrilligAndTheSlithyToves
PROPERTIES PASS_REGULAR_EXPRESSION "TwasBrilligAndTheSlithyToves"
)
add_test(NAME checkKeepDropSwitch
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND python scripts/check_KeepDropSwitch.py)
set_test_env(checkKeepDropSwitch)
set_property(TEST checkKeepDropSwitch APPEND PROPERTY DEPENDS ReadExampleEventData)
add_test_with_env(TestDataHandleUniquePtr options/TestDataHandleUniquePtr.py)
add_test_with_env(TestUniqueIDGenSvc options/TestUniqueIDGenSvc.py)
add_test_with_env(CreateLegacyExampleEventData options/createLegacyExampleEventData.py)
add_test_with_env(TestEventHeaderFiller options/createEventHeader.py)
add_test(NAME TestExec WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} COMMAND python options/TestExec.py)
set_test_env(TestExec)

add_test_with_env(Testk4runNoArguments PROPERTIES PASS_REGULAR_EXPRESSION "Usage: k4run <options_file.py>, use --help to get a complete list of arguments")
add_test_with_env(Testk4runCustomArguments options/TestArgs.py --foo=42 PROPERTIES PASS_REGULAR_EXPRESSION "The answer is 42")
add_test_with_env(Testk4runVerboseOutput options/TestArgs.py --verbose PROPERTIES PASS_REGULAR_EXPRESSION " VERBOSE ")
add_test_with_env(Testk4runHelpOnly options/TestArgs.py --help PROPERTIES PASS_REGULAR_EXPRESSION "show this help message and exit")


add_test_with_env(ExampleFunctionalProducer options/runExampleFunctionalProducer.py)
add_test_with_env(ExampleFunctionalConsumer options/runExampleFunctionalConsumer.py PROPERTIES DEPENDS ExampleFunctionalProducer)
add_test_with_env(ExampleFunctionalTransformer options/runExampleFunctionalTransformer.py PROPERTIES DEPENDS ExampleFunctionalProducer)
add_test_with_env(ExampleFunctionalProducerMultiple options/runExampleFunctionalProducerMultiple.py)
add_test_with_env(ExampleFunctionalConsumerMultiple options/runExampleFunctionalConsumerMultiple.py PROPERTIES DEPENDS ExampleFunctionalProducerMultiple)
add_test_with_env(ExampleFunctionalTransformerMultiple options/runExampleFunctionalTransformerMultiple.py PROPERTIES DEPENDS ExampleFunctionalProducerMultiple)
add_test_with_env(FunctionalChain options/runFunctionalChain.py)
add_test_with_env(FunctionalMix options/runFunctionalMix.py PROPERTIES DEPENDS ExampleFunctionalProducerMultiple)
