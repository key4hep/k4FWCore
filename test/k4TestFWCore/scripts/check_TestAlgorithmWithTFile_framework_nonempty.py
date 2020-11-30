import ROOT

f = ROOT.TFile.Open("output_TestAlgorithmWithTFile_framework.root")
t = f.Get("events")
print("File: output_TestAlgorithmWithTFile_framework.root  contains TTree events with " + str(t.GetEntries()) + " entries.")
