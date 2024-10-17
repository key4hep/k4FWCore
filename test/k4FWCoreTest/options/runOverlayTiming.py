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
from Gaudi.Configuration import INFO

from k4FWCore import ApplicationMgr
from k4FWCore import IOSvc
from Configurables import EventDataSvc
from Configurables import OverlayTiming
from Configurables import UniqueIDGenSvc

id_service = UniqueIDGenSvc("UniqueIDGenSvc")

eds = EventDataSvc("EventDataSvc")

iosvc = IOSvc()
# iosvc.input = "input.root"
iosvc.Input = "input.root"
iosvc.Output = "output_overlay.root"

# inp.collections = [
#     "EventHeader",
#     "MCParticle",
#     "VertexBarrelCollection",
#     "VertexEndcapCollection",
#     "HCalRingCollection",
# ]

overlay = OverlayTiming()
overlay.MCParticles = ["MCParticle"]
overlay.SimTrackerHits = ["VertexBarrelCollection", "VertexEndcapCollection"]
overlay.SimCalorimeterHits = ["HCalRingCollection"]
overlay.BackgroundMCParticleCollectionName = "MCParticle"
overlay.OutputSimTrackerHits = ["NewVertexBarrelCollection", "NewVertexEndcapCollection"]
overlay.OutputSimCalorimeterHits = ["NewHCalRingCollection"]
overlay.OutputCaloHitContributions = ["NewCaloHitCollection"]
# overlay.StartBackgroundEventIndex = 0
overlay.NumberBackground = [0.32, 15]
overlay.Poisson_random_NOverlay = [True, False]
overlay.BackgroundFileNames = [
      ["background_group1_1.root"],
      ["background_group2_1.root"],
]
overlay.TimeWindows = {"MCParticle": [0, 23.5], "VertexBarrelCollection": [0, 23.5], "VertexEndcapCollection": [0, 23.5], "HCalRingCollection": [0, 23.5]}
overlay.CopyCellIDMetadata = True

ApplicationMgr(TopAlg=[overlay],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[eds],
               OutputLevel=INFO,
               )
