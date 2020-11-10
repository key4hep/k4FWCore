
source init.sh
mkdir build install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_CXX_STANDARD=17 -DHOST_BINARY_TAG=x86_64-linux-gcc9-opt .. && \
make -j4 && \
make install && \
ctest --output-on-failure
