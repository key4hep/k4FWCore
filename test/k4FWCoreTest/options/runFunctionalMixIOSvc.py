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

# This is an example mixing functional and non-functional algorithms
#

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleFunctionalConsumerMultiple,
    ExampleFunctionalTransformerMultiple,
)
from Configurables import (
    ExampleFunctionalProducerMultiple,
    k4FWCoreTest_CreateExampleEventData,
)
from Configurables import k4FWCoreTest_CheckExampleEventData
from k4FWCore import ApplicationMgr, IOSvc

iosvc = IOSvc("IOSvc")
iosvc.input = "output_k4test_exampledata_producer_multiple.root"

# inp.collections = [
#     "VectorFloat",
#     "MCParticles1",
#     "MCParticles2",
#     "SimTrackerHits",
#     "TrackerHits",
#     "Tracks",
# ]

consumer_input_functional = ExampleFunctionalConsumerMultiple(
    "ExampleFunctionalConsumerMultiple",
    Offset=0,
)
consumer_input_algorithm = k4FWCoreTest_CheckExampleEventData("CheckExampleEventData")
consumer_input_algorithm.mcparticles = "MCParticles1"
consumer_input_algorithm.keepEventNumberZero = True

# We only care about the new FunctionalMCParticles collection in this example
producer_functional = ExampleFunctionalProducerMultiple(
    "ProducerFunctional",
    OutputCollectionFloat=["VectorFloat_"],
    OutputCollectionParticles1=["FunctionalMCParticles"],
    OutputCollectionParticles2=["MCParticles2_"],
    OutputCollectionSimTrackerHits=["SimTrackerHits_"],
    OutputCollectionTrackerHits=["TrackerHits_"],
    OutputCollectionTracks=["Tracks_"],
    ExampleInt=5,
)

consumer_producerfun_functional = ExampleFunctionalConsumerMultiple(
    "FunctionalConsumerFunctional",
    InputCollectionParticles=["FunctionalMCParticles"],
    Offset=0,
)
consumer_producerfun_algorithm = k4FWCoreTest_CheckExampleEventData("CheckFunctional")
consumer_producerfun_algorithm.mcparticles = "FunctionalMCParticles"
consumer_producerfun_algorithm.keepEventNumberZero = True

producer_algorithm = k4FWCoreTest_CreateExampleEventData("CreateExampleEventData")
# We only care about the MCParticles collection
producer_algorithm.mcparticles = "AlgorithmMCParticles"
producer_algorithm.simtrackhits = "SimTrackerHits__"
producer_algorithm.trackhits = "TrackerHits__"
producer_algorithm.tracks = "Tracks__"
producer_algorithm.vectorfloat = "VectorFloat__"

consumer_produceralg_functional = ExampleFunctionalConsumerMultiple(
    "FunctionalConsumerAlgorithm",
    Offset=0,
)
consumer_produceralg_algorithm = k4FWCoreTest_CheckExampleEventData("CheckAlgorithm")
consumer_produceralg_algorithm.mcparticles = "FunctionalMCParticles"
consumer_produceralg_algorithm.keepEventNumberZero = True

# Let's also run the transformer, why not
transformer_functional = ExampleFunctionalTransformerMultiple(
    "FunctionalTransformerMultiple"
)

iosvc.output = "output_k4test_exampledata_functional_mix_iosvc.root"

ApplicationMgr(
    TopAlg=[
        # Check we can read input
        consumer_input_functional,
        consumer_input_algorithm,
        producer_functional,
        # Check we can read what's produced by a functional
        consumer_producerfun_functional,
        consumer_producerfun_algorithm,
        producer_algorithm,
        # Check we can read what's produced by an algorithm
        consumer_produceralg_functional,
        consumer_produceralg_algorithm,
        transformer_functional,
    ],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[iosvc],
    OutputLevel=INFO,
)
