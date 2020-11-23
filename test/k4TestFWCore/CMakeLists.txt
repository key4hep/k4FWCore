################################################################################
# Package: k4TestFWCore
################################################################################

find_package(EDM4HEP)


gaudi_subdir(k4TestFWCore v1r0)

gaudi_depends_on_subdirs(GaudiAlg GaudiKernel)


file(GLOB k4testfwcore_plugin_sources src/components/*.cpp)
gaudi_add_module(k4TestFWCorePlugins
                 SOURCES ${k4testfwcore_plugin_sources}
                 LINK GaudiAlgLib GaudiKernel k4FWCore k4FWCorePlugins ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hepDict EDM4HEP::edm4hep)


include(CTest)

set(K4RUN ${CMAKE_BINARY_DIR}/run ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run)

add_test(NAME CreateExampleEventData
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
               COMMAND ${K4RUN} k4TestFWCore/options/createExampleEventData.py)

add_test(NAME ReadExampleEventData
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
               COMMAND ${K4RUN} k4TestFWCore/options/readExampleEventData.py)

