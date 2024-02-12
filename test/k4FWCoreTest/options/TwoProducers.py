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
from Gaudi.Configuration import *

from Configurables import ApplicationMgr

ApplicationMgr(
    EvtSel="NONE",
    EvtMax=100,
    OutputLevel=INFO,
    StopOnSignal=True,
)


from Configurables import k4DataSvc

podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import k4FWCoreTest_CreateExampleEventData

producer1 = k4FWCoreTest_CreateExampleEventData("Producer1")
ApplicationMgr().TopAlg += [producer1]


from Configurables import k4FWCoreTest_CreateExampleEventData

producer2 = k4FWCoreTest_CreateExampleEventData("Producer2")
producer2.mcparticles.Path = "mcparticles2"
producer2.trackhits.Path = "trackhits2"
producer2.simtrackhits.Path = "simtrackhits2"
producer2.tracks.Path = "tracks2"
producer2.vectorfloat.Path = "vectorfloat2"
ApplicationMgr().TopAlg += [producer2]

from Configurables import PodioOutput

out = PodioOutput("out")
out.filename = "output_k4test_exampledata_twoproducer.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]
