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
```
$ k4run --help
usage: k4run [--dry-run] [-v] [-n NUM_EVENTS] [-l] [--gdb] [--interactive-root] [-h] [config_files ...]

Run job in the Key4hep framework

positional arguments:
  config_files          Gaudi config (python) files describing the job

options:
  --dry-run             Do not actually run the job, just parse the config files
  -v, --verbose         Run job with verbose output
  -n NUM_EVENTS, --num-events NUM_EVENTS
                        Number of events to run
  -l, --list            Print all the configurable components available in the framework and exit
  --gdb                 Attach gdb debugger
  --interactive-root    Run with ROOT in interactive mode (e.g. to see plots)
  -h, --help            show this help message and exit
```
When supplied with a Gaudi steering file `k4run --help file.py` also shows the settable properties of the Gaudi algorithms used in the file. Additionally, it is possible to add further arguments and use them in the steering file by using the Python `argparse.ArgumentParser` shared by `k4run`.
```python
from k4FWCore.parseArgs import parser
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

``` bash
mkdir build
cd build
cmake ..
make install
```

## Implementing algorithms
k4FWCore uses `Gaudi::Functional` for executing algorithms. There are several
types of algorithms, depending on your use case:
- The `Consumer` takes inputs but no outputs; can be used for reading
- The `Producer` takes outputs but no inputs; can be used for generating
  collections or events
- The `Transformer` is the more general one (both the `Consumer` and the
  `Producer` are a particular case of this one) and takes both inputs and
  outputs
- The `Filter`, for removing events from the events that are going to be
  processed.

A more complete list of algorithms can be found in
https://lhcb.github.io/DevelopKit/03a-gaudi/, in the `Gaudi::Functional`
section.

In all cases the implementation process is the same: we'll create a new class
that will inherit from one of the previous algorithms. Then, we implement
`operator()`, where our algorithm will be. This `operator()` will return either
a single type (including `void`) or a tuple with multiple types. It will take
one parameter per input. Simple examples can be found in the test folder for
each one of the above-mentioned algorithms. In addition, there are tests that
have either multiple inputs and / or multiple outputs (like
`ExampleFunctionalProducerMultiple`) that can be used as a template for the more
typical case when working with multiple inputs or outputs.
