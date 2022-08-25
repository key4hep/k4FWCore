################################################################################
# Package: k4FWCore
################################################################################

find_package(podio)

gaudi_install(SCRIPTS)


gaudi_add_library(k4FWCore
		  SOURCES src/PodioDataSvc.cpp
              src/KeepDropSwitch.cpp
                  LINK Gaudi::GaudiAlgLib Gaudi::GaudiKernel podio::podioRootIO ROOT::Core ROOT::RIO ROOT::Tree
                  )
target_include_directories(k4FWCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

file(GLOB k4fwcore_plugin_sources components/*.cpp)
gaudi_add_module(k4FWCorePlugins
                 SOURCES ${k4fwcore_plugin_sources}
                 LINK Gaudi::GaudiAlgLib Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree)
target_include_directories(k4FWCorePlugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS k4FWCore k4FWCorePlugins
  EXPORT k4FWCoreTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)


add_test(NAME WriteTest
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND ${CMAKE_BINARY_DIR}/run ${PROJECT_SOURCE_DIR}/k4FWCore/scripts/k4run k4FWCore/test/options/simple_write.py)

