import ROOT

f = ROOT.TFile.Open("output_TestAlgorithmWithTFile_myTFile.root")
t = f.Get("mytree")
print("File: output_TestAlgorithmWithTFile_myTFile.root  contains TTree mytree with " + str(t.GetEntries()) + " entries.")
