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

gaudi_install(SCRIPTS)

gaudi_add_library(k4FWCore
                  SOURCES src/PodioDataSvc.cpp
                          src/KeepDropSwitch.cpp
                  LINK Gaudi::GaudiKernel podio::podioIO ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::utils
                  )
target_include_directories(k4FWCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

gaudi_add_module(k4FWCorePlugins
                 SOURCES
                   components/CollectionMerger.cpp
                   components/EventCounter.cpp
                   components/EventHeaderCreator.cpp
                   components/FCCDataSvc.cpp
                   components/IOSvc.cpp
                   components/MetadataSvc.cpp
                   components/PodioInput.cpp
                   components/PodioOutput.cpp
                   components/Reader.cpp
                   components/UniqueIDGenSvc.cpp
                   components/Writer.cpp
                   components/k4DataSvc.cpp
                 LINK Gaudi::GaudiKernel k4FWCore k4FWCore::k4Interface ROOT::Core ROOT::RIO ROOT::Tree EDM4HEP::edm4hep)

target_include_directories(k4FWCorePlugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS k4FWCore k4FWCorePlugins
  EXPORT k4FWCoreTargets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
