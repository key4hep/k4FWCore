CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)


#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

gaudi_project(K4FWCore HEAD
              USE Gaudi v32r0 )
