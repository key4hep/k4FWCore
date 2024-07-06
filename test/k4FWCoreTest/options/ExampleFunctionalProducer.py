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

# This is an example using a producer with a single output and saving that to a file

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalProducer
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc
from k4FWCore.parseArgs import parser

parser.add_argument("--second", action="store_true")
args = parser.parse_known_args()

iosvc = IOSvc("IOSvc")
name = (
    "functional_producer.root"
    if not args[0].second
    else "functional_producer2.root"
)
iosvc.output = name
# Collections can be dropped
# out.outputCommands = ["drop *"]


producer = ExampleFunctionalProducer("ExampleFunctionalProducer")

ApplicationMgr(
    TopAlg=[producer],
    EvtSel="NONE",
    EvtMax=10 if not args[0].second else 20,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
