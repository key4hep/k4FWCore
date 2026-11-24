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

# This is an example copying frame categories that the framework does not
# handle itself (e.g. the "runs" frame written by ddsim) from the input files
# to the output file with IOSvc.CopyInputCategories

from Gaudi.Configuration import INFO
from Configurables import EventDataSvc, ExampleFunctionalTransformer
from k4FWCore import ApplicationMgr, IOSvc
from k4FWCore.parseArgs import parser

parser.add_argument("--no-copy", action="store_true", help="Do not copy any input category")
args = parser.parse_known_args()[0]

iosvc = IOSvc("IOSvc")
iosvc.Input = ["extra_categories_0.root", "extra_categories_1.root"]
iosvc.Output = "functional_copy_input_categories_all.root"
# The default is to copy all the categories not handled by the framework ("*").
# Specific categories can be selected from the command line, e.g.
# --IOSvc.CopyInputCategories runs
if args.no_copy:
    iosvc.CopyInputCategories = []

transformer = ExampleFunctionalTransformer(
    "Transformer", InputCollection="MCParticles", OutputCollection="NewMCParticles"
)

ApplicationMgr(
    TopAlg=[transformer],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
