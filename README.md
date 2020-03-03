# K4FWCore


K4FWCore (Key4HEP FrameWork Core) is a package comprising core Gaudi components for the Key4HEP software framework


## Components

### Basic I/O

#### PodioDataSvc


Data service extended to handle PODIO types and collections.


#### PodioInput 

Algorithm to read data from input file(s) on disk. 

#### PodioOutput

Algorithm to write data to output file on disk.

#### FWFloatProducer

Algorithm with a simple float as output, for testing and adding parameters to jobs. 

### Background Overlay


#### PileupOverlayAlg

Algorithm that overlays its input with data from an external (background) data source. 

#### RangePileup

Tool to specify pile-up parameters.

#### ConstPileUp

Tool to specify pile-up parameters.

#### PoissonPileUp

Tool to specify pile-up parameters.




## Dependencies

* ROOT

* PODIO

* Gaudi

## Installation

K4FWCore is a Gaudi-based project and uses the Gaudi CMake configuration


```
mkdir build install
cd build;
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make install


```


