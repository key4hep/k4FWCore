#
# Copyright (c) 2014-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This is an example using two producers that create histograms and persist them to a ROOT file

from Gaudi.Configuration import INFO, WARNING
from Configurables import ExampleFunctionalProducer, ExampleFunctionalTransformerHist
from k4FWCore import ApplicationMgr
from Configurables import RootHistSvc
from Configurables import Gaudi__Histograming__Sink__Root as RootHistoSink
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc

# Multithreaded also works fine with histograms
multithreaded = False
threads = 2
slots = 3
if multithreaded:
    whiteboard = HiveWhiteBoard(
        "EventDataSvc",
        EventSlots=slots,
        ForceLeaves=True,
    )
    slimeventloopmgr = HiveSlimEventLoopMgr(
        "HiveSlimEventLoopMgr",
        SchedulerName="AvalancheSchedulerSvc",
        OutputLevel=WARNING,
    )

    scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=INFO)
    scheduler.ShowDataDependencies = True
    scheduler.ShowDataFlow = True
    scheduler.ShowControlFlow = True


producer1 = ExampleFunctionalProducer(
    "ExampleFunctionalProducer1", OutputCollection=["MCParticles1"]
)
producer2 = ExampleFunctionalProducer(
    "ExampleFunctionalProducer2", OutputCollection=["MCParticles2"]
)

transformer1 = ExampleFunctionalTransformerHist(
    "ExampleFunctionalTransformerHist1",
    InputCollection=["MCParticles1"],
    OutputCollection=["dummy1"],
)

transformer2 = ExampleFunctionalTransformerHist(
    "ExampleFunctionalTransformerHist2",
    InputCollection=["MCParticles2"],
    OutputCollection=["dummy2"],
    FirstParticle=False,
)

hps = RootHistSvc("HistogramPersistencySvc")
root_hist_svc = RootHistoSink("RootHistoSink")
root_hist_svc.FileName = "functional_transformer_hist.root"

try:
    transformer1.CustomHistogram_Title = "Custom Title"
    # Bins can be defined here
    transformer1.CustomHistogram_Axis0 = (10, -5.0, 10.0, "X")
# Before Gaudi v39 there isn't a way to set the bins from python
except Exception:
    pass


app = ApplicationMgr(
    TopAlg=[producer1, producer2, transformer1, transformer2],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[root_hist_svc] + ([whiteboard] if multithreaded else []),
    OutputLevel=INFO,
    HistogramPersistency="ROOT",
    EventLoop="EventLoopMgr" if not multithreaded else slimeventloopmgr,
)
