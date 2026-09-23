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

# Tests RandomMixBackgroundFiles of the OverlayTiming algorithm, overlaying the
# single-event background files written by CreateOverlayBackgroundFiles.py on
# top of functional_producer_multiple.root.
#
# Group A is given as a directory, which has to be expanded into its .root
# files only. Group B mixes a directory with an explicitly listed file. With 6
# bunch crossings, group A draws 18 events from its 8 files and group B 6 from
# its 4, so both wrap around their shuffled list of files at least once.

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
iosvc.Input = "functional_producer_multiple.root"
iosvc.Output = "overlay_random_mix.root"

header = EventHeaderCreator("EventHeaderCreator")

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles1"
overlay.SimTrackerHits = ["SimTrackerHits"]
overlay.SimCalorimeterHits = ["SimCalorimeterHits"]
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = ["OverlaySimTrackerHits"]
overlay.OutputSimCalorimeterHits = ["OverlaySimCalorimeterHits"]
overlay.OutputCaloHitContributions = ["OverlayCaloHitContributions"]
overlay.BackgroundMCParticleCollectionName = "MCParticles"
overlay.RandomMixBackgroundFiles = True
overlay.BackgroundFileNames = [
    ["overlay_background/groupA"],
    ["overlay_background/groupB", "overlay_background/groupB_extra.root"],
]
overlay.NumberBackground = [3, 1]
overlay.Poisson_random_NOverlay = [False, False]
overlay.NBunchtrain = 6
overlay.TimeWindows = {
    "SimTrackerHits": [-10000, 10000],
    "SimCalorimeterHits": [-10000, 10000],
}

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
