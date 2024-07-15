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

# This is an example reading from a file and using a consumer with several inputs
# to check that the contents of the file are the expected ones

from Gaudi.Configuration import INFO, DEBUG
from Configurables import CollectionMerger
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc

from Configurables import ExampleFunctionalProducer

svc = IOSvc("IOSvc")
svc.input = "output_k4test_exampledata_producer_multiple.root"
svc.output = "functional_merged_collections.root"
svc.outputCommands = ["drop *",
                      "keep MCParticles1",
                      "keep MCParticles2",
                      "keep MCParticles3",
                      "keep NewMCParticles",
                      "keep SimTrackerHits",
                      ]


particle_producer = ExampleFunctionalProducer(
    OutputCollection=["MCParticles3"],
    )


merger = CollectionMerger(
    "CollectionConcatenator",
    # List of collections to concatenate
    InputCollections=["MCParticles2", "MCParticles1", "MCParticles3"],
    # Name of the single output collection
    OutputCollection=["NewMCParticles"],
    OutputLevel=DEBUG,
)

# If we want to copy instead of creating a subset collection
merger.Copy = True

mgr = ApplicationMgr(
    TopAlg=[particle_producer, merger],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
