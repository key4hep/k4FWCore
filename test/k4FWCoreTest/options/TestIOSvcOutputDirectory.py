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

"""Write one event to a configurable output path."""

from Configurables import EventDataSvc, k4FWCoreTest_CreateExampleEventData
from k4FWCore import ApplicationMgr, IOSvc
from k4FWCore.parseArgs import parser

parser.add_argument("--output", default="output/nested/output.root")
parser.add_argument("--mode", choices=["constructor", "assignment"], default="assignment")
args, _ = parser.parse_known_args()

if args.mode == "constructor":
    svc = IOSvc(Output=args.output)
else:
    svc = IOSvc()
    svc.Output = args.output

ApplicationMgr(
    TopAlg=[k4FWCoreTest_CreateExampleEventData()],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc()],
)
