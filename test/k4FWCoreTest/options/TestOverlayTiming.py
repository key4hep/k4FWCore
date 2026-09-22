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

# Tests the OverlayTiming algorithm using functional_producer_multiple.root as
# both signal and background. SimTrackerHits and SimCalorimeterHits, plus their
# relation-free counterparts, are overlaid.
#
# Indexing the output particle collection with the -1 of an unset ObjectID used
# to produce a broken relation that crashed when the contributions were written
# out.
#
# It also covers CopyCellIDMetadata: the encoding of the input collection is
# pre-filled into the metadata, and ExampleFunctionalMetadataConsumer checks
# that OverlayTiming has published it for the overlaid output collection by the
# time a downstream algorithm initializes.

from Gaudi.Configuration import INFO
from Configurables import (
    EventDataSvc,
    EventHeaderCreator,
    ExampleFunctionalMetadataConsumer,
    MetadataSvc,
    OverlayTiming,
    UniqueIDGenSvc,
)
from k4FWCore import ApplicationMgr, IOSvc

ENCODING = "system:5,side:-2,layer:9,module:8,sensor:8"

uid_svc = UniqueIDGenSvc("UniqueIDGenSvc")

iosvc = IOSvc("IOSvc")
iosvc.Input = "functional_producer_multiple.root"
iosvc.Output = "overlay_output.root"

header = EventHeaderCreator("EventHeaderCreator")

# Stand in for an input file that carries the cellID encoding of the hits
metadata_svc = MetadataSvc("MetadataSvc")
metadata_svc.StringParameters = {
    "SimTrackerHits__CellIDEncoding": ENCODING,
    "SimTrackerHitsWithoutParticleRelations__CellIDEncoding": ENCODING,
    "SimCalorimeterHits__CellIDEncoding": ENCODING,
    "SimCalorimeterHitsWithoutParticleRelations__CellIDEncoding": ENCODING,
}

overlay = OverlayTiming("OverlayTiming")
overlay.MCParticles = "MCParticles1"
overlay.SimTrackerHits = [
    "SimTrackerHits",
    "SimTrackerHitsWithoutParticleRelations",
]
overlay.SimCalorimeterHits = [
    "SimCalorimeterHits",
    "SimCalorimeterHitsWithoutParticleRelations",
]
overlay.OutputMCParticles = "OverlayMCParticles"
overlay.OutputSimTrackerHits = [
    "OverlaySimTrackerHits",
    "OverlaySimTrackerHitsWithoutParticleRelations",
]
overlay.OutputSimCalorimeterHits = [
    "OverlaySimCalorimeterHits",
    "OverlaySimCalorimeterHitsWithoutParticleRelations",
]
overlay.OutputCaloHitContributions = [
    "OverlayCaloHitContributions",
    "OverlayCaloHitContributionsWithoutParticleRelations",
]
overlay.BackgroundMCParticleCollectionName = "MCParticles1"
overlay.BackgroundFileNames = [["functional_producer_multiple.root"]]
overlay.NumberBackground = [1]
overlay.Poisson_random_NOverlay = [False]
overlay.NBunchtrain = 1
overlay.TimeWindows = {
    "SimTrackerHits": [-10000, 10000],
    "SimTrackerHitsWithoutParticleRelations": [-10000, 10000],
    "SimCalorimeterHits": [-10000, 10000],
    "SimCalorimeterHitsWithoutParticleRelations": [-10000, 10000],
}
overlay.CopyCellIDMetadata = True

# Fails to initialize if OverlayTiming has not published the encoding of
# OverlaySimTrackerHits by the time this algorithm is initialized
encoding_consumer = ExampleFunctionalMetadataConsumer("EncodingConsumer")
encoding_consumer.InputCollection = ["OverlayMCParticles"]
encoding_consumer.CheckMetadata = False
encoding_consumer.CellIDCollection = "OverlaySimTrackerHits"
encoding_consumer.ExpectedCellIDEncoding = ENCODING

ApplicationMgr(
    TopAlg=[header, overlay, encoding_consumer],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
