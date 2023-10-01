# This is an example using a producer with a multiple outputs and saving that to a file

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
                                             OutputCollectionParticles1="MCParticles1",
                                             OutputCollectionParticles2="MCParticles2",
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
