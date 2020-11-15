
include(CMakeFindDependencyMacro)
find_dependency(podio REQUIRED)
find_dependency(Gaudi REQUIRED)

# - Include the targets file to create the imported targets that a client can
# link to (libraries) or execute (programs)
include("${CMAKE_CURRENT_LIST_DIR}/k4FWCoreTargets.cmake")

get_property(TEST_K4FWCORE_LIBRARY TARGET k4FWCore::k4FWCore PROPERTY LOCATION)
find_package_handle_standard_args(k4FWCore  DEFAULT_MSG CMAKE_CURRENT_LIST_FILE TEST_K4FWCORE_LIBRARY)
