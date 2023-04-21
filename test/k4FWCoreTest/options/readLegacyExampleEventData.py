from Gaudi.Configuration import *

from Configurables import k4LegacyDataSvc
podioevent = k4LegacyDataSvc("EventDataSvc")
podioevent.input = "output_k4test_legacyexampledata.root"

from Configurables import PodioLegacyInput
inp = PodioLegacyInput()
inp.collections = ["MCParticles", "SimTrackerHits", "TrackerHits", "Tracks"]

from Configurables import PodioLegacyOutput
oup = PodioLegacyOutput()
oup.filename = "output_k4test_legacyexampledata_2.root"
oup.outputCommands = ["drop MCParticles"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[inp, oup],
                EvtSel="NONE",
                EvtMax=10,
                ExtSvc=[podioevent],
                OutputLevel=DEBUG,
                )


