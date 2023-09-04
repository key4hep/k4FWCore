from Gaudi.Configuration import *
from Configurables import ExampleProducer
from Configurables import EvtStoreSvc
from Configurables import ApplicationMgr

producer = ExampleProducer("ExampleProducer", OutputLocation="/ExampleInt", ExampleInt=5)

ApplicationMgr( TopAlg=[producer],
                EvtSel="NONE",
                EvtMax=10,
                ExtSvc=[EvtStoreSvc("EventDataSvc")],
                OutputLevel=INFO,
                )
