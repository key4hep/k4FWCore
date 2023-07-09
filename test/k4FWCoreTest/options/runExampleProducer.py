from Gaudi.Configuration import *
from Configurables import ExampleProducer
from Configurables import EvtStoreSvc
from Configurables import ApplicationMgr

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata_twoproducer.root"
out.outputCommands = ["keep *"]

producer = ExampleProducer("ExampleProducer", OutputLocation="/ExampleInt", ExampleInt=5)

ApplicationMgr( TopAlg=[producer, out],
                EvtSel="NONE",
                EvtMax=10,
                ExtSvc=[k4DataSvc("EventDataSvc")],
                OutputLevel=INFO,
                )
