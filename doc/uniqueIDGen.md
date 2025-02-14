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

# Seeding algorithm PRNG

Correctly seeding pseudo-random number generators (PRNG) is important to ensuring reproducible and good-quality randomness. A common pattern is to reinitialize each algorithm’s PRNG with a unique seed for each processed event.

## UniqueIDGenSvc service

The `UniqueIDGenSvc` Gaudi service can generate unique `size_t` values that can be used as PRNG seeds, based on an event number, run number, and algorithm.

The service can be included in a steering file by:

```python
from Configurables import UniqueIDGenSvc
uidgen_svc = UniqueIDGenSvc()
```

The service can be configured with a central seed from which the unique values will be derived. This seed can be configured with the `Seed` property:

::::{tab-set}
:::{tab-item} Python
```python
uidgen_svc.Seed = 987654321
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --UniqueIDGenSvc.Seed 987654321
```
:::
::::

## Using UniqueIDGenSvc in algorithms

To use the `UniqueIDGenSvc` in a Gaudi algorithm first include the header file and declare a member variable:

```cpp
#include <k4Interface/IUniqueIDGenSvc.h>

SmartIF<IUniqueIDGenSvc> m_service;
```

Then, initialize the service in:

```cpp
StatusCode ExampleAlgorithm::initialize() {
  m_service = service("UniqueIDGenSvc");
```

Then, use the service during execution:

```cpp
StatusCode ExampleAlgorithm::execute(const EventContext&) const {
  m_service->getUniqueID(1, 2, name());
}
```

## Seeding functional algorithm example

In the EDM4hep data model, the event number and run number can be obtained from an `EventHeaderCollection`.
The following is an example of a functional algorithm using `EventHeaderCollection` and `UniqueIDGenSvc` service to seed a PRNG.

To begin, some boilerplate code is needed to declare the `EventHeaderCollection` as an algorithm input. In this example, the algorithm will also generate and produce a `podio::UserDataCollection<double>` containing random numbers generated during its execution.
The name of input `EventHeaderCollection` object can be customized using the `EventHeaderCollection` property, with `EventHeader` set as the default name. Similarly, the name of the output `podio::UserDataCollection<double>`object can be adjusted via the `OutputCollection` property, which defaults to `RandomNumbers`.


```cpp
class ExampleRNGSeedingAlg final
    : public k4FWCore::Transformer<podio::UserDataCollection<double>(const edm4hep::EventHeaderCollection&)> {
public:
  ExampleRNGSeedingAlg(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("EventHeaderCollection", {"EventHeader"})},
                    {KeyValues("OutputCollection", {"RandomNumbers"})}) {}
```

Then, declare a member variable for the service interface and locate the service during algorithm initialization:

```cpp
private:
  SmartIF<IUniqueIDGenSvc> m_uniqueIDSvc{nullptr};

public:
  StatusCode initialize() final {
    m_uniqueIDSvc = service("UniqueIDGenSvc");
    if (!m_uniqueIDSvc) {
      error() << "Unable to locate the UniqueIDGenSvc" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
```

During algorithm execution the `EventHeaderCollection` can be used to obtain a unique value from the `UniqueIDSvc`, then the value can be used as a seed for PRNG. In this example the PRNG is then used to generate a random value and push it to an output collection.

```cpp
public:
  podio::UserDataCollection<double> operator()(const edm4hep::EventHeaderCollection& evtHeader) const final {
    const auto evt = evtHeader[0];

    // obtain  unique value
    auto uid = m_uniqueIDSvc->getUniqueID(evt.getEventNumber(), evt.getRunNumber(), name());

    // seed TRandom3 or some other PRNG of your choice
    auto prng = TRandom3(uid);

    auto coll = podio::UserDataCollection<double>();
    coll.push_back(a);
    return coll;
  }
```

A complete code example can be found [here](https://github.com/key4hep/k4FWCore/blob/main/test/k4FWCoreTest/src/components/ExampleRNGSeedingAlg.cpp).
