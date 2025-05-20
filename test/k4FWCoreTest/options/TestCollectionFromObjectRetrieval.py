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

# This is a simple test checking that the collection IDs that are assigned for
# the Functional algorithms are the same as we get when adding to a frame.
# Effectively, it checks whether we hash the right thing when we assign a
# collection ID. We need this check because for Functional algorithms the
# insertion into a Frame (where the usual assignment happens) happens just
# before writing.

from Gaudi.Configuration import VERBOSE
from Configurables import (
    TestCollectionFromObjectRetrieval,
    ExampleFunctionalProducerMultiple,
    CollectionFromObjectSvc,
)

from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

producer = ExampleFunctionalProducerMultiple(
    "Producer", OutputCollectionParticles1=["UnconventionalCollName"], OutputLevel=VERBOSE
)


collid_checker = TestCollectionFromObjectRetrieval(
    "CollectionIDChecker", InputCollection=["UnconventionalCollName"], OutputLevel=VERBOSE
)

ApplicationMgr(
    TopAlg=[producer, collid_checker],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc(), CollectionFromObjectSvc("CollectionFromObjSvc")],
)
