from Gaudi.Configuration import *

from Configurables import k4LegacyDataSvc
podioevent = k4LegacyDataSvc("EventDataSvc")

from Configurables import k4FWCoreTest_CreateExampleEventData
producer = k4FWCoreTest_CreateExampleEventData()

from Configurables import PodioLegacyOutput
out = PodioLegacyOutput("out")
out.filename = "output_k4test_legacyexampledata.root"
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer, out],
                EvtSel="NONE",
                EvtMax=100,
                ExtSvc=[podioevent],
                OutputLevel=INFO,
                StopOnSignal=True,
                )


