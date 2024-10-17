include(CMakePackageConfigHelpers)

# Version file is same wherever we are
write_basic_package_version_file(${PROJECT_BINARY_DIR}/k4FWCoreConfigVersion.cmake
                                 VERSION ${k4FWCore_VERSION}
                                 COMPATIBILITY SameMajorVersion)


configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/k4FWCoreConfig.cmake.in
                              ${PROJECT_BINARY_DIR}/k4FWCoreConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/k4FWCore
                              PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/k4FWCoreConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/k4FWCoreConfigVersion.cmake
              DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/"
)
