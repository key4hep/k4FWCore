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

# This is an example using a producer with a multiple outputs and saving that to a file

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalProducerMultiple
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput

podioevent = k4DataSvc("EventDataSvc")

out = PodioOutput("out")
out.filename = "output_k4test_exampledata_producer_multiple.root"
# Collections can be dropped
# out.outputCommands = ["drop *"]

producer = ExampleFunctionalProducerMultiple("ExampleFunctionalProducerMultiple",
                                             OutputCollectionFloat="VectorFloat",
                                             OutputCollectionParticles1="MCParticles1",
                                             OutputCollectionParticles2="MCParticles2",
                                             OutputCollectionSimTrackerHits="SimTrackerHits",
                                             OutputCollectionTrackerHits="TrackerHits",
                                             OutputCollectionTracks="Tracks",
                                             ExampleInt=5)

ApplicationMgr(TopAlg=[producer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
