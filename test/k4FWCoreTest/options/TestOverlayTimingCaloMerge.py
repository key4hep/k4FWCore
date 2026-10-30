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

# Tests how OverlayTiming merges calorimeter hits: hits with only some
# contributions in the time window are kept whole (or, with
# --OverlayTiming.KeepWholeCaloHits=False, only with those contributions), and
# background hits landing on occupied cells add their contributions and energies
# to the hits there.
# Signal and background are calo_merge_signal.root and calo_merge_background.root,
# written by CreateOverlayBackgroundFiles.py, overlaid in a single bunch crossing.

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
from CreateOverlayBackgroundFiles import CALO_MERGE_WINDOW

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "overlay_background/calo_merge_signal.root"
iosvc.Output = "overlay_calo_merge.root"

header = EventHeaderCreator("EventHeaderCreator")

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles"
overlay.SimTrackerHits = []
overlay.SimCalorimeterHits = ["SimCalorimeterHits"]
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = []
overlay.OutputSimCalorimeterHits = ["OverlaySimCalorimeterHits"]
overlay.OutputCaloHitContributions = ["OverlayCaloHitContributions"]
overlay.BackgroundMCParticleCollectionName = "MCParticles"
overlay.BackgroundFileNames = [["overlay_background/calo_merge_background.root"]]
overlay.NumberBackground = [1]
overlay.Poisson_random_NOverlay = [False]
overlay.NBunchtrain = 1
overlay.TimeWindows = {"SimCalorimeterHits": CALO_MERGE_WINDOW}

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
