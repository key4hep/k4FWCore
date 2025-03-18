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

# This is an example creating EventHeader created in GaudiHive

from Gaudi.Configuration import *

from Configurables import EventHeaderCreator
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


eventHeaderCreator = EventHeaderCreator(
    "eventHeaderCreator", runNumber=42, eventNumberOffset=42, OutputLevel=DEBUG
)

svc = IOSvc("IOSvc")
svc.Output = "eventHeaderConcurrent.root"

ApplicationMgr(
    TopAlg=[eventHeaderCreator],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)
