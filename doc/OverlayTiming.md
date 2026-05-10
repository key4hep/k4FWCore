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

# OverlayTiming

`OverlayTiming` is a Gaudi algorithm that overlays background events on top of signal events, typically used to simulate pile-up or beam-induced backgrounds. Background events are read from one or more sets of input files independently of the main event loop.

It uses [`UniqueIDGenSvc`](uniqueIDGen.md) to seed the internal random number generator.

## Collections

- `MCParticleCollection` from signal and background are merged into a single output collection. Background particles are flagged with `isOverlay = true`.
- `SimTrackerHit` collections are cropped to a configurable time window and overlaid. Background hits are flagged with `isOverlay = true`.
- `SimCalorimeterHit` collections are overlaid by merging hits that share the same cell ID. Their `CaloHitContribution` entries are filtered by the time window.

## Properties

| Property | Default | Description |
|----------|---------|-------------|
| `BackgroundFileNames` | `[]` | List of groups of background input files, one group per overlay stream |
| `NumberBackground` | `[]` | Number of background events to overlay per stream (fixed or Poisson mean) |
| `Poisson_random_NOverlay` | `[]` | If true, draw the number of events from a Poisson distribution with mean `NumberBackground` |
| `NBunchtrain` | `1` | Number of bunch crossings in the bunch train |
| `PhysicsBX` | `1` | Position of the physics event within the bunch train |
| `RandomBx` | `false` | If true, place the physics event at a random bunch crossing (overrides `PhysicsBX`) |
| `Delta_t` | `0.5` | Time between consecutive bunch crossings (ns) |
| `TimeWindows` | `{}` | Map from collection name to `[t_min, t_max]` (ns) defining the acceptance window. Required for every `SimTrackerHit` and `SimCalorimeterHit` collection. |
| `BackgroundMCParticleCollectionName` | `"MCParticle"` | Name of the MCParticle collection in the background files |
| `AllowReusingBackgroundFiles` | `false` | If true, wrap around the background file when events are exhausted |
| `CopyCellIDMetadata` | `false` | Copy cell ID encoding metadata from input to output collections |
| `StartBackgroundEventIndex` | `-1` | Index of the background event to start from (`-1` means start from the beginning) |

## Usage example

The following example reads signal events from `signal.root` and overlays background events from two separate background files onto the `MCParticles`, `VertexBarrelCollection`, and `HCalRingCollection` collections. Two independent overlay streams are configured: one with a fixed number of overlaid events and one drawing from a Poisson distribution.

```python
from Gaudi.Configuration import INFO
from Configurables import EventDataSvc, OverlayTiming, UniqueIDGenSvc
from k4FWCore import ApplicationMgr, IOSvc

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "signal.root"
iosvc.Output = "signal_with_background.root"

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles"
overlay.SimTrackerHits = ["VertexBarrelCollection"]
overlay.SimCalorimeterHits = ["HCalRingCollection"]
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = ["OverlayVertexBarrelCollection"]
overlay.OutputSimCalorimeterHits = ["OverlayHCalRingCollection"]
overlay.OutputCaloHitContributions = ["OverlayCaloHitContributions"]
overlay.BackgroundMCParticleCollectionName = "MCParticles"
# Two independent streams of background events
overlay.BackgroundFileNames = [
    ["background_beam1.root", "background_beam2.root"],
    ["background_gamma.root"],
]
overlay.NumberBackground = [10, 1]
overlay.Poisson_random_NOverlay = [True, False]
overlay.NBunchtrain = 5
overlay.PhysicsBX = 1
overlay.Delta_t = 0.5  # ns
overlay.TimeWindows = {
    "VertexBarrelCollection": [-0.5, 5.0],
    "HCalRingCollection": [-5.0, 20.0],
}

ApplicationMgr(
    TopAlg=[overlay],
    EvtSel="NONE",
    EvtMax=100,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
```
