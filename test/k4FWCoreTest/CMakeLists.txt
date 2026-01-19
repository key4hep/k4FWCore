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

set(k4fwcoretest_plugin_sources
  src/components/ExampleEventHeaderConsumer.cpp
  src/components/ExampleFailingConsumer.cpp
  src/components/ExampleFunctionalConsumer.cpp
  src/components/ExampleFunctionalConsumerKeyValues.cpp
  src/components/ExampleFunctionalConsumerMultiple.cpp
  src/components/ExampleFunctionalConsumerMultipleKeyValues.cpp
  src/components/ExampleFunctionalConsumerRuntimeCollections.cpp
  src/components/ExampleFunctionalConsumerRuntimeCollectionsMultiple.cpp
  src/components/ExampleFunctionalConsumerInputOutputLocations.cpp
  src/components/ExampleFunctionalConsumerEventContext.cpp
  src/components/ExampleFunctionalFilter.cpp
  src/components/ExampleFunctionalMetadataConsumer.cpp
  src/components/ExampleFunctionalMetadataProducer.cpp
  src/components/ExampleFunctionalProducer.cpp
  src/components/ExampleFunctionalProducerMultiple.cpp
  src/components/ExampleFunctionalProducerRuntimeCollections.cpp
  src/components/ExampleFunctionalTransformer.cpp
  src/components/ExampleFunctionalTransformerHist.cpp
  src/components/ExampleFunctionalTransformerMultiple.cpp
  src/components/ExampleFunctionalTransformerMultipleEventContext.cpp
  src/components/ExampleFunctionalTransformerRuntimeCollections.cpp
  src/components/ExampleFunctionalTransformerRuntimeCollectionsMultiple.cpp
  src/components/ExampleFunctionalTransformerRuntimeEmpty.cpp
  src/components/ExampleFunctionalTransformerInputOutputLocations.cpp
  src/components/ExampleFunctionalMultiTransformerInputOutputLocations.cpp
  src/components/ExampleFunctionalTransformerEventContext.cpp
  src/components/ExampleGaudiFunctionalProducer.cpp
  src/components/ExampleParticleIDConsumer.cpp
  src/components/ExampleParticleIDProducer.cpp
  src/components/ExampleRNGSeedingAlg.cpp
  src/components/k4FWCoreTest_AlgorithmWithTFile.cpp
  src/components/k4FWCoreTest_cellID_reader.cpp
  src/components/k4FWCoreTest_cellID_writer.cpp
  src/components/k4FWCoreTest_CheckExampleEventData.cpp
  src/components/k4FWCoreTest_CreateExampleEventData.cpp
  src/components/k4FWCoreTest_CreateMarlinWrapperCollection.cpp
  src/components/k4FWCoreTest_HelloWorldAlg.cpp
  src/components/TestUniqueIDGenSvc.cpp
  src/components/TypeMisMatchDemo.cpp
  src/components/ExampleTupleWriter.cpp
)

gaudi_add_module(k4FWCoreTestPlugins
                 SOURCES ${k4fwcoretest_plugin_sources}
                 LINK Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree podio::podioIO EDM4HEP::edm4hep)

set(K4RUN ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run)


#[[
add_test_fwcore
-----------------

Adds a test with custom properties and environment setup.

.. command:: add_test_fwcore(testname [test_arguments...] [PROPERTIES property1 value1...] [ADD_TO_CHECK_FILES])

  :param testname: Name of the test
  :param test_arguments: Arguments passed to the test command

  Supports two special keywords:

  - ``PROPERTIES``: Followed by property-value pairs to set on the test
  - ``ADD_TO_CHECK_FILES``: Adds the test to the 'FunctionalCheckFiles' test dependencies

  Key Features:
  * Uses k4run as the test command prefix
  * Handles multi-test dependencies by escaping semicolons in arguments

  Example::

    add_test_fwcore(FunctionalTest options/Example.py PROPERTIES TIMEOUT 300 ADD_TO_CHECK_FILES)
]]
function(add_test_fwcore testname)
  foreach(arg ${ARGN})
    if(arg STREQUAL "PROPERTIES")
      set(TEST_PROPERTIES_FOUND TRUE)
    elseif(arg STREQUAL "ADD_TO_CHECK_FILES")
      set(ADD_TO_CHECK_FILES TRUE)
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
  if(ADD_TO_CHECK_FILES)
    get_test_property(${testname} FIXTURES_SETUP existing_fixtures)
    if(existing_fixtures AND NOT existing_fixtures STREQUAL "NOTFOUND")
      set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP "${existing_fixtures};TestFiles")
    else()
      set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP TestFiles)
    endif()
  endif()
endfunction()

add_test(NAME FunctionalCheckFiles COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/CheckOutputFiles.py)
set_tests_properties(FunctionalCheckFiles PROPERTIES FIXTURES_REQUIRED TestFiles)

add_test_fwcore(CreateExampleEventData options/createExampleEventData.py)
set_tests_properties(CreateExampleEventData PROPERTIES FIXTURES_SETUP ExampleEventDataFile)
add_test_fwcore(CreateExampleEventDataInDirectory options/createExampleEventDataInDirectory.py)

add_test_fwcore(CheckExampleEventData options/checkExampleEventData.py PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(CheckExampleEventData_noCollections options/checkExampleEventData.py --collections= PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(CheckExampleEventData_toolong options/checkExampleEventData.py -n 999 PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(CheckExampleEventData_unbounded options/checkExampleEventData.py -n -1 PROPERTIES PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop" FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(ReadExampleEventData options/readExampleEventData.py)
set_property(TEST ReadExampleEventData APPEND PROPERTY FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(ReadExampleDataFromNthEvent options/readExampleDataFromNthEvent.py PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(ReadLimitedInputsk4DataSvc options/readLimitedSetOfCollectionsk4DataSvc.py ADD_TO_CHECK_FILES PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile)
add_test_fwcore(ReadLimitedInputsAllEventsk4DataSvc options/readLimitedSetOfCollectionsk4DataSvc.py -n -1 --PodioOutput.filename "output_k4test_exampledata_limited_allevents.root" ADD_TO_CHECK_FILES PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile)

add_test_fwcore(AlgorithmWithTFile options/TestAlgorithmWithTFile.py PROPERTIES FIXTURES_SETUP AlgorithmWithTFileFixture)
set_property(TEST AlgorithmWithTFile PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
add_test(NAME AlgorithmWithTFileCheckFrameworkOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND  python scripts/check_TestAlgorithmWithTFile_framework_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckFrameworkOutput APPEND PROPERTY FIXTURES_REQUIRED AlgorithmWithTFileFixture)
add_test(NAME AlgorithmWithTFileCheckMyTFileOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND  python scripts/check_TestAlgorithmWithTFile_myTFile_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckMyTFileOutput APPEND PROPERTY FIXTURES_REQUIRED AlgorithmWithTFileFixture)

add_test_fwcore(CreateExampleEventData_cellID options/createExampleEventData_cellID.py ADD_TO_CHECK_FILES)
add_test_fwcore(TwoProducers options/TwoProducers.py --filename output_k4fwcore_test_twoproducer.root
                  --magicNumberOffset.Producer2 12345 --Producer1.magicNumberOffset 54321)
add_test_fwcore(CheckCommandLineArguments options/createHelloWorld.py --HelloWorldAlg1.PerEventPrintMessage TwasBrilligAndTheSlithyToves
PROPERTIES PASS_REGULAR_EXPRESSION "TwasBrilligAndTheSlithyToves"
)

add_test_fwcore(TestUniqueIDGenSvc options/TestUniqueIDGenSvc.py -n 1)
add_test_fwcore(TestUniqueIDGenSvcRepeated options/TestUniqueIDGenSvc.py -n 2 PROPERTIES PASS_REGULAR_EXPRESSION "Duplicate ID for event number, run number and algorithm name")
add_test_fwcore(TestEventHeaderFiller options/createEventHeader.py PROPERTIES FIXTURES_SETUP EventHeaderFillerFile)
add_test_fwcore(EventHeaderCheck options/runEventHeaderCheck.py PROPERTIES FIXTURES_REQUIRED EventHeaderFillerFile)
add_test(NAME TestExec WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} COMMAND python options/TestExec.py)

add_test(NAME Testk4runNoArgumentsHelp COMMAND ${K4RUN})
set_tests_properties(Testk4runNoArgumentsHelp PROPERTIES PASS_REGULAR_EXPRESSION "Usage: k4run <options_file.py>, use --help to get a complete list of arguments")
add_test_fwcore(Testk4runCustomArguments options/TestArgs.py --foo=42 PROPERTIES PASS_REGULAR_EXPRESSION "The answer is 42")
add_test_fwcore(Testk4runVerboseOutput options/TestArgs.py --log-level=verbose PROPERTIES PASS_REGULAR_EXPRESSION " VERBOSE ")
add_test_fwcore(Testk4runHelpOnly options/TestArgs.py --help PROPERTIES PASS_REGULAR_EXPRESSION "show this help message and exit")

add_test_fwcore(TestEventCounter options/TestEventCounter.py)
set_tests_properties(TestEventCounter PROPERTIES FAIL_REGULAR_EXPRESSION "Processing event 1;Processing event 3"
                     PASS_REGULAR_EXPRESSION "(Processing event 0.*Processing event 2|Processing event 2.*Processing event 0).*Processed 4 events.*Processed 4 events")

add_test_fwcore(FunctionalMemory options/ExampleFunctionalMemory.py)
add_test_fwcore(FunctionalMemoryEventContext options/ExampleFunctionalMemory.py --use-event-context)
add_test_fwcore(FunctionalMemoryKeyValues options/ExampleFunctionalMemory.py --use-key-values)
add_test_fwcore(FunctionalMTMemory options/ExampleFunctionalMTMemory.py)
add_test_fwcore(FunctionalMultipleMemory options/ExampleFunctionalMultipleMemory.py)
add_test_fwcore(FunctionalMultipleMemoryEventContext options/ExampleFunctionalMultipleMemory.py --use-event-context)
add_test_fwcore(FunctionalProducer options/ExampleFunctionalProducer.py PROPERTIES FIXTURES_SETUP ProducerFile)
add_test_fwcore(FunctionalProducerAnother options/ExampleFunctionalProducer.py --second PROPERTIES FIXTURES_SETUP AnotherProducerFile)
add_test_fwcore(FunctionalProducerSingleEvent options/ExampleFunctionalProducer.py --num-events=1 --IOSvc.Output=functional_producer_single_event.root PROPERTIES FIXTURES_SETUP ProducerFileSingleEvent)

add_test_fwcore(ExampleFailingSingleEvent options/ExampleFailingSingleEvent.py
   PROPERTIES
     FIXTURES_REQUIRED ProducerFileSingleEvent
     WILL_FAIL TRUE
   )

# PODIO_DEFAULT_WRITE_RNTUPLE needs to be unset because the test FunctionalMix reads through
# PodioInput, which does not support RNTuple output
add_test(NAME FunctionalProducerMultiple
         COMMAND bash -c "unset PODIO_DEFAULT_WRITE_RNTUPLE; ${K4RUN} ${CMAKE_CURRENT_LIST_DIR}/options/ExampleFunctionalProducerMultiple.py"
)
set_tests_properties(FunctionalProducerMultiple PROPERTIES FIXTURES_SETUP ProducerMultipleFile)

add_test_fwcore(FunctionalProducerAbsolutePath options/ExampleFunctionalProducerAbsolutePath.py ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalFile options/ExampleFunctionalFile.py PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalSeveralInputFiles options/ExampleFunctionalSeveralInputFiles.py PROPERTIES FIXTURES_REQUIRED "ProducerFile\\\\;AnotherProducerFile")
add_test_fwcore(FunctionalMTFile options/ExampleFunctionalMTFile.py PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMultipleFile options/ExampleFunctionalFileMultiple.py PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMix options/runFunctionalMix.py PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMixIOSvc options/runFunctionalMix.py --iosvc PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalOutputCommands options/ExampleFunctionalOutputCommands.py PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalConsumerRuntimeCollections options/ExampleFunctionalConsumerRuntimeCollections.py)
add_test_fwcore(FunctionalConsumerRuntimeCollectionsMultiple options/ExampleFunctionalConsumerRuntimeCollectionsMultiple.py)
add_test_fwcore(FunctionalProducerRuntimeCollections options/ExampleFunctionalProducerRuntimeCollections.py)
add_test_fwcore(FunctionalTransformerRuntimeCollections options/ExampleFunctionalTransformerRuntimeCollections.py)
add_test_fwcore(FunctionalTransformerRuntimeEmpty options/ExampleFunctionalTransformerRuntimeEmpty.py ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalTransformerRuntimeCollectionsMultiple options/ExampleFunctionalTransformerRuntimeCollectionsMultiple.py)
add_test_fwcore(FunctionalTransformerHist options/ExampleFunctionalTransformerHist.py ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalCollectionMerger options/ExampleFunctionalCollectionMerger.py PROPERTIES FIXTURES_REQUIRED ProducerMultipleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalFilterFile options/ExampleFunctionalFilterFile.py ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMetadata options/ExampleFunctionalMetadata.py ADD_TO_CHECK_FILES PROPERTIES FIXTURES_SETUP FunctionalMetadataFile)
add_test_fwcore(FunctionalMetadataRead options/ExampleFunctionalMetadataRead.py PROPERTIES FIXTURES_REQUIRED FunctionalMetadataFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMetadataOldAlgorithm options/ExampleFunctionalMetadataOldAlgorithm.py ADD_TO_CHECK_FILES PROPERTIES FIXTURES_SETUP OldAlgorithmFile)
add_test_fwcore(createEventHeaderConcurrent options/createEventHeaderConcurrent.py ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalMetadataReadOldAlgorithm options/ExampleFunctionalMetadataReadOldAlgorithm.py PROPERTIES FIXTURES_REQUIRED OldAlgorithmFile ADD_TO_CHECK_FILES)

add_test_fwcore(FunctionalNonExistingFile options/ExampleFunctionalNonExistingFile.py)
set_tests_properties(FunctionalNonExistingFile PROPERTIES PASS_REGULAR_EXPRESSION "does not exist")
add_test_fwcore(FunctionalNonExistingFileOverwritten options/ExampleFunctionalNonExistingFile.py --IOSvc.Input functional_producer.root PROPERTIES FIXTURES_REQUIRED ProducerFile)
add_test_fwcore(FunctionalFileTooLong options/ExampleFunctionalFile.py -n 999 --IOSvc.Output toolong.root PROPERTIES FIXTURES_REQUIRED ProducerFile PASS_REGULAR_EXPRESSION
                  "Application Manager Terminated successfully with a user requested ScheduledStop")
add_test_fwcore(FunctionalFileTooShort options/ExampleFunctionalFile.py -n 2 --IOSvc.Output two_events.root PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalFileCLI options/ExampleFunctionalNoFile.py --IOSvc.Input functional_producer.root --IOSvc.Output functional_transformer_cli.root ADD_TO_CHECK_FILES)
set_tests_properties(FunctionalFileCLI PROPERTIES FIXTURES_REQUIRED ProducerFile)
add_test_fwcore(FunctionalFileCLIMultiple options/ExampleFunctionalNoFile.py --IOSvc.Input functional_producer.root functional_producer.root --IOSvc.Output functional_transformer_cli_multiple.root -n -1 ADD_TO_CHECK_FILES)
set_tests_properties(FunctionalFileCLIMultiple PROPERTIES FIXTURES_REQUIRED ProducerFile)

add_test_fwcore(FunctionalWrongImport options/ExampleFunctionalWrongImport.py)
set_tests_properties(FunctionalWrongImport PROPERTIES PASS_REGULAR_EXPRESSION "ImportError: Importing ApplicationMgr or IOSvc from Configurables is not allowed.")
add_test_fwcore(FunctionalReadNthEvent options/ExampleFunctionalReadNthEvent.py PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalProducerRNTuple options/ExampleFunctionalProducer.py --IOSvc.OutputType RNTuple --IOSvc.Output functional_producer_rntuple.root ADD_TO_CHECK_FILES PROPERTIES FIXTURES_SETUP FunctionalRNTupleFile)
add_test_fwcore(FunctionalFileRNTuple options/ExampleFunctionalFile.py --IOSvc.OutputType RNTuple --IOSvc.Input functional_producer_rntuple.root --IOSvc.Output functional_producer_rntuple_file.root PROPERTIES FIXTURES_REQUIRED FunctionalRNTupleFile ADD_TO_CHECK_FILES)
add_test_fwcore(FunctionalTTreeToRNTuple options/ExampleFunctionalTTreeToRNTuple.py PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(GaudiFunctional options/ExampleGaudiFunctional.py PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(ReadLimitedInputsIOSvc options/ExampleIOSvcLimitInputCollections.py PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile ADD_TO_CHECK_FILES)
add_test_fwcore(ReadLimitedInputsAllEventsIOSvc options/ExampleIOSvcLimitInputCollections.py --IOSvc.Output "functional_limited_input_all_events.root" -n -1 PROPERTIES FIXTURES_REQUIRED ExampleEventDataFile ADD_TO_CHECK_FILES)

add_test_fwcore(ExampleTupleWriterWithOutput options/ExampleFunctionalFile.py --add-ttree-ntuple --IOSvc.Output functional_with_tuple.root
  PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)
add_test_fwcore(ExampleTupleWriterRNTupleWithOutput options/ExampleFunctionalFile.py --add-ttree-ntuple --use-rntuple-for-ntuple --TupleWriter.OutputFile ntuple_rntuple.root --IOSvc.Output functional_with_tuple_rntuple.root
  PROPERTIES FIXTURES_REQUIRED ProducerFile ADD_TO_CHECK_FILES)

# Tests that ensure that load_file populates the necessary python globals
# accordingly to make loaded files work like imported files
# __file__ points to the loaded file and not k4FWCore/python/utils.py
add_test_fwcore(CheckLoadedFilesHaveCorrectDunderFile options/checkLoadedFileProperties.py)
# If there is an error in the loaded file the filename of that file needs to
# appear in the output
add_test_fwcore(CheckLoadedFileCorrectPathOnError options/checkLoadedFileProperties.py --with-error)
set_tests_properties(CheckLoadedFileCorrectPathOnError
  PROPERTIES PASS_REGULAR_EXPRESSION [=[  File ".*/test/k4FWCoreTest/options/checkLoadedFileProperties.py", line 36, in <module>]=]
)

add_test_fwcore(ParticleIDMetadataFramework options/ExampleParticleIDMetadata.py PROPERTIES FIXTURES_SETUP ParticleIDMetadataFile)

add_test_fwcore(EfficiencyFilter options/ExampleEfficiencyFilter.py PROPERTIES FIXTURES_REQUIRED "ProducerFile\\\\;EventHeaderFillerFile" ADD_TO_CHECK_FILES)

add_test_fwcore(FunctionalTransformerInputOutputLocations options/ExampleFunctionalTransformerInputOutputLocations.py)
add_test_fwcore(FunctionalMultiTransformerInputOutputLocations options/ExampleFunctionalMultiTransformerInputOutputLocations.py)
add_test_fwcore(FunctionalConsumerInputOutputLocations options/ExampleFunctionalConsumerInputOutputLocations.py)

add_executable(check_ParticleIDOutputs src/check_ParticleIDOutputs.cpp)
target_link_libraries(check_ParticleIDOutputs PRIVATE podio::podioIO EDM4HEP::edm4hep EDM4HEP::utils fmt::fmt)
add_test(NAME check_ParticleIDOutputs COMMAND check_ParticleIDOutputs example_with_particleids.root)
set_tests_properties(check_ParticleIDOutputs PROPERTIES FIXTURES_REQUIRED ParticleIDMetadataFile)
add_test_fwcore(TypeMisMatchDemo options/TypeMisMatchDemo.py PROPERTIES PASS_REGULAR_EXPRESSION "Failed to cast collection MCParticles to the required type")

add_test_fwcore(TypeMisMatchDemoMultiple options/TypeMisMatchDemoMultiple.py PROPERTIES PASS_REGULAR_EXPRESSION "Failed to cast collection MCParticles1 to the required type")
add_test_fwcore(ReadMarlinWrapperCollection options/readMarlinWrapperCollection.py)

add_test_fwcore(InvalidOutputCommandsNoCrash options/invalidOutputCommandsNoCrash.py)
set_tests_properties(InvalidOutputCommandsNoCrash PROPERTIES FIXTURES_REQUIRED ProducerFile PASS_REGULAR_EXPRESSION "ERROR 'abc' is not a valid command for the KeepDropSwitch")

add_test(NAME check_broken_pipe
         COMMAND bash -c "[ $(${K4RUN} options/ExampleFunctionalProducer.py | head -n 2 | wc -l) = 2 ]"
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
       )

get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(test_environment "\
LD_LIBRARY_PATH=\
${PROJECT_BINARY_DIR}:\
${PROJECT_BINARY_DIR}/${PROJECT_NAME}:\
${CMAKE_CURRENT_BINARY_DIR}:\
${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${CURRENT_DIR_NAME}:\
$ENV{LD_LIBRARY_PATH};\
\
PYTHONPATH=\
${PROJECT_BINARY_DIR}/${PROJECT_NAME}/genConfDir:\
${CMAKE_CURRENT_BINARY_DIR}/genConfDir:\
$ENV{PYTHONPATH};\
"
)
get_property(test_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY TESTS)
set_tests_properties(${test_names} PROPERTIES ENVIRONMENT "${test_environment}")

# The following is done to make the tests work without installing the files in
# the installation directory. The k4FWCore in the build directory is populated by
# Gaudi so we can't just make a k4FWCore folder and put the python files there.
# We put everything in one of the folders generated by Gaudi so that importing from
# k4FWCore will give the right result
add_custom_target(copy_directory ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${PROJECT_SOURCE_DIR}/python/k4FWCore/ ${PROJECT_BINARY_DIR}/k4FWCore/genConfDir/k4FWCore
  COMMENT "Copying k4FWCore python package after building all the libraries and plugins"
  DEPENDS k4FWCoreTestPlugins k4FWCore k4FWCorePlugins
)
