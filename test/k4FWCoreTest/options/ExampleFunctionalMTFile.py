#
# Copyright (c) 2014-2023 Key4hep-Project.
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

from Gaudi.Configuration import INFO, WARNING
from Gaudi.Configuration import Gaudi__Sequencer
from Configurables import ExampleFunctionalTransformer
from Configurables import ApplicationMgr
from Configurables import IOSvc
from Configurables import Reader, Writer
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard, AvalancheSchedulerSvc

evtslots = 2
threads = 2

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            ForceLeaves=True,
                            )

slimeventloopmgr = HiveSlimEventLoopMgr("HiveSlimEventLoopMgr",
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=WARNING
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

svc = IOSvc("IOSvc")
svc.input = ['output_k4test_exampledata_producer.root']
svc.output = 'functional_transformerMT.root'
# svc.CollectionNames = ['MCParticles']

reader = Reader("Reader")

writer = Writer("Writer")

# out = PodioOutput("out")
# out.filename = "output_k4test_exampledata_transformer.root"
# # The collections that we don't drop will also be present in the output file
# out.outputCommands = ["drop MCParticles"]

transformer = ExampleFunctionalTransformer("Transformer",
                                           InputCollection="MCParticles",
                                           OutputCollection="NewMCParticles")

seq = Gaudi__Sequencer("Node", Members=[reader, transformer, writer], Sequential=True, OutputLevel=INFO)

mgr = ApplicationMgr(TopAlg=[seq],
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,      
               MessageSvcType="InertMessageSvc",
               OutputLevel=INFO,
               )
    
