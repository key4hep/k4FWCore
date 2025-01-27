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

"""Example showcasing how to use ParticleID related metadata"""

from Gaudi.Configuration import INFO
from Configurables import (
    ExampleParticleIDProducer,
    ExampleParticleIDConsumer,
    ExampleFunctionalProducerMultiple,
    EventDataSvc,
)
from k4FWCore import ApplicationMgr, IOSvc

# NOTE: If you are not using the IOSvc (e.g. because you don't need I/O), make
# sure to add the MetadataSvc to the ExtSvc as that is necessary to store /
# retrieve the metadata for ParticleIDs
iosvc = IOSvc()
iosvc.Output = "example_with_particleids.root"
iosvc.outputCommands = ["drop *", "keep RecoParticles*"]

reco_producer = ExampleFunctionalProducerMultiple(
    "RecoProducer", OutputCollectionRecoParticles=["RecoParticles"]
)

pid_producer1 = ExampleParticleIDProducer(
    "PIDProducer1",
    InputCollection=["RecoParticles"],
    ParticleIDCollection=["RecoParticlesPIDs_1"],
    PIDAlgoName="PIDAlgo1",
    PIDParamNames=["single_param"],
)

pid_producer2 = ExampleParticleIDProducer(
    "PIDProducer2",
    InputCollection=["RecoParticles"],
    ParticleIDCollection=["RecoParticlesPIDs_2"],
    PIDAlgoName="PIDAlgo2",
    PIDParamNames=["param_1", "param_2", "param_3"],
)

pid_consumer = ExampleParticleIDConsumer(
    "PIDConsumer",
    RecoParticleCollection=reco_producer.OutputCollectionRecoParticles,
    # From first producer
    ParticleIDCollection1=pid_producer1.ParticleIDCollection,
    PIDAlgoName1=pid_producer1.PIDAlgoName,
    PIDParamNames1=pid_producer1.PIDParamNames,
    ParamName1="single_param",
    # From second producer
    ParticleIDCollection2=pid_producer2.ParticleIDCollection,
    PIDAlgoName2=pid_producer2.PIDAlgoName,
    PIDParamNames2=pid_producer2.PIDParamNames,
    ParamName2="param_2",
)

ApplicationMgr(
    TopAlg=[reco_producer, pid_producer1, pid_producer2, pid_consumer],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
