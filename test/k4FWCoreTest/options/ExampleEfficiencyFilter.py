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

# This is an creating some collections and filtering after
# to check that the contents of the file are the expected ones

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleFunctionalProducerMultiple,
    EfficiencyFilter,
    UniqueIDGenSvc,
    EventHeaderCreator,
    CollectionMerger,
)
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import EventDataSvc

uid_svc = UniqueIDGenSvc()

iosvc = IOSvc("IOSvc")
iosvc.Output = "functional_random_filter.root"

producer = ExampleFunctionalProducerMultiple(
    "Producer",
)

header_producer = EventHeaderCreator("EventHeaderCreator")

# Merge collections to get more MCParticles
merger = CollectionMerger(
    "CollectionMerger",
    InputCollections=["MCParticles1"] * 10,
    OutputCollection=["MergedMCParticles"],
)

merger_links = CollectionMerger(
    "LinksMerger",
    InputCollections=["Links"] * 10,
    OutputCollection=["MergedLinks"],
)

filter = EfficiencyFilter(
    "Filter",
    InputCollection=["MergedMCParticles"],
    OutputCollection=["FilteredMCParticles"],
    Efficiency=0.8,
)

filter_not_exact = EfficiencyFilter(
    "FilterNotExact",
    InputCollection=["MergedMCParticles"],
    OutputCollection=["FilteredNotExactMCParticles"],
    Efficiency=0.8,
    Exact=False,  # If False, every hit will be kept with a probability of 0.8
    # If True, then 80% of the hits, for each event, will be kept
)

filter_links = EfficiencyFilter(
    "FilterLinks",
    InputCollection=["MergedLinks"],
    OutputCollection=["FilteredLinks"],
    Efficiency=0.8,
)

ApplicationMgr(
    TopAlg=[
        header_producer,
        producer,
        merger,
        merger_links,
        filter,
        filter_not_exact,
        filter_links,
    ],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[EventDataSvc("EventDataSvc"), uid_svc],
    OutputLevel=INFO,
)
