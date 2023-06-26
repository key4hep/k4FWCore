# k4FWCore (key4hep FrameWork Core)

k4FWCore is a Gaudi package that provides the PodioDataService, that allows to
use podio-based event data models like EDM4hep in Gaudi workflows.

## Components

### Basic I/O

#### k4DataSvc

Component wrapping the PodioDataService to handle PODIO types and collections.

#### PodioInput

Algorithm to read data from input file(s) on disk.

#### PodioOutput

Algorithm to write data to output file on disk.


## Dependencies

* ROOT

* PODIO

* Gaudi

## Installation and downstream usage.

k4FWCore is a cmake project. After setting up the dependencies (use for example `source /cvmfs/sw.hsf.org/key4hep/setup.sh`)


```
mkdir build install
cd build;
cmake ..
make install
```


