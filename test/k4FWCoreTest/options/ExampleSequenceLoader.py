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

# This is an example reading from a file and using a producer to create new
# data

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalTransformer
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc, SequenceLoader
from pathlib import Path

svc = IOSvc("IOSvc")
svc.Input = "functional_producer.root"
svc.Output = "gaudi_functional.root"

algList = []
sequenceLoader = SequenceLoader(algList)

sequenceLoader.load(f"{Path(__file__).parent}/ExampleSequence")

transformer = ExampleFunctionalTransformer(
    "Transformer", InputCollection="MCParticles", OutputCollection="NewMCParticles"
)
algList.append(transformer)

mgr = ApplicationMgr(
    TopAlg=algList,
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
