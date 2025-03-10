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
from Configurables import k4DataSvc
from Configurables import PodioInput
from Configurables import PodioOutput
from k4FWCore import ApplicationMgr

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata.root"
podioevent.OutputLevel = DEBUG

inp = PodioInput()
inp.collections = ["MCParticles", "Links"]

output = PodioOutput()
output.filename = "output_k4test_exampledata_limited.root"

ApplicationMgr(
    TopAlg=[inp, output], EvtSel="NONE", EvtMax=5, ExtSvc=[podioevent], OutputLevel=DEBUG
)
