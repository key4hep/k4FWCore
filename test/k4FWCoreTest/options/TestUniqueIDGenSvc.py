from Gaudi.Configuration import *

from Configurables import HepRndm__Engine_CLHEP__RanluxEngine_ as RndmEngine
rndmengine = RndmEngine('RndmGenSvc.Engine')
rndmengine.SetSingleton = True
rndmengine.Seeds = [ 4072012 ]

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
