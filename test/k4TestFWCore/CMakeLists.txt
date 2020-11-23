################################################################################
# Package: k4TestFWCore
################################################################################

find_package(EDM4HEP)




file(GLOB k4testfwcore_plugin_sources src/components/*.cpp)
gaudi_add_module(k4TestFWCorePlugins
                 SOURCES ${k4testfwcore_plugin_sources}
                 LINK GaudiAlgLib GaudiKernel k4FWCore ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hepDict EDM4HEP::edm4hep)


include(CTest)

set(K4RUN ${CMAKE_BINARY_DIR}/run ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run)

add_test(NAME CreateExampleEventData
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND ${K4RUN} options/createExampleEventData.py)

add_test(NAME ReadExampleEventData
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND ${K4RUN} options/readExampleEventData.py)

add_test(NAME AlgorithmWithTFile
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND ${K4RUN} options/TestAlgorithmWithTFile.py)

add_test(NAME AlgorithmWithTFileCheckFrameworkOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND python scripts/check_TestAlgorithmWithTFile_framework_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckFrameworkOutput APPEND PROPERTY DEPENDS AlgorithmWithTFile)

add_test(NAME AlgorithmWithTFileCheckMyTFileOutput
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND python scripts/check_TestAlgorithmWithTFile_myTFile_nonempty.py)
set_property(TEST AlgorithmWithTFileCheckMyTFileOutput APPEND PROPERTY DEPENDS AlgorithmWithTFile)
