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

from Configurables import EventHeaderCreator
from Configurables import k4DataSvc
from Configurables import PodioOutput
from Configurables import ExampleRNGSeedingAlg
from k4FWCore import ApplicationMgr

eventHeaderCreator = EventHeaderCreator(
    "eventHeaderCreator", runNumber=42, eventNumberOffset=42, OutputLevel=DEBUG
)

# algorithm using the header to seed a prng
rngAlg = ExampleRNGSeedingAlg("ExampleRNGSeedingAlg")

podioevent = k4DataSvc("EventDataSvc")


out = PodioOutput("out")
out.filename = "eventHeader.root"


ApplicationMgr(
    TopAlg=[
        eventHeaderCreator,
        rngAlg,
        out,
    ],
    EvtSel="NONE",
    EvtMax=2,
    ExtSvc=[podioevent],
    StopOnSignal=True,
)
