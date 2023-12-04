<!--
Copyright (c) 2014-2023 Key4hep-Project.

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

The facilities to read and write EDM4hep (or in general event data models based
on podio) are provided by [`k4FWCore`](https://github.com/key4hep/k4FWCore).
This page will describe their usage, but not go into too much details of their
internals. This page also assumes a certain familiarity with Gaudi, i.e. most of
the snippets just show a minimal configuration part, and not a complete runnable
example.

## The `k4DataSvc`

Whenever you want to work with EDM4hep in the Gaudi based framework of Key4hep,
you will need to use the `k4DataSvc` as *EventDataSvc*. You can instantiate and
configure this service like the following

```python
from Gaudi.Configuration import *
from Configurables import k4DataSvc

evtSvc = k4DataSvc("EventDataSvc")
```

**It is important that the name is `EventDataSvc` in this case, as otherwise
this is an assumption from Gaudi.** Once you have the `k4DataSvc` instantiated,
you still have to make the `ApplicationMgr` aware of it, by making sure that the
`evtSvc` is in the list of the *external services* (`ExtSvc`):

```python
from Configurables import ApplicationMgr
ApplicationMgr(
    # other args
    ExtSvc = [evtSvc]
)
```

## Reading events

To read events you will need to use the `PodioInput` algorithm in addition to
the [`k4DataSvc`](#the-k4datasvc). Currently, you will need to pass the input
file to the `k4DataSvc` via the `input` option but pass the collections that you
want to read to the `PodioInput`. We are working on making this (discussion
happens in this [issue](https://github.com/key4hep/k4FWCore/issues/105)). The
parts of your options file related to reading EDM4hep files will look something
like this

```python
from Configurables import PodioInput, k4DataSvc

evtSvc = k4DataSvc("EventDataSvc")
evtSvc.input = "/path/to/your/input-file.root"

podioInput = PodioInput()
```

It is possible to change the input file from the command line via
```bash
k4run <your-options-file> --EventDataSvc.input=<input-file>
```

By default the `PodioInput` will read all collections that are available from
the input file. It is possible to limit the collections that should become
available via the `collections` option

```python
podioInput.collections = [
  # List of collection names that should be made available
]
```

## Writing events

To write events you will need to use the `PodioOutput` algorithm in addition to
the [`k4DataSvc`](#the-k4datasvc):

```python
from Configurables import PodioOutput

podioOutput = PodioOutput("PodioOutput", filename="my_output.root")
```

By default this will write the complete event contents to the output file.

### Writing only a subset of collections

Sometimes it is desirable to limit the collections to a subset of all available
collections from the EventStore. The `PodioOutput` allows to do this via the
`outputCommands` option that takes a list of `keep` or `drop` commands. Each
command must consist of the `keep`/`drop` command and a target. The target is a
collection name that may include the `?` or `*` wildcard patterns. This might
look like the following

```python
podioOutput.outputCommands = ["keep *"]
```

which will keep everything (the default), while

```python
podioOutput.outputCommands = ["drop *"]
```

will simply drop all collections and effectively write an empty file (apart from
some metadata). A common pattern is to `"drop *"` and then selectively adding
`keep` collections to keep, e.g. to only keep the highest level MC and reco
information:

```python
podioOutput.outputCommands = [
    "drop *",
    "keep MCParticlesSkimmed",
    "keep PandoraPFOs",
    "keep RecoMCTruthLink",
]
```
