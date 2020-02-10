# see gh issue #2
#source /cvmfs/sft.cern.ch/lcg/views/LCG_96c_LS/x86_64-centos7-gcc8-opt/setup.sh

source /cvmfs/fcc.cern.ch/sw/latest/setup.sh

# workaround for a bug in the Gaudi CMake configuration,
# can be removed in the future
export Gaudi_DIR=/cvmfs/sft.cern.ch/lcg/releases/LCG_96c_LS/Gaudi/v32r2/x86_64-centos7-gcc8-opt/

