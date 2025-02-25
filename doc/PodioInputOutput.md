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
# Reading and writing EDM4hep files in Gaudi

The facilities to read and write EDM4hep (or in general event data models based on podio) are provided by [k4FWCore](https://github.com/key4hep/k4FWCore). This page will describe their usage, but not go into too much details of their internals. This page also assumes a certain familiarity with Gaudi, i.e. most of the snippets just show a minimal configuration part, and not a complete runnable example.

## Accessing event data

`IOSvc` is an external Gaudi service for reading and writing EDM4hep files. The service should be imported from `k4FWCore` and named "IOSvc" as other components may look for it under this name.

```python
from k4FWCore import IOSvc

io_svc = IOSvc("IOSvc") # or just IOSvc() as "IOSvc" name is used by default
```

After instantiation the service should be register as an external service in the `ApplicationMgr`. Similarly, it's important to import the `ApplicationMgr` from `k4FWCore`:

```python
from k4FWCore import ApplicationMgr

ApplicationMgr(
    # other args
    ExtSvc=[
        io_svc,
        # other services
    ]
)
```

### Reading events

The `IOSvc` supports reading EDM4hep ROOT files. Both files written with the ROOT TTree or RNTuple backend are supported with the backend inferred automatically from the files themselves.

The `Input` property can be used to specify the input. The `IOSvc` will not read any files unless the `Input` property is specified.

::::{tab-set}
:::{tab-item} Python
```python
io_svc.Input = "input.root"
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.Input input.root
```
:::
::::

:::{note}
The value assigned to the `Input` will be processed as is, in particular without regular expression or glob expansion.
:::

A list of filenames can be given in order to specify multiple input files:

::::{tab-set}
:::{tab-item} Python
```python
io_svc.Input = ["input.root", "another_input.root", ]
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.Input input.root another_input.root
```
:::
::::


During processing, for each event in the Gaudi event loop the `IOSvc` will read a frame from the input and populate the Gaudi Transient Event Store (TES) with the collections stored in that frame.

The `FirstEventEntry` property of `IOSvc` can be used to start processing from a given frame instead of from the first frame in the input:

::::{tab-set}
:::{tab-item} Python
```python
io_svc.FirstEventEntry = 7 # default 0
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.FirstEventEntry 7
```
:::
::::

A list of collection names can be assigned to the `CollectionNames` property of `IOSvc` to limit the number of collections that will be populated. Without specifying the `CollectionNames` all present collections will be read and put into TES.

::::{tab-set}
:::{tab-item} Python
```python
io_svc.CollectionNames = ["MCParticles", "SimTrackerHits"]
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.CollectionNames "MCParticles" "SimTrackerHits"
```
:::
::::

### Writing events

The `IOSvc` supports writing EDM4hep to the ROOT output. The `Output` property can be used to specify the output. The `IOSvc` will not write any files unless the `Output` property is specified.

::::{tab-set}
:::{tab-item} Python
```python
io_svc.Output = "output.root"
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.Output output.root
```
:::
::::

:::{note}
Unlike the `Input`, the `Output` property should be a single string even when writing multiple files is expected. When the size limit for an output file is reached, the system will automatically open a new file and start writing to it.
:::

The writing backend can be specified with the `OutputType` property of `IOSvc`. The allowed values are `"ROOT"` for TTree-based output or `"RNTuple"` for RNTuple-based output. By default the `"ROOT"` backend is used.

::::{tab-set}
:::{tab-item} Python
```python
io_svc.OutputType = "RNTuple"
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.OutputType "RNTuple"
```
:::
::::

During processing, at the end of each event from the Gaudi event loop the `IOSvc` will write a frame with the collection present in TES. By default all the collections will be written. The `outputCommands` property of `IOSvc` can be used to specify commands to select which collections should be written. For example, the following commands will skip writing all the collections except for the collections named `MCParticles1`, `MCParticles2` and `SimTrackerHits`:

::::{tab-set}
:::{tab-item} Python
```python
io_svc.outputCommands = [
    "drop *",
    "keep MCParticles1",
    "keep MCParticles2",
    "keep SimTrackerHits",
]
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --IOSvc.outputCommands \
    "drop *" \
    "keep MCParticles1" \
    "keep MCParticles2" \
    "keep SimTrackerHits"
```
:::
::::

## Accessing metadata

The k4FWCore provides the `MetadataSvc` that allows accessing user metadata in PODIO-based data-models. There is no need to instantiate the `MetadataSvc` explicitly when using `IOSvc` as `IOSvc` can instantiate it on its own if needed.

When both the `Input` and `Output` properties of `IOSvc` are defined, all the metadata originally present in the input will be propagated to the output, possibly adding also any user metadata created during processing.

Unlike event data, metadata is not exposed to users through the Gaudi TES and cannot be accessed directly by algorithms in the same way. Instead, handling metadata is encapsulated within the algorithm implementation itself. For more details on how this is managed, refer to the developer documentation.


## Migrating from the legacy `k4DataSvc`

Migrating from the legacy `k4DataSvc` or `PodioDataSvc` is rather straightforward. On a steering file level the `PodioDataSvc` should be replaced with the `IOSvc`, while the `PodioInput` and `PodioOutput` algorithms should be removed. For example:

```diff
-from Configurables import k4DataSvc
-from Configurables import PodioInput
-from Configurables import PodioOutput
+from k4FWCore import IOSvc
from k4FWCore import ApplicationMgr
from Configurables import SelectorAlg

-podioevent = k4DataSvc("EventDataSvc")
-podioevent.input = "example_input.root"
+io_svc = IOSvc("IOSvc")
+io_svc.Input= "example_output.root"

-inp = PodioInput()
-inp.collections = ["MCParticles", "SimTrackerHits", "TrackerHits", "Tracks"]
+io_svc.CollectionNames = ["MCParticles", "SimTrackerHits"]

alg = SelectorAlg(
    "Selector",
    InputParticles="MCParticles",
    InputHits="SimTrackerHits",
    Output="SelectedParticles",
)

-oup = PodioOutput()
-oup.filename = "example_output.root"
-oup.outputCommands = ["drop MCParticles"]
+io_svc.Output = "example_output.root"
+io_svc.outputCommands = ["drop MCParticles"]


ApplicationMgr(
-    TopAlg=[inp, alg,oup],
+    TopAlg=[alg],
    EvtSel="NONE",
-    ExtSvc=[podioevent],
+    ExtSvc=[io_svc],
)
```

Both functional algorithms and classic algorithms are compatible with either `IOSvc` or `PodioDataSvc`.
