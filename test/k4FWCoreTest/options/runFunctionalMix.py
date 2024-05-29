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
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioInput, PodioOutput
from k4FWCore.parseArgs import parser

parser.add_argument(
    "--iosvc",
    help="Use the IOSvc instead of PodioInput and PodioOutput",
    action="store_true",
    default=False,
)
args = parser.parse_known_args()[0]

print(args.iosvc)

if not args.iosvc:
    podioevent = k4DataSvc("EventDataSvc")
    podioevent.input = "output_k4test_exampledata_producer_multiple.root"

    inp = PodioInput()
    inp.collections = [
        "VectorFloat",
        "MCParticles1",
        "MCParticles2",
        "SimTrackerHits",
        "TrackerHits",
        "Tracks",
    ]

    out = PodioOutput()
    out.filename = "output_k4test_exampledata_functional_mix.root"
    out.outputCommands = ["keep *"]

else:
    from k4FWCore import IOSvc, ApplicationMgr

    iosvc = IOSvc("IOSvc")
    iosvc.input = "output_k4test_exampledata_producer_multiple.root"
    iosvc.output = "output_k4test_exampledata_functional_mix_iosvc.root"

# Check input with functional and old algorithms

consumer_input_functional = ExampleFunctionalConsumerMultiple(
    "ExampleFunctionalConsumerMultiple",
    Offset=0,
)
consumer_input_algorithm = k4FWCoreTest_CheckExampleEventData("CheckExampleEventData")
consumer_input_algorithm.mcparticles = "MCParticles1"
consumer_input_algorithm.keepEventNumberZero = True

###############################

producer_functional = ExampleFunctionalProducerMultiple(
    "ProducerFunctional",
    OutputCollectionFloat=["FunctionalVectorFloat"],
    OutputCollectionParticles1=["FunctionalMCParticles"],
    OutputCollectionParticles2=["FunctionalMCParticles2"],
    OutputCollectionSimTrackerHits=["FunctionalSimTrackerHits"],
    OutputCollectionTrackerHits=["FunctionalTrackerHits"],
    OutputCollectionTracks=["FunctionalTracks"],
    ExampleInt=5,
)

# Check the functional-produced collections with functional and old algorithms

# Here we check the new FunctionalMCParticles and the others that are
# read from the file
consumer_producerfun_functional = ExampleFunctionalConsumerMultiple(
    "FunctionalConsumerFromFunctional",
    InputCollectionParticles=["FunctionalMCParticles"],
    Offset=0,
)
consumer_producerfun_algorithm = k4FWCoreTest_CheckExampleEventData(
    "CheckFunctional", keepEventNumberZero=True
)
consumer_producerfun_algorithm.mcparticles = "FunctionalMCParticles"
consumer_producerfun_algorithm.keepEventNumberZero = True

###############################

producer_algorithm = k4FWCoreTest_CreateExampleEventData("CreateExampleEventData")
# We only care about the MCParticles collection
producer_algorithm.mcparticles = "OldAlgorithmMCParticles"
producer_algorithm.simtrackhits = "OldAlgorithmSimTrackerHits"
producer_algorithm.trackhits = "OldAlgorithmTrackerHits"
producer_algorithm.tracks = "OldAlgorithmTracks"
producer_algorithm.vectorfloat = "OldAlgorithmVectorFloat"

# Check the functional-produced collections with functional and old algorithms

consumer_produceralg_functional = ExampleFunctionalConsumerMultiple(
    "FunctionalConsumerFromAlgorithm",
    InputCollectionParticles=["OldAlgorithmMCParticles"],
    Offset=0,
)
consumer_produceralg_algorithm = k4FWCoreTest_CheckExampleEventData("CheckAlgorithm")
consumer_produceralg_algorithm.mcparticles = "OldAlgorithmMCParticles"
consumer_produceralg_algorithm.vectorfloat = "OldAlgorithmVectorFloat"

###############################

# Let's also run the transformer on collections that are either read, produced by a functional or an algorithm
transformer_functional = ExampleFunctionalTransformerMultiple(
    "FunctionalTransformerMultiple",
    InputCollectionFloat=["VectorFloat"],
    InputCollectionParticles=["FunctionalMCParticles"],
    InputCollectionSimTrackerHits=["OldAlgorithmSimTrackerHits"],
    InputCollectionTrackerHits=["TrackerHits"],
    OutputCollectionCounter=["Counter"],
    OutputCollectionParticles=["TransformedFunctionalMCParticles1"],
)


ApplicationMgr(
    TopAlg=([inp] if not args.iosvc else [])
    + [
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
    ]
    + ([out] if not args.iosvc else []),
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[iosvc if args.iosvc else podioevent],
    OutputLevel=INFO,
)
