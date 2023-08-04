# k4FWCore (key4hep FrameWork Core)

k4FWCore is a Gaudi package that provides the PodioDataService, which allows to
use podio-based event data models like EDM4hep in Gaudi workflows.

k4FWCore also provides the `k4run` script used to run Gaudi steering files.

## Components

### Basic I/O

#### k4DataSvc

Component wrapping the PodioDataService to handle PODIO types and collections.

#### PodioInput

Algorithm to read data from one or multiple input file(s) on disk.

#### PodioOutput

Algorithm to write data to an output file on disk.

## k4run
```bash
$ k4run --help
usage: k4run [-h] [--dry-run] [-v] [-n NUM_EVENTS] [-l] [--gdb] [--ncpus NCPUS] [config_files ...]

Run job in the Key4HEP framework

positional arguments:
  config_files          Gaudi config (python) files describing the job

options:
  -h, --help            show this help message and exit
  --dry-run             Do not actually run the job, just parse the config files
  -v, --verbose         Run job with verbose output
  -n NUM_EVENTS, --num-events NUM_EVENTS
                        Number of events to run
  -l, --list            Print all the configurable components available in the framework and exit
  --gdb                 Attach gdb debugger
  --ncpus NCPUS         Start Gaudi in parallel mode using NCPUS processes. 0 => serial mode (default), -1 => use all CPUs
```
When supplied with a Gaudi steering file `k4run --help file.py` also shows the settable properties of the Gaudi algorithms used in the file. Additionally, it is possible to add further arguments and use them in the steering file by using the python `argparse.ArgumentParser` shared by `k4run`.
```python
parser.add_argument("-f", "--foo", type=int, help="hello world")
my_opts = parser.parse_known_args()
print(my_opts[0].foo)
```

## Dependencies

* ROOT

* PODIO

* Gaudi

## Installation and downstream usage.

k4FWCore is a CMake project. After setting up the dependencies (use for example `source /cvmfs/sw.hsf.org/key4hep/setup.sh`)


```
mkdir build install
cd build;
cmake ..
make install
```


