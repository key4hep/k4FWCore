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
from Gaudi.Configuration import *

from Configurables import k4DataSvc

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata.root"

from Configurables import PodioInput

from k4FWCore.parseArgs import parser

parser.add_argument(
    "--collections",
    action="extend",
    nargs="?",
    help="The input collections to read",
    default=["VectorFloat", "MCParticles", "SimTrackerHits", "TrackerHits", "Tracks"],
)
my_args = parser.parse_known_args()[0]

inp = PodioInput()
inp.collections = my_args.collections

from Configurables import k4FWCoreTest_CheckExampleEventData

checker = k4FWCoreTest_CheckExampleEventData()

from Configurables import ApplicationMgr

ApplicationMgr(
    TopAlg=[inp, checker],
    EvtSel="NONE",
    EvtMax=100,
    ExtSvc=[podioevent],
    OutputLevel=INFO,
    StopOnSignal=True,
)
