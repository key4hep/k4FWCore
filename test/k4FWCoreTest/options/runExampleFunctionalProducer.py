from Gaudi.Configuration import INFO
from Gaudi import Configurables
from Configurables import ExampleFunctionalProducer
from Configurables import ApplicationMgr
from Configurables import k4DataSvc

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata_producer.root"
out.outputCommands = ["keep *"]

producer = ExampleFunctionalProducer("ExampleFunctionalProducer",
                                     OutputLocation="MCParticles",
                                     ExampleInt=5)

ApplicationMgr(TopAlg=[producer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
