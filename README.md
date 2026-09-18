# k4FWCore (key4hep FrameWork Core)

k4FWCore is a Gaudi package that provides the IOSvc, which allows to
use EDM4hep in Gaudi workflows.

k4FWCore also provides the `k4run` script used to run Gaudi steering files. See the [documentation](doc/k4run-args.md) for more information.

## Components

### Basic I/O

| Name | Description |
|---------|-|
| IOSvc | Service handling the PODIO types and collections |
| Reader | Algorithm to read data from input files on disk. |
| Writer | Algorithm to write data to an output file on disk. |
| MetadataSvc | Service/Handle handling user defined metadata |

### Collection Merger

Algorithm merging multiple collections of the same type into a single collection.

### EfficiencyFilter

Algorithm that takes a collection and creates a subset collection by randomly accepting elements according to the user-defined `Efficiency`.

### EventHeaderCreator

Algorithm creating a new `edm4hep::EventHeaderCollection` data object.

### EventCounter

Algorithm counting processed events and printing heart-bit.

### OverlayTiming

Algorithm overlaying background events on top of signal events, used, for example, to simulate beam-induced backgrounds. See its [documentation](doc/OverlayTiming.md) for more information.

### UniqueIDGenSvc

Service generating unique, reproducible numbers to be used for seeding RNG used by the algorithms. See the [documentation](doc/uniqueIDGen.md) for more information.

## k4run
```
$ k4run --help
usage: k4run [--dry-run] [-v] [-n NUM_EVENTS] [-l] [--gdb] [--interactive-root] [--log-level {VERBOSE,DEBUG,INFO,WARNING,ERROR}] [-i] [-h] [config_files ...]

Run job in the Key4hep framework

positional arguments:
  config_files          Gaudi config (python) files describing the job

options:
  --dry-run             Do not actually run the job, just parse the config files
  -n NUM_EVENTS, --num-events NUM_EVENTS
                        Number of events to run
  -l, --list            Print all the configurable components available in the framework and exit
  --gdb                 Attach gdb debugger
  --interactive-root    Run with ROOT in interactive mode (e.g. to see plots)
  --log-level {VERBOSE,DEBUG,INFO,WARNING,ERROR}
                        Set the log (output) level for python and the ApplicationMgr
  -i,--interactive      Start a Python command loop after reading the configuration files
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

* EDM4HEP

* fmt

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
- The `FilterPredicate`, for determining whether subsequent algorithms in the
  sequence are bypassed.

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

### Generating boilerplate with generateFunctional

`k4FWCore/helpers/generateFunctional` is a code generator that produces the C++
boilerplate for a new functional algorithm. The functional type (Consumer,
Producer, Transformer, MultiTransformer, FilterPredicate) is inferred
automatically from the number of inputs and outputs, or can be set explicitly.

Requirements: Python ≥ 3.9 and [jinja2](https://pypi.org/project/Jinja2/).
With [uv](https://github.com/astral-sh/uv) installed, dependencies are
resolved automatically via the PEP 723 script block.

```bash
# Producer with one output collection and one property
python3 k4FWCore/helpers/generateFunctional MyProducer \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    -p 'int:ExampleInt:3:An example integer property'

# Transformer (inferred from 1 input + 1 output)
python3 k4FWCore/helpers/generateFunctional MyTransformer \
    -i 'edm4hep::MCParticleCollection:InputCollection' \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    --private-properties

# MultiTransformer with type aliases
python3 k4FWCore/helpers/generateFunctional MyMulti \
    -i 'edm4hep::MCParticleCollection:Particles' \
       'edm4hep::TrackCollection:Tracks' \
    -o 'edm4hep::MCParticleCollection:NewParticles' \
       'podio::UserDataCollection<int>:Counter' \
    --type-aliases

# FilterPredicate (type must be specified explicitly)
python3 k4FWCore/helpers/generateFunctional MyFilter filter \
    -i 'edm4hep::MCParticleCollection:InputCollection'

# Consumer with runtime (variable-length) input collections
python3 k4FWCore/helpers/generateFunctional MyConsumer \
    -i 'edm4hep::MCParticleCollection:Inputs' \
    --runtime-inputs 'edm4hep::MCParticleCollection:Inputs:MCParticles0,MCParticles1'

# Also emit a CMakeLists.txt skeleton
python3 k4FWCore/helpers/generateFunctional MyProducer \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    --cmake
```

Run `python3 k4FWCore/helpers/generateFunctional --help` for the full list of options.
