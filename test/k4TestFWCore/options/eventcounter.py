from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import EventCounterExample
eventcounter = EventCounterExample()


from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[eventcounter],
                EvtSel="NONE",
                EvtMax=100,
                ExtSvc=[podioevent],
                OutputLevel=DEBUG,
                )


