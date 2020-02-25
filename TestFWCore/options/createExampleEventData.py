from Gaudi.Configuration import *

from Configurables import K4DataSvc
podioevent = K4DataSvc("EventDataSvc")

from Configurables import CreateExampleEventData
producer = CreateExampleEventData()

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata.root"
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer, out],
                EvtSel="NONE",
                EvtMax=100,
                ExtSvc=[podioevent],
                OutputLevel=INFO,
                )


