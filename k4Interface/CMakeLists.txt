################################################################################
# Package: SimG4Interface
################################################################################


gaudi_add_header_only_library(k4Interface)

install(TARGETS k4Interface
  EXPORT k4FWCoreTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
