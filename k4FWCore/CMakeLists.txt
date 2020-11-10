################################################################################
# Package: k4FWCore
################################################################################
gaudi_subdir(k4FWCore v1r0)

find_package(podio)

gaudi_install_scripts()


gaudi_add_library(k4FWCore
                 src/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel podio::podioRootIO
                  PUBLIC_HEADERS k4FWCore
                  )

gaudi_add_module(k4FWCorePlugins
                 components/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel k4FWCore)

gaudi_add_test(k4FWWriteTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               FRAMEWORK test/options/simple_write.py)
