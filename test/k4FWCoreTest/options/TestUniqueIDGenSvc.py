from Gaudi.Configuration import *

from Configurables import UniqueIDGenSvc
UniqueIDGenSvc().Seed = 987

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = "NONE"
ApplicationMgr().EvtMax = 5
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().StopOnSignal = True

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import TestUniqueIDGenSvc
uniqueidtest = TestUniqueIDGenSvc()
ApplicationMgr().TopAlg += [uniqueidtest]
