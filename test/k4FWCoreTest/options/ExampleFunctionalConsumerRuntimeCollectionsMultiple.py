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
from Configurables import ExampleFunctionalProducerMultiple, ExampleFunctionalConsumerRuntimeCollectionsMultiple
from k4FWCore import ApplicationMgr
from Configurables import EventDataSvc

producer0 = ExampleFunctionalProducerMultiple("Producer0",
                                              OutputCollectionFloat=["VectorFloat0"],
                                              OutputCollectionParticles1=["MCParticles0"],
                                              OutputCollectionParticles2=["MCParticles1"],
                                              OutputCollectionSimTrackerHits=["SimTrackerHits0"],
                                              OutputCollectionTrackerHits=["TrackerHits0"],
                                              OutputCollectionTracks=["Tracks0"],
                                              ExampleInt=5
                                              )
producer1 = ExampleFunctionalProducerMultiple("Producer1",
                                              OutputCollectionFloat=["VectorFloat1"],
                                              OutputCollectionParticles1=["MCParticles2"],
                                              OutputCollectionParticles2=["MCParticles3"],
                                              OutputCollectionSimTrackerHits=["SimTrackerHits1"],
                                              OutputCollectionTrackerHits=["TrackerHits1"],
                                              OutputCollectionTracks=["Tracks1"],
                                              ExampleInt=5
                                              )
producer2 = ExampleFunctionalProducerMultiple("Producer2",
                                              OutputCollectionFloat=["VectorFloat2"],
                                              OutputCollectionParticles1=["MCParticles4"],
                                              OutputCollectionParticles2=["MCParticles5"],
                                              OutputCollectionSimTrackerHits=["SimTrackerHits2"],
                                              OutputCollectionTrackerHits=["TrackerHits2"],
                                              OutputCollectionTracks=["Tracks2"],
                                              ExampleInt=5
                                              )

consumer = ExampleFunctionalConsumerRuntimeCollectionsMultiple("Consumer",
                                                               Particles=["MCParticles0", "MCParticles1", "MCParticles2", "MCParticles3", "MCParticles4"],
                                                               Tracks=["Tracks0", "Tracks1", "Tracks2"],
                                                               SimTrackerHits="SimTrackerHits0",
                                                               Offset=0,
                                                               )


ApplicationMgr(TopAlg=[producer0, producer1, producer2, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[EventDataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
