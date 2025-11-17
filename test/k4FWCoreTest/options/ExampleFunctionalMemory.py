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
    ExampleFunctionalTransformerEventContext,
    ExampleFunctionalProducer,
    ExampleFunctionalConsumer,
    ExampleFunctionalConsumerEventContext,
    ExampleFunctionalConsumerKeyValues,
)
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

from k4FWCore.parseArgs import parser

arg_group = parser.add_mutually_exclusive_group()
arg_group.add_argument("--use-key-values", action="store_true", help="Use KeyValues for consumer")
arg_group.add_argument(
    "--use-event-context",
    action="store_true",
    help="Use the variants of the consumer and transformer that take an EventContext",
)

my_opts = parser.parse_known_args()

consumer_alg = ExampleFunctionalConsumer
transformer_alg = ExampleFunctionalTransformer
if my_opts[0].use_event_context:
    consumer_alg = ExampleFunctionalConsumerEventContext
    transformer_alg = ExampleFunctionalTransformerEventContext

transformer = transformer_alg(
    "Transformer", InputCollection="MCParticles", OutputCollection="NewMCParticles"
)

if my_opts[0].use_key_values:
    consumer = ExampleFunctionalConsumerKeyValues(
        "Consumer",
        InputCollection=["NewMCParticles"],
        Offset=10,
    )
else:
    consumer = consumer_alg(
        "Consumer",
        InputCollection="NewMCParticles",
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
