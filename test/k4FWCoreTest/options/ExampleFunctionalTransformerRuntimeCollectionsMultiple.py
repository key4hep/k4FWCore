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

# This is an example using multiple producers, a transformer that can read any
# number of collections for its input and multiple consumers to check the
# transformed collections

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleFunctionalTransformerRuntimeCollectionsMultiple,
    ExampleFunctionalConsumerMultiple,
    ExampleFunctionalProducerMultiple,
)
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

producer0 = ExampleFunctionalProducerMultiple(
    "Producer0",
    OutputCollectionFloat=["VectorFloat0"],
    OutputCollectionParticles1=["MCParticles0"],
    OutputCollectionParticles2=["MCParticles1"],
    OutputCollectionSimTrackerHits=["SimTrackerHits0"],
    OutputCollectionTrackerHits=["TrackerHits0"],
    OutputCollectionTracks=["Tracks0"],
    OutputCollectionRecoParticles=["Recos0"],
    ExampleInt=5,
)
producer1 = ExampleFunctionalProducerMultiple(
    "Producer1",
    OutputCollectionFloat=["VectorFloat1"],
    OutputCollectionParticles1=["MCParticles2"],
    OutputCollectionParticles2=["MCParticles3"],
    OutputCollectionSimTrackerHits=["SimTrackerHits1"],
    OutputCollectionTrackerHits=["TrackerHits1"],
    OutputCollectionTracks=["Tracks1"],
    OutputCollectionRecoParticles=["Recos1"],
    ExampleInt=5,
)
producer2 = ExampleFunctionalProducerMultiple(
    "Producer2",
    OutputCollectionFloat=["VectorFloat2"],
    OutputCollectionParticles1=["MCParticles4"],
    OutputCollectionParticles2=["MCParticles5"],
    OutputCollectionSimTrackerHits=["SimTrackerHits2"],
    OutputCollectionTrackerHits=["TrackerHits2"],
    OutputCollectionTracks=["Tracks2"],
    OutputCollectionRecoParticles=["Recos2"],
    ExampleInt=5,
)

transformer = ExampleFunctionalTransformerRuntimeCollectionsMultiple(
    "Transformer",
    InputCollectionFloat=["VectorFloat0", "VectorFloat1", "VectorFloat2"],
    InputCollectionParticles=["MCParticles0", "MCParticles2", "MCParticles4"],
    InputCollectionSimTrackerHits=[
        "SimTrackerHits0",
        "SimTrackerHits1",
        "SimTrackerHits2",
    ],
    InputCollectionTrackerHits=["TrackerHits0", "TrackerHits1", "TrackerHits2"],
    InputCollectionTracks=["Tracks0", "Tracks1", "Tracks2"],
    OutputCollectionFloat=["NewVectorFloat0", "NewVectorFloat1", "NewVectorFloat2"],
    OutputCollectionParticles1=[
        "NewMCParticles0",
        "NewMCParticles2",
        "NewMCParticles4",
    ],
    OutputCollectionParticles2=[
        "",
    ],
    OutputCollectionSimTrackerHits=[
        "NewSimTrackerHits0",
        "NewSimTrackerHits1",
        "NewSimTrackerHits2",
    ],
    OutputCollectionTrackerHits=[
        "NewTrackerHits0",
        "NewTrackerHits1",
        "NewTrackerHits2",
    ],
    OutputCollectionTracks=["NewTracks0", "NewTracks1", "NewTracks2"],
    Offset=0,
)

consumer0 = ExampleFunctionalConsumerMultiple(
    "Consumer0",
    InputCollectionFloat=["NewVectorFloat0"],
    InputCollectionParticles=["NewMCParticles0"],
    InputCollectionSimTrackerHits=["NewSimTrackerHits0"],
    InputCollectionTrackerHits=["NewTrackerHits0"],
    InputCollectionTracks=["NewTracks0"],
)

consumer1 = ExampleFunctionalConsumerMultiple(
    "Consumer1",
    InputCollectionFloat=["NewVectorFloat1"],
    InputCollectionParticles=["NewMCParticles2"],
    InputCollectionSimTrackerHits=["NewSimTrackerHits1"],
    InputCollectionTrackerHits=["NewTrackerHits1"],
    InputCollectionTracks=["NewTracks1"],
)

consumer2 = ExampleFunctionalConsumerMultiple(
    "Consumer2",
    InputCollectionFloat=["NewVectorFloat2"],
    InputCollectionParticles=["NewMCParticles4"],
    InputCollectionSimTrackerHits=["NewSimTrackerHits2"],
    InputCollectionTrackerHits=["NewTrackerHits2"],
    InputCollectionTracks=["NewTracks2"],
)


ApplicationMgr(
    TopAlg=[
        producer0,
        producer1,
        producer2,
        transformer,
        consumer0,
        consumer1,
        consumer2,
    ],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
