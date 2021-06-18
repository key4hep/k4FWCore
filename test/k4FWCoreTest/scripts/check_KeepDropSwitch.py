import ROOT

file = ROOT.TFile.Open("output_k4test_exampledata_2.root")
tree = file.Get("events")
tree.GetEntry(0)

try:
  ndf = tree.Tracks.at(0).ndf
  if ndf==0:
    raise Exception('cannot retrieve Track.ndf')
except Exception as IOError:
  print('podio::CollectionBase read from file did not saved properly')

try:
  status = tree.GetBranchStatus("MCParticles")
  if status==True:
    raise Exception('MCParticles did not drop')
except Exception as IOError:
  print('Drop switch for PodioOutput does not work properly') 
