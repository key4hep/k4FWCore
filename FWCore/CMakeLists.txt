################################################################################
# Package: FWCore
################################################################################
gaudi_subdir(FWCore v1r0)

find_package(podio)

# this declaration will not be needed in the future
gaudi_depends_on_subdirs(GaudiAlg GaudiKernel)
gaudi_install_scripts()
gaudi_install_python_modules()

gaudi_add_library(FWCore
		  src/*.cpp
                  INCLUDE_DIRS podio ROOT
                  LINK_LIBRARIES GaudiAlgLib GaudiKernel podio::podioRootIO ROOT
                  PUBLIC_HEADERS FWCore)

gaudi_add_module(FWCorePlugins
                 src/components/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel FWCore ROOT)

gaudi_add_test(FWWriteTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               FRAMEWORK test/options/simple_write.py)
