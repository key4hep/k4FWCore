from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata.root"
podioevent.FirstEventEntry = 66

from Configurables import PodioInput
inp = PodioInput()
inp.collections = ["MCParticles", "SimTrackerHits", "Tracks"]

from Configurables import PodioOutput
oup = PodioOutput()
oup.filename = "output_k4test_exampledata_3.root"

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[inp, oup],
                EvtSel="NONE",
                EvtMax=5,
                ExtSvc=[podioevent],
                OutputLevel=DEBUG,
                )


