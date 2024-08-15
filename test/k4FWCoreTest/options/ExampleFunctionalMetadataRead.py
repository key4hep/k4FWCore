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

# This is an example accessing matadata read from input file using MetadataSvc

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalMetadataConsumer
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import EventDataSvc

iosvc = IOSvc()
iosvc.input = "functional_metadata.root"
iosvc.output = "functional_metadata_propagate.root"

consumer = ExampleFunctionalMetadataConsumer("Consumer", InputCollection=["MCParticles"])

ApplicationMgr(
    TopAlg=[consumer],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
