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

# This is an example producing, transforming and consuming data in memory

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleFunctionalTransformer,
    ExampleFunctionalProducer,
    ExampleFunctionalConsumer,
    ExampleFunctionalConsumerKeyValues,
)
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

from k4FWCore.parseArgs import parser

parser.add_argument("--use-key-values", action="store_true", help="Use KeyValues for consumer")

my_opts = parser.parse_known_args()

transformer = ExampleFunctionalTransformer(
    "Transformer", InputCollection="MCParticles", OutputCollection="NewMCParticles"
)

if not my_opts[0].use_key_values:
    consumer = ExampleFunctionalConsumer(
        "Consumer",
        InputCollection="NewMCParticles",
        Offset=10,
    )
else:
    consumer = ExampleFunctionalConsumerKeyValues(
        "Consumer",
        InputCollection=["NewMCParticles"],
        Offset=10,
    )

producer = ExampleFunctionalProducer("Producer", OutputCollection="MCParticles")


ApplicationMgr(
    TopAlg=[producer, transformer, consumer],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
