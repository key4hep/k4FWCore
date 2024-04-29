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

# This is an example reading from a file and using a consumer with several inputs
# to check that the contents of the file are the expected ones

from Gaudi.Configuration import INFO, WARNING
from Configurables import ExampleFunctionalTransformer, ExampleFunctionalConsumer
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard, AvalancheSchedulerSvc
from k4FWCore import ApplicationMgr, IOSvc

evtslots = 6
threads = 6

whiteboard = HiveWhiteBoard(
    "EventDataSvc",
    EventSlots=evtslots,
    ForceLeaves=True,
)

slimeventloopmgr = HiveSlimEventLoopMgr(
    "HiveSlimEventLoopMgr", SchedulerName="AvalancheSchedulerSvc", OutputLevel=WARNING
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, ShowDataFlow=True, OutputLevel=WARNING)

svc = IOSvc("IOSvc")
svc.input = "output_k4test_exampledata_producer_multiple.root"
svc.output = "functional_transformerMT.root"

consumer = ExampleFunctionalConsumer(
    "Consumer1",
    InputCollection=["MCParticles1"],
    Offset=0,
)

transformer = ExampleFunctionalTransformer(
    "Transformer", InputCollection=["MCParticles1"], OutputCollection=["NewMCParticles"]
)

mgr = ApplicationMgr(
    TopAlg=[consumer, transformer],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)
