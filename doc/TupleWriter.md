<!--
Copyright (c) 2014-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->
# TupleWriter

`TupleWriter` is a Gaudi functional template that lets you map typed values to
named fields and write them per event into:
- ROOT `TTree` (default)
- ROOT `RNTuple` (when `RNTuple=True`)

You specify the possible output field types via a `std::tuple<...>` template
(where typically the output types go for other functional algorithms) and fill a
field map keyed by strings. Each event typically fills one entry per configured
output tree/tuple. Declare your writer by inheriting from `TupleWriter` and
specifying the output types and input collections in the signature:

```cpp
#include "k4FWCore/TupleWriter.h"
#include "edm4hep/MCParticleCollection.h"

struct MyTupleWriter final
  : k4FWCore::TupleWriter<std::tuple<int, double, std::string>(const edm4hep::MCParticleCollection&)> {
  MyTupleWriter(const std::string& name, ISvcLocator* svcLoc)
    : TupleWriter(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  void operator()(const edm4hep::MCParticleCollection& input) const override {
    auto& map = getNextTupleMap();
    map["PDG"] = input[0].getPDG();
    map["Mass"] = input[0].getMass();
    map["Note"] = std::string("hello");
    // Optional: map.fill(); // manually fill now (see below)
  }
};
```

In this case the types of the branches of the resulting TTree or fields of the
resulting RNTuple will be `int`, `double` or `std::string`. Fields behave like
assigning to a `std::variant<Out...>`, where the usual `variant` rules apply
(e.g. `3.14` is `double`; use `3.14f` for `float`). If transforming to one of
the output types is not possible (for example, the output is only `int` but we
have assign a floating point value), then compilation fails. Complex types are
supported if ROOT can write them (e.g. `edm4hep::Vector3d`).

## Multiple Outputs (Trees/Tuples)

You can write to multiple TTrees/RNTuples in one file by configuring multiple
names. `getNextTupleMap()` returns the field map for the next output, in order.

Properties:
- `Names`: list of output names, e.g. `["Main", "Extra"]`
- `Descriptions`: list of descriptions (same length as `Names`)

Example within `operator()`:

```cpp
auto& mainMap = getNextTupleMap();
mainMap["PDG"] = input[0].getPDG();
mainMap["Text"] = std::string("main");

auto& extraMap = getNextTupleMap();
extraMap["Charge"] = input[0].getCharge();
extraMap["Text"] = std::string("extra");
```

The component will automatically call `fill()` once per event for any map not
filled manually. The order of calls to `getNextTupleMap()` must match the
configured outputs. If you want to fill more than once per event (e.g. once per
particle), you can call `fill()` on the returned map. When `fill()` is called
manually for a map in an event, the automatic fill at the end of `operator()`
for that map is skipped.

## Configuration Properties

- `OutputFile` (string): name of the ROOT output file, default `tree.root`
- `Names` (vector<string>): names of TTrees/RNTuples, default `["tree"]`
- `Descriptions` (vector<string>): descriptions (TTree title), must be same
  length as `Names` (ignored with RNTuples)
- `RNTuple` (bool): write RNTuples instead of TTrees, default `False`

Example Python configuration (Gaudi):

```python
from Configurables import MyTupleWriter
writer = MyTupleWriter(
    "TupleWriter",
    OutputFile="output.root",
    Names=["Main", "Extra"],
    Descriptions=["Main tree", "Extra tree"],
    RNTuple=False,  # or True
    InputCollection="MCParticles",
)
```

## Example Component

Find `ExampleTupleWriter.cpp` for a working example in this repository that
demonstrates multiple outputs and manual fills.
