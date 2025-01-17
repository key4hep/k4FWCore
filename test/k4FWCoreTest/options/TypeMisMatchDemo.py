#!/usr/bin/env python3

from Gaudi.Configuration import INFO, DEBUG
from Configurables import ExampleFunctionalProducer, TypeMisMatchDemo, EventDataSvc

from k4FWCore import ApplicationMgr

producer = ExampleFunctionalProducer("Producer", OutputCollection=["MCParticles"])

mismatch = TypeMisMatchDemo(InputCollection=["MCParticles"], OutputLevel=DEBUG)

ApplicationMgr(
    TopAlg=[producer, mismatch],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
