from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import TestAlgorithmWithTFile
producer = TestAlgorithmWithTFile()

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_algorithmWithTFile.root"
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer, out],
                EvtSel="NONE",
                EvtMax=5000000,
                ExtSvc=[podioevent],
                OutputLevel=INFO,
                StopOnSignal=True,
                )


