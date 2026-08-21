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

# Runs the OverlayTiming algorithm with intra-event multithreading (multiple
# event slots processed concurrently by the Avalanche scheduler). This exercises
# the const operator() from several worker threads sharing a single background
# EventHolder, checking that its worker-thread ROOT I/O serialization is safe.

from Gaudi.Configuration import INFO, WARNING
from Configurables import EventDataSvc, EventHeaderCreator, OverlayTiming, UniqueIDGenSvc
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard, AvalancheSchedulerSvc
from k4FWCore import ApplicationMgr, IOSvc

evtslots = 6
threads = 6

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

whiteboard = HiveWhiteBoard(
    "EventDataSvc",
    EventSlots=evtslots,
    ForceLeaves=True,
)
slimeventloopmgr = HiveSlimEventLoopMgr(
    "HiveSlimEventLoopMgr",
    SchedulerName="AvalancheSchedulerSvc",
    OutputLevel=WARNING,
)
scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

iosvc = IOSvc("IOSvc")
iosvc.Input = "functional_producer_multiple.root"
iosvc.Output = "overlay_mt_output.root"

header = EventHeaderCreator("EventHeaderCreator")

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles1"
overlay.SimTrackerHits = ["SimTrackerHits"]
overlay.SimCalorimeterHits = []
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = ["OverlaySimTrackerHits"]
overlay.OutputSimCalorimeterHits = []
overlay.OutputCaloHitContributions = []
overlay.BackgroundMCParticleCollectionName = "MCParticles1"
overlay.BackgroundFileNames = [["functional_producer_multiple.root"]]
overlay.NumberBackground = [1]
overlay.Poisson_random_NOverlay = [False]
overlay.NBunchtrain = 3
overlay.TimeWindows = {"SimTrackerHits": [-10000, 10000]}
overlay.AllowReusingBackgroundFiles = True

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[whiteboard, uid_svc],
    EventLoop=slimeventloopmgr,
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)
