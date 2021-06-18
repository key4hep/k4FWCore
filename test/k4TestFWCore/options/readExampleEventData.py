from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata.root"

from Configurables import PodioInput
inp = PodioInput()
inp.collections = ["MCParticles", "SimTrackerHits", "Tracks"]

from Configurables import PodioOutput
oup = PodioOutput()
oup.filename = "output_k4test_exampledata_2.root"
oup.outputCommands = ["drop MCParticles"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[inp, oup],
                EvtSel="NONE",
                EvtMax=100,
                ExtSvc=[podioevent],
                OutputLevel=DEBUG,
                )


