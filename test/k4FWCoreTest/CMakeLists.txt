#[[
Copyright (c) 2014-2024 Key4hep-Project.

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
                 LINK Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hepDict EDM4HEP::edm4hep)

include(CTest)

set(K4RUN ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run)

function(set_test_env testname)
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "ROOT_INCLUDE_PATH=$<$<TARGET_EXISTS:podio::podio>:$<TARGET_FILE_DIR:podio::podio>/../include>:$<$<TARGET_EXISTS:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>/../include>:$ENV{ROOT_INCLUDE_PATH}")
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR}/${PROJECT_NAME}:${PROJECT_BINARY_DIR}/${PROJECT_NAME}/genConfDir/${PROJECT_NAME}:${PROJECT_BINARY_DIR}/test/k4FWCoreTest:${PROJECT_BINARY_DIR}/test/k4FWCoreTest/genConfDir/k4FWCoreTest:$<$<TARGET_EXISTS:ROOT::Core>:$<TARGET_FILE_DIR:ROOT::Core>>:$<$<TARGET_EXISTS:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>>:$<$<TARGET_EXISTS:podio::podio>:$<TARGET_FILE_DIR:podio::podio>>:$ENV{LD_LIBRARY_PATH}")
  set_property(TEST ${testname} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}/${PROJECT_NAME}/genConfDir:${PROJECT_BINARY_DIR}/test/k4FWCoreTest/genConfDir:$ENV{PYTHONPATH}")
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
           # WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
           COMMAND ${K4RUN} ${CMAKE_CURRENT_LIST_DIR}/${TEST_ARGUMENTS})
  if(TEST_PROPERTIES_FOUND)
    set_tests_properties(${testname} PROPERTIES ${TEST_PROPERTIES})
  endif()
  set_test_env(${testname})
endfunction()

add_test_with_env(CreateExampleEventData options/createExampleEventData.py)
add_test_with_env(CreateExampleEventDataInDirectory options/createExampleEventDataInDirectory.py)

add_test_with_env(CheckExampleEventData options/checkExampleEventData.py PROPERTIES DEPENDS CreateExampleEventData)
add_test_with_env(CheckExampleEventData_noCollections options/checkExampleEventData.py --collections= PROPERTIES DEPENDS CreateExampleEventData)
add_test_with_env(CheckExampleEventData_toolong options/checkExampleEventData.py -n 999 PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" DEPENDS CreateExampleEventData)
add_test_with_env(CheckExampleEventData_unbounded options/checkExampleEventData.py -n -1 PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" DEPENDS CreateExampleEventData)
add_test_with_env(ReadExampleEventData options/readExampleEventData.py)
set_property(TEST ReadExampleEventData APPEND PROPERTY DEPENDS CreateExampleEventData)
add_test_with_env(ReadExampleDataFromNthEvent options/readExampleDataFromNthEvent.py PROPERTIES DEPENDS CreateExampleEventData)

add_test_with_env(AlgorithmWithTFile options/TestAlgorithmWithTFile.py)
set_property(TEST AlgorithmWithTFile PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
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
         COMMAND python scripts/check_KeepDropSwitch.py ${PROJECT_BINARY_DIR}/test/k4FWCoreTest/output_k4test_exampledata_2.root)
set_test_env(checkKeepDropSwitch)
set_property(TEST checkKeepDropSwitch APPEND PROPERTY DEPENDS ReadExampleEventData)
add_test_with_env(TestUniqueIDGenSvc options/TestUniqueIDGenSvc.py)
add_test_with_env(TestEventHeaderFiller options/createEventHeader.py)
add_test_with_env(EventHeaderCheck options/runEventHeaderCheck.py PROPERTIES DEPENDS TestEventHeaderFiller)
add_test(NAME TestExec WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} COMMAND python options/TestExec.py)
set_test_env(TestExec)

add_test(NAME Testk4runNoArgumentsHelp COMMAND ${K4RUN})
set_test_env(Testk4runNoArgumentsHelp)
set_tests_properties(Testk4runNoArgumentsHelp PROPERTIES PASS_REGULAR_EXPRESSION "Usage: k4run <options_file.py>, use --help to get a complete list of arguments")
add_test_with_env(Testk4runCustomArguments options/TestArgs.py --foo=42 PROPERTIES PASS_REGULAR_EXPRESSION "The answer is 42")
add_test_with_env(Testk4runVerboseOutput options/TestArgs.py --verbose PROPERTIES PASS_REGULAR_EXPRESSION " VERBOSE ")
add_test_with_env(Testk4runHelpOnly options/TestArgs.py --help PROPERTIES PASS_REGULAR_EXPRESSION "show this help message and exit")


# WARNING: If test names are changed, they also need to be changed below for the dependencies of
# the FunctionalCheckFiles test

add_test_with_env(FunctionalMemory options/ExampleFunctionalMemory.py)
add_test_with_env(FunctionalMTMemory options/ExampleFunctionalMTMemory.py)
add_test_with_env(FunctionalMultipleMemory options/ExampleFunctionalMultipleMemory.py)
add_test_with_env(FunctionalProducer options/ExampleFunctionalProducer.py)
add_test_with_env(FunctionalProducerAnother options/ExampleFunctionalProducer.py --second)
add_test_with_env(FunctionalProducerMultiple options/ExampleFunctionalProducerMultiple.py)
add_test_with_env(FunctionalProducerAbsolutePath options/ExampleFunctionalProducerAbsolutePath.py)
add_test_with_env(FunctionalFile options/ExampleFunctionalFile.py PROPERTIES DEPENDS FunctionalProducer)
add_test_with_env(FunctionalSeveralInputFiles options/ExampleFunctionalSeveralInputFiles.py)
set_tests_properties(FunctionalSeveralInputFiles PROPERTIES DEPENDS "FunctionalProducer;FunctionalProducerAnother")
add_test_with_env(FunctionalMTFile options/ExampleFunctionalMTFile.py PROPERTIES DEPENDS FunctionalProducer)
add_test_with_env(FunctionalMultipleFile options/ExampleFunctionalFileMultiple.py PROPERTIES DEPENDS FunctionalProducerMultiple)
add_test_with_env(FunctionalMix options/runFunctionalMix.py PROPERTIES DEPENDS FunctionalProducerMultiple)
add_test_with_env(FunctionalMixIOSvc options/runFunctionalMix.py --iosvc PROPERTIES DEPENDS FunctionalProducerMultiple)
add_test_with_env(FunctionalOutputCommands options/ExampleFunctionalOutputCommands.py PROPERTIES DEPENDS FunctionalProducerMultiple)
add_test_with_env(FunctionalConsumerRuntimeCollections options/ExampleFunctionalConsumerRuntimeCollections.py)
add_test_with_env(FunctionalConsumerRuntimeCollectionsMultiple options/ExampleFunctionalConsumerRuntimeCollectionsMultiple.py)
add_test_with_env(FunctionalProducerRuntimeCollections options/ExampleFunctionalProducerRuntimeCollections.py)
add_test_with_env(FunctionalTransformerRuntimeCollections options/ExampleFunctionalTransformerRuntimeCollections.py)
add_test_with_env(FunctionalTransformerRuntimeEmpty options/ExampleFunctionalTransformerRuntimeEmpty.py)
add_test_with_env(FunctionalTransformerRuntimeCollectionsMultiple options/ExampleFunctionalTransformerRuntimeCollectionsMultiple.py)
add_test_with_env(FunctionalTransformerHist options/ExampleFunctionalTransformerHist.py)
add_test_with_env(FunctionalCollectionMerger options/ExampleFunctionalCollectionMerger.py)
add_test_with_env(FunctionalFilterFile options/ExampleFunctionalFilterFile.py)
add_test_with_env(FunctionalMetadata options/ExampleFunctionalMetadata.py)
add_test_with_env(FunctionalMetadataOldAlgorithm options/ExampleFunctionalMetadataOldAlgorithm.py)
add_test_with_env(createEventHeaderConcurrent options/createEventHeaderConcurrent.py)

add_test_with_env(FunctionalWrongImport options/ExampleFunctionalWrongImport.py)
set_tests_properties(FunctionalWrongImport PROPERTIES PASS_REGULAR_EXPRESSION "ImportError: Importing ApplicationMgr or IOSvc from Configurables is not allowed.")

add_test(NAME FunctionalCheckFiles COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/options/CheckOutputFiles.py)
set_tests_properties(FunctionalCheckFiles PROPERTIES DEPENDS "FunctionalFile;FunctionalMTFile;FunctionalMultipleFile;FunctionalOutputCommands;FunctionalProducerAbsolutePath;FunctionalTransformerRuntimeEmpty;FunctionalMix;FunctionalMixIOSvc;FunctionalTransformerHist;FunctionalCollectionMerger;FunctionalFilterFile;FunctionalMetadata;FunctionalMetadataOldAlgorithm;createEventHeaderConcurrent")

# Do this after checking the files not to overwrite them
add_test_with_env(FunctionalFile_toolong options/ExampleFunctionalFile.py -n 999 PROPERTIES DEPENDS FunctionalCheckFiles PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop")

# The following is done to make the tests work without installing the files in
# the installation directory. The k4FWCore in the build directory is populated by
# Gaudi so we can't just make a k4FWCore folder and put the python files there.
# We put everything in one of the folders generated by Gaudi so that importing from
# k4FWCore will give the right result
add_custom_command(TARGET k4FWCoreTestPlugins POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${PROJECT_SOURCE_DIR}/python/k4FWCore/ ${PROJECT_BINARY_DIR}/k4FWCore/genConfDir/k4FWCore)
