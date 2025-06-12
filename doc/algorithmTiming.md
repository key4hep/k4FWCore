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

# Getting algorithm run time information

It might be interesting to know in which algorithms the most time is spent in a
workflow. This can be achieved with builtin Gaudi services, and auditors,
specifically using either the `TimelineSvc` and/or the `AlgTimingAuditor`
depending on the desired level of detail.

## `AlgTimingAuditor`

The `AlgTimingAuditor` in combination with the `AuditorSvc` allow to easily get
an overview over the execution time of a workflow and in which algorithms,
services or tools time is spent. In order to configure it, you need to simply do
the following

```python
from Configurables import AuditorSvc, AlgTimingAuditor, EventDataSvc
from k4FWCore import ApplicationMgr

# List of your algorithms that you want to run
algList = []

# setup the AuditorSvc and add the AlgTimingAuditor to it
auditorSvc = AuditorSvc
auditorSvc.Auditors = [AlgTimingAuditor()]

appMgr = ApplicationMgr(
    TopAlg=algList,
    EvtSel="NONE",
    ExtSvc=[EventDataSvc("EventDataSvc", auditorSvc)]
)
```

This will result in an output similar to this one
```console
AlgTimingAuditor     INFO -------------------------------------------------------------------
AlgTimingAuditor     INFO Algorithm                      | exec (ms) |   count   | total (s)
AlgTimingAuditor     INFO -------------------------------------------------------------------
AlgTimingAuditor     INFO EVENT LOOP                     |      7210 |         3 |     21.63
AlgTimingAuditor     INFO -------------------------------------------------------------------
```

### Per Algorithm timing
In order to enable a more detailed output and auditing, it is necessary to
enable them via

```python
appMgr.AuditAlgorithms = True
appMgr.AuditTools = True
appMgr.AuditServices = True
```

This will result in output like
```console
AlgTimingAuditor     INFO -------------------------------------------------------------------
AlgTimingAuditor     INFO Algorithm                      | exec (ms) |   count   | total (s)
AlgTimingAuditor     INFO -------------------------------------------------------------------
AlgTimingAuditor     INFO                                |         0 |         0 |         0
AlgTimingAuditor     INFO IncidentSvc                    |         0 |         0 |         0
AlgTimingAuditor     INFO EventPersistencySvc            |         0 |         0 |         0
AlgTimingAuditor     INFO                                |         0 |         0 |         0
AlgTimingAuditor     INFO                                |         0 |         0 |         0
AlgTimingAuditor     INFO                                |         0 |         0 |         0
AlgTimingAuditor     INFO Gaudi::Utils::SignalMonitorSvc |         0 |         0 |         0
AlgTimingAuditor     INFO                                |         0 |         0 |         0
AlgTimingAuditor     INFO TimelineSvc                    |         0 |         0 |         0
AlgTimingAuditor     INFO PodioInput                     |     67.34 |         3 |     0.202
AlgTimingAuditor     INFO AlgExecStateSvc                |         0 |         0 |         0
AlgTimingAuditor     INFO MyAIDAProcessor                |     56.44 |         3 |    0.1693
AlgTimingAuditor     INFO  RndmGenSvc                    |         0 |         0 |         0
AlgTimingAuditor     INFO   RndmGenSvc.Engine            |         0 |         0 |         0
AlgTimingAuditor     INFO ToolSvc                        |         0 |         0 |         0
AlgTimingAuditor     INFO ToolSvc.InputConversion        |         0 |         0 |         0
AlgTimingAuditor     INFO MyStatusmonitor                |   0.02489 |         3 | 7.467e-05
AlgTimingAuditor     INFO MySplitCollectionByLayer       |   0.04234 |         3 |  0.000127
AlgTimingAuditor     INFO VXDPlanarDigiProcessor_CMOSVXD |    0.5061 |         3 |  0.001518
AlgTimingAuditor     INFO SITPlanarDigiProcessor         |    0.4672 |         3 |  0.001402
AlgTimingAuditor     INFO EDM4hepOutput                  |     97.03 |         3 |    0.2911
AlgTimingAuditor     INFO HistogramDataSvc               |         0 |         0 |         0
AlgTimingAuditor     INFO  HistogramPersistencySvc       |         0 |         0 |         0
AlgTimingAuditor     INFO -------------------------------------------------------------------
```

## `TimelineSvc`

Using the `TimelineSvc` it is possible to get the exact start and end times for
each algorithm and event. It's output can be used for very detailed analysis
later. It can be configured as

```python
from Configurables import TimelineSvc, EventDataSvc
from k4FWCore import ApplicationMgr

# List of your algorithms that you want to run
algList = []

timelineSvc = TimelineSvc(RecordTimeline=True, DumpTimeline=True, TimelineFile="timeline.csv")

appMgr = ApplicationMgr(
    TopAlg=algList,
    EvtSel="NONE",
    ExtSvc=[EventDataSvc("EventDataSvc", timelineSvc)]
)
```

This will create `timelines.csv` file that looks similar to
```csv
#start end algorithm thread slot event
1745848335520532076 1745848335574872001 PodioInput 135806878369600 0 0
1745848335574879058 1745848335631430347 MyAIDAProcessor 135806878369600 0 0
1745848335631436925 1745848335631466797 MyStatusmonitor 135806878369600 0 0
1745848335631467617 1745848335631525405 MySplitCollectionByLayer 135806878369600 0 0
1745848335631526275 1745848335632537227 VXDPlanarDigiProcessor_CMOSVXD5 135806878369600 0 0
1745848335632538493 1745848335633010729 SITPlanarDigiProcessor 135806878369600 0 0
1745848335633012309 1745848335633141450 FTDPixelPlanarDigiProcessor 135806878369600 0 0
1745848335633142527 1745848335633573502 FTDStripPlanarDigiProcessor 135806878369600 0 0
1745848335633574995 1745848335634521760 FTDDDSpacePointBuilder 135806878369600 0 0
```

each line has
- the start time of the algorithm in nanoseconds
- the end time of the algorithm in nanoseconds
- the algorithm name
- the thread-id
- the (thread) slot in which the event was processed in Gaudi
- the event number


```{note}
The event number is a sequential number inside Gaudi and does not necessarily
correspond with the event number that can be obtained from the `EventHeader`
```

### Visualization using `hivetimeline.py`

The `hivetimeline.py` tool that comes with Gaudi can be used to visualize the
output of the `TimelineSvc`. It has some limitations, e.g. the number of
algorithms it supports for visualization, but it might be useful to have a first
quick look.

