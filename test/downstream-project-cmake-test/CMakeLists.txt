cmake_minimum_required(VERSION 3.11)


project(DownstreamProjectUsingk4FWCore)

find_package(k4FWCore)

add_executable(appUsingk4FWCore main.cxx)
target_link_libraries(appUsingk4FWCore k4FWCore::k4FWCore)
