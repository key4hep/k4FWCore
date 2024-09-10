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
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(k4FWCore)

find_package(ROOT COMPONENTS RIO Tree REQUIRED)
find_package(Gaudi REQUIRED)
  find_package(podio 0.16.3)  # this will not find 1.0 and newer
  if(NOT podio_FOUND)
    # we try to find a newer version now
    find_package(podio 1.0 REQUIRED)
  endif()
find_package(EDM4HEP REQUIRED)

include(cmake/Key4hepConfig.cmake)

include(GNUInstallDirs)
include(CTest)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/InstallArea/ CACHE PATH
    "Install path prefix, prepended onto install directories." FORCE )
endif()

add_subdirectory(k4FWCore)
add_subdirectory(k4Interface)
add_subdirectory(python)
if(BUILD_TESTING)
  add_subdirectory(test/k4FWCoreTest)
endif()


option(ENABLE_CPACK "Whether or not to use cpack config" OFF)
if(ENABLE_CPACK)
  include(cmake/${PROJECT_NAME}CPack.cmake)
endif()


install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/"
  )

gaudi_install(CMAKE cmake/${PROJECT_NAME}Config.cmake)

