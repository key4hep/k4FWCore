CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)


find_package(ROOT COMPONENTS RIO Tree)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

include(GNUInstallDirs)

if( NOT DEFINED ENV{LCG_VERSION})
  set(HOST_BINARY_TAG x86_64-linux-gcc9-opt)
endif()

# Set up C++ Standard
# ``-DCMAKE_CXX_STANDARD=<standard>`` when invoking CMake
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")


if(NOT CMAKE_CXX_STANDARD MATCHES "14|17")
  message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD}")
endif()

gaudi_project(k4FWCore v0r1
              USE Gaudi v32r2 )

option(ENABLE_CPACK "Whether or not to use cpack config" OFF)
if(ENABLE_CPACK)
  include(cmake/k4FWCoreCPack.cmake)
endif()
