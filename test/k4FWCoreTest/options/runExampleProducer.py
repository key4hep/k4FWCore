from Gaudi.Configuration import *
from Configurables import FunctionalProducer
from Configurables import EvtStoreSvc
from Configurables import ApplicationMgr

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata_producer.root"
out.outputCommands = ["keep *"]

producer = FunctionalProducer("FunctionalProducer",
                              OutputLocation="ExampleInt", ExampleInt=5)

ApplicationMgr(TopAlg=[producer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
