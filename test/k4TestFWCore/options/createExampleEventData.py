from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import CreateExampleEventData
producer = CreateExampleEventData()

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata.root"
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer, out],
                EvtSel="NONE",
                EvtMax=1000000,
                ExtSvc=[podioevent],
                OutputLevel=INFO,
                StopOnSignal=True,
                )


