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

# Monitoring Functional algorithms

It might be necessary to monitor an algorithm, e.g. by keeping track of how many
times different branches in the algorithm logic are taken. Since Functional
algorithms run potentially multithreaded, the usual approach of simply using
mutable counters or histograms for this is not easily possible. Gaudi provides
some threadsafe tools to make monitoring Functional algorithms more easily
possible. These tools generally take care of updating counters and histograms in a
threadsafe way, as well as collecting all the data from potentially multiple
instances running on different threads at the end of the execution. We describe
how to use these tools here.

In Gaudi all of these tools live in the `Gaudi::Accumulators` namespace and we
refer to the [documentation of
that](https://gaudi.web.cern.ch/doxygen/v40r0/da/dd5/namespace_gaudi_1_1_accumulators.html)
for more technical details. We will focus on the use of some of these tools.

## Using histograms
Histograms come in two flavors:
- **Static histograms** are fully defined at compile time, i.e. axis ranges,
  binning as well as axis titles cannot be altered at runtime.
- **Non-static (or configurable) histograms** are somewhat dynamically
  configurable at runtime. Effectively only their dimension is fixed at compile
  time.

The main difference between the two flavors is how they are declared in a
Functional algorithm and obviously the configurability at runtime. Both flavors
are filled the same way.

Histograms also exist as generic versions and as ROOT backed versions. The main
difference is that the ROOT backed version will obviously produce ROOTs
`TH[1-3][I,D,F]`s (and are hence also limited to at most 3 dimensions), whereas
the generic versions can have arbitrary dimensions and will be serialized to
JSON.

### Declaring a histogram

In the following we declare a static (`StaticRootHistogram`) and a dynamic
(`RootHistogram`) ROOT histogram with different dimensions. The unspecified template
parameters in this case default to full atomicity and double as arithmetic type.

::::{tab-set}
:::{tab-item} Static histogram
```cpp
#include <Gaudi/Accumulators/RootHistogram.h>

struct MyAlgorithm : public /* whatever Functional you want */ {

  // Define as a member of your (Functional) algorithm
  mutable Gaudi::Accumulators::StaticRootHistogram<1> m_hist1d{
      this, "MyHistogram1D", "A 1D histogram", {100, 0., 100.f}};

  // For two dimensions simply specify two axis in the constructor
  mutable Gaudi::Accumulators::StaticRootHistogram<2> m_hist2d{
      this, "MyHistogram2D", "A 2D histogram", {100, 0.0, 100.0}, {25, 0.0, 1.0, "axis title"}};
};
```
:::
:::{tab-item} Configurable histogram
```cpp
#include <Gaudi/Accumulators/RootHistogram.h>

#include <functional>

struct MyAlgorithm : public /* whatever Functional you want */ {

  // Define as a member of your (Functional) algorithm
  // NOTE: it is possible to define a default for the axis, but not strictly necessary
  // NOTE: The name we give this histogram is also how we are going to customize from python later
  mutable Gaudi::Accumulators::RootHistogram<1> m_hist1d{
      this, "Hist1D", "A 1D histogram with configurable axis", {100, 0., 100.f}};

  // We can also just define the dimensions and the title
  mutable Gaudi::Accumulators::RootHistogram<2> m_hist2d{this, "Hist2D", "A 2D histogram"};

  // NOTE: For configurable histograms you have to register this empty callback
  void registerCallBack(Gaudi::StateMachine::Transition, std::function<void()>){};
};
```
:::

### Filling a histogram

Filling a histogram is done by using the `operator[]` of the histogram classes
and is the same whether the histograms are configurable or
static.

```cpp
auto operator()(/* However your signature looks like */) {
  // assume that we have computed value somehow and now want to fill the histogram with it
  ++m_hist1d[value];

  // For multi-dimensional histograms we have to construct a proxy index on the fly
  ++m_hist2d[{valueX, valueY}];
}
```

By default the histograms are defined with *full atomicity*, i.e. changing a
counter is a potentially costly operation and we usually don't want to pay the
price for this in tight loops, where we are certain that we are in any case on a
single thread only. For these cases we use the `buffer` method to get a
non-atomic version that can be updated quickly and which will automatically
update its parent once it goes out of scope, e.g.

```cpp
auto operator()(/* However your signature looks like */) {
  { // create a scope to limit the lifetime of the buffer
    auto histBuffer = m_hist1d.buffer();
    for (int i = 0; i < 1000; ++i) {
      ++histBuffer[i];
    }
  } // Here histBuffer goes out of scope and its destructor will update m_hist1d in a threadsafe way
}
```

### Configuring the necessary services

In order to actually store the histograms that are filled it is necessary to
configure a Gaudi service to persist them, for ROOT histograms this can be done
as follows

```python
from Configurables import Gaudi__Histograming__Sink__Root as RootHistoSink

histoSinkSvc = RootHistoSink("RootHistoSink")
histoSinkSvc.FileName = "monitoring_histograms.root"
```

**Don't forget to add this service to the `ExtSvc` list in the `ApplicationMgr`**

### Customizing a histogram

Configurable histograms need to be created in `initialize` as follows in order to
pick up the configuration from python:
```cpp
StatusCode MyAlgorithm::initialize() {
  m_hist1d.createHistogram(*this);
  m_hist2d.createHistogram(*this);
}
```

To change properties on the python side you first need to instantiate your
algorithm and you then have access to the histograms of this instance. Note how
the name of the histogram that was given on the c++ side is used as part of the
property name on the python side:

```python
algo = MyAlgorithm("MyAlgorithm")
algo.Hist2D_Title = "Title changed from python"
algo.Hist1D_Axis0 = (20, -1.0, 1.0, "X-axis")
```
