################################################################################
# Package: TestFWCore
################################################################################

find_package(EDM4HEP)

if (NOT EDM4HEP_FOUND)
  RETURN()
ENDIF()


gaudi_subdir(TestFWCore v1r0)

gaudi_depends_on_subdirs(GaudiAlg GaudiKernel FWCore)


gaudi_add_module(TestFWCorePlugins
                 src/components/*.cpp
                 INCLUDE_DIRS FWCore
                 LINK_LIBRARIES GaudiKernel FWCore)


include(CTest)

gaudi_add_test(CreateExampleEventData
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               COMMAND gaudirun.py Test/TestFWCore/options/createExampleEventData.py  --filename=output_k4test_01.root)

