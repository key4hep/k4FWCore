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

# This is an example reading from a file and using a transformer to create new
# data

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalTransformer
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import ExampleTupleWriter

from k4FWCore.parseArgs import parser

parser.add_argument(
    "--add-ttree-ntuple",
    action="store_true",
    help="Add TTree Ntuple to the output files",
)
parser.add_argument(
    "--use-rntuple-for-ntuple",
    action="store_true",
    help="Use RNTuple for the Ntuple output",
)

args = parser.parse_known_args()[0]

svc = IOSvc("IOSvc")
svc.Input = "functional_producer.root"
svc.Output = "functional_transformer.root"

transformer = ExampleFunctionalTransformer(
    "Transformer", InputCollection="MCParticles", OutputCollection="NewMCParticles"
)

extra_algs = []
if args.add_ttree_ntuple:
    tuple_writer = ExampleTupleWriter(
        "TupleWriter",
        OutputFile="ntuple.root",
        Names=["rntuple"] if args.use_rntuple_for_ntuple else ["tree"],
        RNTuple=args.use_rntuple_for_ntuple,
    )
    extra_algs.append(tuple_writer)


mgr = ApplicationMgr(
    TopAlg=[transformer] + extra_algs,
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
