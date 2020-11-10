################################################################################
# Package: k4FWCore
################################################################################
gaudi_subdir(k4FWCore v1r0)

find_package(podio)

gaudi_install_scripts()
gaudi_install_python_modules()


gaudi_add_module(k4FWCore
                 src/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel podio::podioRootIO)

gaudi_add_test(k4FWWriteTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               FRAMEWORK test/options/simple_write.py)
