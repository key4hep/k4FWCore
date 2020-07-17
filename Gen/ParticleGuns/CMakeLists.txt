################################################################################
# Package: ParticleGuns
################################################################################
gaudi_subdir(ParticleGuns v1r0)

find_package(Pythia8)
find_package(HepPDT)

gaudi_add_module(Generation
                 src/*.cpp
                 INCLUDE_DIRS ParticleGuns FWCore Pythia8 HepPDT
                 LINK_LIBRARIES GaudiAlgLib FWCore Pythia8 HepPDT )

include(CTest)

gaudi_add_test(particleGun
               FRAMEWORK options/particleGun.py
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})


