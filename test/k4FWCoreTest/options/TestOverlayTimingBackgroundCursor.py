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

# Tests where OverlayTiming reads its background events from, using
# output_k4test_exampledata.root (100 events) as both signal and background.
# The first particle of event n in that file has a momentum of n along x, so
# every overlaid background particle tells which event it was read from.
#
# Both background groups start at event 95. Group 0 overlays 2 events per
# signal event and group 1 one, so group 0 runs out of events in the third
# signal event: with AllowReusingBackgroundFiles it starts over from the first
# event, otherwise the job has to fail.

from Gaudi.Configuration import INFO
from Configurables import (
    EventDataSvc,
    EventHeaderCreator,
    OverlayTiming,
    UniqueIDGenSvc,
)
from k4FWCore import ApplicationMgr, IOSvc

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "output_k4test_exampledata.root"
iosvc.Output = "overlay_start_index.root"

header = EventHeaderCreator("EventHeaderCreator")

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles"
overlay.SimTrackerHits = ["SimTrackerHits"]
overlay.SimCalorimeterHits = []
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = ["OverlaySimTrackerHits"]
overlay.OutputSimCalorimeterHits = []
overlay.OutputCaloHitContributions = []
overlay.BackgroundMCParticleCollectionName = "MCParticles"
overlay.BackgroundFileNames = [
    ["output_k4test_exampledata.root"],
    ["output_k4test_exampledata.root"],
]
overlay.StartBackgroundEventIndex = 95
overlay.NumberBackground = [2, 1]
overlay.Poisson_random_NOverlay = [False, False]
overlay.NBunchtrain = 1
overlay.TimeWindows = {"SimTrackerHits": [-10000, 10000]}

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=4,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
