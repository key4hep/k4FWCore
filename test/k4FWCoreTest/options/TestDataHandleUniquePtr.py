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

ApplicationMgr().EvtSel = "NONE"
ApplicationMgr().EvtMax = 100
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().StopOnSignal = True

from Configurables import k4DataSvc

podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import TestDataHandleUniquePtr

producer = TestDataHandleUniquePtr()
ApplicationMgr().TopAlg += [producer]

from Configurables import PodioOutput

out = PodioOutput("out")
out.filename = "output_TestDataHandleUniquePtr.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]
