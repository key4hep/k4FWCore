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

# This is an example checking EventHeader created in GaudiHive
# The checks include whether a correct range of EventNumber is created
# and if there are no duplicates

from Gaudi.Configuration import *

from Configurables import ExampleEventHeaderCheck
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc


eventHeaderCreator = ExampleEventHeaderCheck(
    "eventHeaderCreator",
    runNumber=42,
    expectedEventNumbers=list(range(42, 42 + 10)),
    OutputLevel=DEBUG,
)

svc = IOSvc("IOSvc")
svc.input = "eventHeaderConcurrent.root"

ApplicationMgr(
    TopAlg=[eventHeaderCreator],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)
