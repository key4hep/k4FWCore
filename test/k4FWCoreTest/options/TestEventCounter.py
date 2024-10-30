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

# This is an example of counting events with EventCounter algorithm.
# During execution the EventCounter algorithms will printout current event number depending on the frequency.
# The summary about total number of events passed through that algorithms is handled by MessageSvcSink

from Gaudi.Configuration import INFO, WARNING
from Configurables import EventCounter
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard, AvalancheSchedulerSvc
from k4FWCore import ApplicationMgr

evtslots = 4
threads = 4

whiteboard = HiveWhiteBoard(
    "EventDataSvc",
    EventSlots=evtslots,
    ForceLeaves=True,
)

slimeventloopmgr = HiveSlimEventLoopMgr(
    "HiveSlimEventLoopMgr",
    SchedulerName="AvalancheSchedulerSvc",
    Warnings=False,
    OutputLevel=WARNING,
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, ShowDataFlow=True, OutputLevel=WARNING)

counter_1 = EventCounter("SilentEventCounter", Frequency=0, OutputLevel=INFO)
counter_2 = EventCounter("EventCounter", Frequency=2, OutputLevel=INFO)


mgr = ApplicationMgr(
    TopAlg=[counter_1, counter_2],
    EvtSel="NONE",
    EvtMax=4,
    # Add "MessageSvcSink" for a table at the end with a statistical summary of the counter data
    # ExtSvc=[whiteboard, "Gaudi::Monitoring::MessageSvcSink"],
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    OutputLevel=INFO,
)
