from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalTransformerMultiple
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput
from Configurables import PodioInput

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer_multiple.root"

inp = PodioInput()
inp.collections = [
    "VectorFloat",
    "MCParticles1",
    "SimTrackerHits",
    "TrackerHits",
    "Tracks",
]

out = PodioOutput("out")
out.filename = "output_k4test_exampledata_transformer_multiple.root"
# The collections that we don't drop will also be present in the output file
out.outputCommands = ["drop VectorFloat", "drop MCParticles1",
                      "drop SimTrackerHits", "drop TrackerHits",
                      "drop Tracks"]

transformer = ExampleFunctionalTransformerMultiple("ExampleFunctionalTransformerMultiple",
                                                   InputCollectionFloat="VectorFloat",
                                                   InputCollectionParticles="MCParticles1",
                                                   InputCollectionSimTrackerHits="SimTrackerHits",
                                                   InputCollectionTrackerHits="TrackerHits",
                                                   InputCollectionTracks="Tracks",
                                                   OutputCollectionCounter="Counter",
                                                   OutputCollectionParticles="NewMCParticles",
                                                   )

ApplicationMgr(TopAlg=[inp, transformer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
