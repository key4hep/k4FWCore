################################################################################
# Package: SimDelphes
################################################################################
gaudi_subdir(SimDelphes v1r0)

# this declaration will not be needed in the future
gaudi_depends_on_subdirs(GaudiAlg)

find_package(Delphes)

gaudi_add_module(SimDelphes
                 src/*.cpp
                 INCLUDE_DIRS FWCore ROOT Delphes
                 LINK_LIBRARIES FWCore GaudiAlgLib Delphes ROOT)
#target_link_libraries(SimDelphes EDM4hep::edm4hep)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/Sim/SimDelphes)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/options DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/Sim/SimDelphes)

include(CTest)
gaudi_add_test(PythiaDelphes
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
               FRAMEWORK options/PythiaDelphes_config.py)
