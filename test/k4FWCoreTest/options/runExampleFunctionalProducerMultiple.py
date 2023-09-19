from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalProducerMultiple
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput

podioevent = k4DataSvc("EventDataSvc")

out = PodioOutput("out")
out.filename = "output_k4test_exampledata_producer_multiple.root"
# Collections can be dropped
# out.outputCommands = ["drop *"]

producer = ExampleFunctionalProducerMultiple("ExampleFunctionalProducerMultiple",
                                             OutputCollectionFloat="VectorFloat",
                                             OutputCollectionParticles="MCParticles",
                                             OutputCollectionSimTrackerHits="SimTrackerHits",
                                             OutputCollectionTrackerHits="TrackerHits",
                                             OutputCollectionTracks="Tracks",
                                             ExampleInt=5)

ApplicationMgr(TopAlg=[producer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
