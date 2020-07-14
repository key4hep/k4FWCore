set(CPACK_PACKAGE_DESCRIPTION "K4FWCore: core components of the key4hep framework")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "K4FWCore: core components of the key4hep framework")
set(CPACK_PACKAGE_VENDOR "HEP Software Foundation")
set(CPACK_PACKAGE_VERSION ${K4FWCORE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${K4FWCORE_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${K4FWCORE_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${K4FWCORE_PATCH_VERSION})

set(CPACK_PACKAGE_NAME "k4fwcore")
set(CPACK_SET_DESTDIR TRUE)
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/key4hep/k4fwcore")

# required fields for .deb
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "valentin.volkl@cern.ch")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${CPACK_PACKAGE_HOMEPAGE_URL})


#--- source package settings ---------------------------------------------------
set(CPACK_SOURCE_IGNORE_FILES
    ${PROJECT_BINARY_DIR}
    "~$"
    "/.git/"
    "/\\\\\\\\.git/"
    "/#"
)
set(CPACK_SOURCE_STRIP_FILES "")

#--- translate buildtype -------------------------------------------------------
if(NOT CMAKE_CONFIGURATION_TYPES)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" HSF_DEFAULT_BUILDTYPE)
endif()

set(HSF_BUILDTYPE "unknown")

if(HSF_DEFAULT_BUILDTYPE STREQUAL "release")
  set(HSF_BUILDTYPE "opt")
elseif(HSF_DEFAULT_BUILDTYPE STREQUAL "debug")
  set(HSF_BUILDTYPE "dbg")
elseif(HSF_DEFAULT_BUILDTYPE STREQUAL "relwithdebinfo")
  set(HSF_BUILDTYPE "owd")
endif()

set(CPACK_PACKAGE_RELOCATABLE True)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "K4FWCore")
set(CPACK_PACKAGE_FILE_NAME "K4FWCore")

include(CPack)
