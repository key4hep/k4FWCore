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
from Gaudi.Configuration import INFO

from Configurables import k4DataSvc
from Configurables import k4FWCoreTest_cellID_writer, k4FWCoreTest_cellID_reader
from Configurables import PodioOutput
from k4FWCore import ApplicationMgr

podioevent = k4DataSvc("EventDataSvc")


producer = k4FWCoreTest_cellID_writer(
    "CellIDWriter",
    intProp2=69,
    floatProp2=2.71828,
    doubleProp2=2.7182818,
    stringProp2="Hello, World!",
    vectorIntProp2=[1, 2, 3, 4],
    vectorFloatProp2=[1.1, 2.2, 3.3, 4.4],
    vectorDoubleProp2=[1.1, 2.2, 3.3, 4.4],
    vectorStringProp2=["one", "two", "three", "four"],
)
consumer = k4FWCoreTest_cellID_reader()


out = PodioOutput("out")
out.filename = "output_k4test_exampledata_cellid.root"
out.outputCommands = ["keep *"]


ApplicationMgr(
    TopAlg=[producer, consumer, out],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[podioevent],
    OutputLevel=INFO,
    StopOnSignal=True,
)
