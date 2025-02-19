#!/usr/bin/env python3
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

from Gaudi.Configuration import DEBUG
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc

svc = IOSvc("IOSvc")
svc.Input = "output_k4test_exampledata.root"
# This will limit the collections to be read to only these two collections and
# the output file contain them
svc.CollectionNames = ["MCParticles", "Links"]
svc.Output = "functional_limited_input.root"

mgr = ApplicationMgr(
    TopAlg=[],
    EvtSel="NONE",
    EvtMax=2,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=DEBUG,
)
