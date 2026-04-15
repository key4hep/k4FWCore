#
# Copyright (c) 2020-2024 Key4hep-Project.
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

# Tests the OverlayTiming algorithm using functional_producer_multiple.root as
# both signal and background. MCParticles and SimTrackerHits are overlaid.

from Gaudi.Configuration import INFO
from Configurables import EventDataSvc, EventHeaderCreator, OverlayTiming, UniqueIDGenSvc
from k4FWCore import ApplicationMgr, IOSvc

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "functional_producer_multiple.root"
iosvc.Output = "overlay_output.root"

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
overlay.NBunchtrain = 1
overlay.TimeWindows = {"SimTrackerHits": [-10000, 10000]}

ApplicationMgr(
    TopAlg=[header, overlay],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
