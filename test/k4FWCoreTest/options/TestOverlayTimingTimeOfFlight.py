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

# Tests that OverlayTiming selects every hit, signal and background, tracker and
# calorimeter, in the same way: relative to the time of flight from the interaction
# point (ApplyTimeOfFlight, the default), or by its absolute time with
# --OverlayTiming.ApplyTimeOfFlight=False. It also tests that only with the time of
# flight the collections of a bunch crossing after the time window are skipped.
# Signal and background are tof_signal.root and tof_background.root, written by
# CreateOverlayBackgroundFiles.py, overlaid in two bunch crossings.

from Gaudi.Configuration import INFO
from Configurables import (
    EventDataSvc,
    EventHeaderCreator,
    OverlayTiming,
    UniqueIDGenSvc,
)
from k4FWCore import ApplicationMgr, IOSvc

# The hits and the window are shared with CheckOutputFiles.py. The module is in
# the scripts directory, which the tests have on their PYTHONPATH.
from CreateOverlayBackgroundFiles import TOF_DELTA_T, TOF_WINDOW

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "overlay_background/tof_signal.root"
iosvc.Output = "overlay_tof.root"

header = EventHeaderCreator("EventHeaderCreator")

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles"
overlay.SimTrackerHits = ["SimTrackerHits"]
overlay.SimCalorimeterHits = ["SimCalorimeterHits"]
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = ["OverlaySimTrackerHits"]
overlay.OutputSimCalorimeterHits = ["OverlaySimCalorimeterHits"]
overlay.OutputCaloHitContributions = ["OverlayCaloHitContributions"]
overlay.BackgroundMCParticleCollectionName = "MCParticles"
# The single background event is overlaid in both bunch crossings
overlay.BackgroundFileNames = [["overlay_background/tof_background.root"]]
overlay.AllowReusingBackgroundFiles = True
overlay.NumberBackground = [1]
overlay.Poisson_random_NOverlay = [False]
overlay.NBunchtrain = 2
overlay.PhysicsBX = 1
overlay.Delta_t = TOF_DELTA_T
overlay.TimeWindows = {"SimTrackerHits": TOF_WINDOW, "SimCalorimeterHits": TOF_WINDOW}

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
