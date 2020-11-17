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




