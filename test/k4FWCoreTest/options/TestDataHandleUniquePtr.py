from Gaudi.Configuration import *

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = "NONE"
ApplicationMgr().EvtMax = 100
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().StopOnSignal = True

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import TestDataHandleUniquePtr
producer = TestDataHandleUniquePtr()
ApplicationMgr().TopAlg += [producer]

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_TestDataHandleUniquePtr.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]



