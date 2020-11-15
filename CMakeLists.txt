CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

project(k4FWCore)

find_package(ROOT COMPONENTS RIO Tree)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(Gaudi)
#---------------------------------------------------------------

include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/ CACHE PATH
    "Install path prefix, prepended onto install directories." FORCE )
endif()

# Set up C++ Standard
# ``-DCMAKE_CXX_STANDARD=<standard>`` when invoking CMake
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT CMAKE_CXX_STANDARD MATCHES "14|17")
  message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD}")
endif()

add_subdirectory(k4FWCore)

option(ENABLE_CPACK "Whether or not to use cpack config" OFF)
if(ENABLE_CPACK)
  include(cmake/k4FWCoreCPack.cmake)
endif()

gaudi_install(CMAKE)

include(CTest)
