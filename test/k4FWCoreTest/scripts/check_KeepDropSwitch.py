#
# Copyright (c) 2014-2023 Key4hep-Project.
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
import ROOT

ROOT.gSystem.Load("libedm4hepDict")

file = ROOT.TFile.Open("output_k4test_exampledata_2.root")
tree = file.Get("events")
tree.GetEntry(0)

ndf = tree.Tracks.at(0).ndf
if ndf==0:
  raise Exception('podio::CollectionBase read from file did not saved properly')

status = tree.GetBranchStatus("MCParticles")
if status==True:
  raise Exception('KeepDropSwitch did not drop the collection')

