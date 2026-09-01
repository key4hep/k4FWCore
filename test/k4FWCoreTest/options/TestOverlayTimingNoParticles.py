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

# Regression test: OverlayTiming must cope with SimTrackerHits and
# CaloHitContributions whose MCParticle relation is unset, both in the signal
# event and in the background event. Indexing the output particle collection
# with the -1 of an unset ObjectID used to produce a broken relation that
# crashed when the contributions were written out.

from Gaudi.Configuration import INFO
from Configurables import EventDataSvc, EventHeaderCreator, OverlayTiming, UniqueIDGenSvc
from k4FWCore import ApplicationMgr, IOSvc

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "sim_hits_without_particles.root"
iosvc.Output = "overlay_no_particles_output.root"

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
# The same file is used as signal and as background, so the unset relations are
# exercised on both the signal-copy and the background-merge path.
overlay.BackgroundFileNames = [["sim_hits_without_particles.root"]]
overlay.NumberBackground = [1]
overlay.Poisson_random_NOverlay = [False]
overlay.NBunchtrain = 1
overlay.AllowReusingBackgroundFiles = True
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
