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

# This is an example reading from a file and using a producer with several inputs
# to check that the contents of the file are the expected ones

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalProducer, ExampleFunctionalConsumerRuntimeCollections
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

producer0 = ExampleFunctionalProducer("Producer0",
                                     OutputCollection="MCParticles0",
                                     )
producer1 = ExampleFunctionalProducer("Producer1",
                                     OutputCollection="MCParticles1",
                                     )
producer2 = ExampleFunctionalProducer("Producer2",
                                     OutputCollection="MCParticles2",
                                     )
consumer = ExampleFunctionalConsumerRuntimeCollections("Consumer",
                                     # InputCollection="MCParticles0 MCParticles1 MCParticles2",
                                     InputCollection=["MCParticles0", "MCParticles1", "MCParticles2"],
                                     Offset=0,
                                     )


ApplicationMgr(TopAlg=[producer0, producer1, producer2, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[EventDataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
