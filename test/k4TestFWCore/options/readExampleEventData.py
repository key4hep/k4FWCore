from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata.root"
from Configurables import CreateExampleEventData
producer = CreateExampleEventData()

from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = ["MCParticles", "SimTrackerHit"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[inp],
                EvtSel="NONE",
                EvtMax=100,
                ExtSvc=[podioevent],
                OutputLevel=DEBUG,
                )


