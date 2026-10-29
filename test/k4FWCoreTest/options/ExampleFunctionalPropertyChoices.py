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

# This is an example of an algorithm with properties that are restricted to a
# fixed set of allowed values with the helpers from k4FWCore/PropertyChoices.h:
# two enums selected by name, an integer and a string

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalPropertyChoices
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr

alg = ExampleFunctionalPropertyChoices(
    "ExampleFunctionalPropertyChoices",
    Gandalf="TheWhite",
    NumBreakfasts=0,
    FellowshipLeader="Frodo",
    SurvivalRate="WithResurrection",
)

ApplicationMgr(
    TopAlg=[alg],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
