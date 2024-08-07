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

# This is an example using a producer that creates any number of collections and
# then reading those with consumers

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleFunctionalProducerRuntimeCollections,
    ExampleFunctionalConsumer,
)
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

producer = ExampleFunctionalProducerRuntimeCollections(
    "Producer",
    OutputCollections=["MCParticles0", "MCParticles1", "MCParticles2"],
)

consumer0 = ExampleFunctionalConsumer(
    "Consumer0",
    InputCollection=["MCParticles0"],
    Offset=0,
)
consumer1 = ExampleFunctionalConsumer(
    "Consumer1",
    InputCollection=["MCParticles1"],
    Offset=0,
)
consumer2 = ExampleFunctionalConsumer(
    "Consumer2",
    InputCollection=["MCParticles2"],
    Offset=0,
)


ApplicationMgr(
    TopAlg=[producer, consumer0, consumer1, consumer2],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
