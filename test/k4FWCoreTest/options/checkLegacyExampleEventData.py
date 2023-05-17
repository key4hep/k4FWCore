#!/usr/bin/env python3
from Gaudi.Configuration import *

from Configurables import k4DataSvc

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_legacyexampledata.root"

from Configurables import PodioInput

inp = PodioInput()
inp.collections = [
    "VectorFloat",
    "MCParticles",
    "SimTrackerHits",
    "TrackerHits",
    "Tracks",
]

from Configurables import k4FWCoreTest_CheckExampleEventData

checker = k4FWCoreTest_CheckExampleEventData()

from Configurables import ApplicationMgr

ApplicationMgr(
    TopAlg=[inp, checker],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[podioevent],
    OutputLevel=INFO,
    StopOnSignal=True,
)
