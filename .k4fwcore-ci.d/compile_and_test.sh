##
## Copyright (c) 2014-2024 Key4hep-Project.
##
## This file is part of Key4hep.
## See https://key4hep.github.io/key4hep-doc/ for further info.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
mkdir build
cd build
git clone --depth 1 https://github.com/key4hep/spack
git clone --depth 1 https://github.com/key4hep/key4hep-spack
cp key4hep-spack/config/packages.yaml spack/etc/spack
cp key4hep-spack/config/cvmfs_build/upstreams.yaml spack/etc/spack
source spack/share/spack/setup-env.sh
spack repo add --scope site key4hep-spack
# setup compiler for spack to find
source /cvmfs/sw.hsf.org/key4hep/setup.sh
spack compiler find --scope site
cd ..
spack dev-build --test=all k4fwcore@master


spack load k4fwcore@master
cd test/downstream-project-cmake-test
mkdir build install
cd build
cmake -DCMAKE_CXX_STANDARD=${STANDARD} -DCMAKE_INSTALL_PREFIX=../install -G Ninja .. \
      && ninja -k0




