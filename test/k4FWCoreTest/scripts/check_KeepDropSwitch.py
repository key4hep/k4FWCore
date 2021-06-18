import ROOT

file = ROOT.TFile.Open("output_k4test_exampledata_2.root")
tree = file.Get("events")
tree.GetEntry(0)

ndf = tree.Tracks.at(0).ndf
if ndf==0:
  raise Exception('podio::CollectionBase read from file did not saved properly')

status = tree.GetBranchStatus("MCParticles")
if status==True:
  raise Exception('KeepDropSwitch did not drop the collection')

