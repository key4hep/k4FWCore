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

################################################################################
# Package: k4FWCore
################################################################################

find_package(podio REQUIRED)
find_package(EDM4HEP)

gaudi_install(SCRIPTS)
gaudi_install(PYTHON)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

gaudi_add_library(k4FWCore
		  SOURCES src/PodioDataSvc.cpp
              src/KeepDropSwitch.cpp
                  LINK Gaudi::GaudiKernel podio::podioRootIO ROOT::Core ROOT::RIO ROOT::Tree
                  )
target_include_directories(k4FWCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

file(GLOB headers /home/juanmi/Key4hep/Key4hep-all/k4FWCore/include/k4FWCore/FunctionalUtils.h)
add_library(my_module)
set_target_properties(my_module PROPERTIES LINKER_LANGUAGE CXX)
# # target_sources(my_module PUBLIC FILE_SET CXX_MODULES FILES /home/juanmi/Key4hep/Key4hep-all/k4FWCore/include/k4FWCore/FunctionalUtils.h)
target_sources(my_module
  PUBLIC
  FILE_SET cxx_modules
  TYPE
  CXX_MODULES
  FILES
  include/k4FWCore/FunctionalUtils.ixx
)
target_link_libraries(my_module PUBLIC Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hep)
target_include_directories(my_module PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

file(GLOB k4fwcore_plugin_sources components/*.cpp)
gaudi_add_module(k4FWCorePlugins
                 SOURCES ${k4fwcore_plugin_sources}
                 LINK Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hep my_module)
target_include_directories(k4FWCorePlugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS k4FWCore k4FWCorePlugins
  EXPORT k4FWCoreTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

# Copy python parsing file to genConfDir in Gaudi
add_custom_command(
        TARGET k4FWCore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/k4FWCore/python/k4FWCore
                ${CMAKE_CURRENT_BINARY_DIR}/genConfDir/k4FWCore)

# This is needed to overwrite the __init__.py, see a long comment in the
# CMakeLists.txt in the test folder
add_custom_command(TARGET k4FWCorePlugins POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/python/k4FWCore/__init__.py ${PROJECT_BINARY_DIR}/k4FWCore/genConfDir/k4FWCore/__init__.py)
