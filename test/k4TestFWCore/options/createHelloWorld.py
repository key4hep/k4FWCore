from Gaudi.Configuration import *

from Configurables import HelloWorldAlg
producer = HelloWorldAlg()
producer.PerEventPrintMessage = "Hello World !"

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer],
                EvtSel="NONE",
                EvtMax=1,                
                OutputLevel=INFO,
                )

