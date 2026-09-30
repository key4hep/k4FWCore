include(CMakePackageConfigHelpers)

# Version file is same wherever we are
write_basic_package_version_file(${PROJECT_BINARY_DIR}/k4FWCoreConfigVersion.cmake
                                 VERSION ${k4FWCore_VERSION}
                                 COMPATIBILITY SameMajorVersion)


configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/k4FWCoreConfig.cmake.in
                              ${PROJECT_BINARY_DIR}/k4FWCoreConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/k4FWCore
                              PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR)

# Copy next to the build-tree config so that projects built together with
# k4FWCore can also use it
configure_file(${PROJECT_SOURCE_DIR}/cmake/k4FWCoreTesting.cmake
               ${PROJECT_BINARY_DIR}/k4FWCoreTesting.cmake COPYONLY)

install(FILES ${PROJECT_BINARY_DIR}/k4FWCoreConfig.cmake
              ${PROJECT_BINARY_DIR}/k4FWCoreConfigVersion.cmake
              ${PROJECT_BINARY_DIR}/k4FWCoreTesting.cmake
              DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/"
)
