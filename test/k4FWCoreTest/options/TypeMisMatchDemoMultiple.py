#!/usr/bin/env python3


#!/usr/bin/env python3

from Gaudi.Configuration import INFO, DEBUG
from Configurables import ExampleFunctionalProducerMultiple, TypeMisMatchDemoMultiple, EventDataSvc

from k4FWCore import ApplicationMgr

producer = ExampleFunctionalProducerMultiple("Producer")

mismatch = TypeMisMatchDemoMultiple(InputCollections=["Tracks", "MCParticles1"], OutputLevel=DEBUG)

ApplicationMgr(
    TopAlg=[producer, mismatch],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
