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
cmake_minimum_required(VERSION 3.11)


project(DownstreamProjectUsingk4FWCore)

find_package(k4FWCore)

if (NOT DEFINED ${k4FWCore_VERSION})
  message(FATAL_ERROR "CAnnot find k4FWCore version information")
endif()

if (NOT ${k4FWCore_VERSION} MATCHES "^[0-9]+\\.[0-9+](\\.[0-9]+)?$")
  message(FATEL_ERROR "Exported k4FWCore version cannot be used as a version: " ${k4FWCore_VERSION})
endif()

add_executable(appUsingk4FWCore main.cxx)
target_link_libraries(appUsingk4FWCore k4FWCore::k4FWCore)
