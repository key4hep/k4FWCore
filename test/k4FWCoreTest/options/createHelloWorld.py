from Gaudi.Configuration import *

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = "NONE"
ApplicationMgr().EvtMax = 1
ApplicationMgr().OutputLevel = INFO

from Configurables import k4FWCoreTest_HelloWorldAlg
producer = k4FWCoreTest_HelloWorldAlg("HelloWorldAlg1")
producer.PerEventPrintMessage = "Hello World !"
ApplicationMgr().TopAlg += [producer]

from Configurables import k4FWCoreTest_HelloWorldAlg
producer2 = k4FWCoreTest_HelloWorldAlg("HelloWorldAlg2")
producer2.PerEventPrintMessage = "Hello World2 !"
ApplicationMgr().TopAlg += [producer2]


