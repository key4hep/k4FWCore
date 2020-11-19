# k4FWCore (Key4HEP FrameWork Core)


k4FWCore is a Gaudi package that  provides the PodioDataService, that allows to use podio-based event data models like edm4hep in gaudi workflows.


## Components

### Basic I/O

#### k4DataSvc

Component wrapping the PodioDataService to handle PODIO types and collections.


#### PodioInput 

Algorithm to read data from input file(s) on disk. 

#### PodioOutput

Algorithm to write data to output file on disk.

#### FWFloatProducer

Algorithm with a simple float as output, for testing and adding parameters to jobs. 


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


