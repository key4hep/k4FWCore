from Gaudi.Configuration import *

from Configurables import EventHeaderCreator
eventHeaderCreator = EventHeaderCreator("eventHeaderCreator",
    runNumber = 42,
    eventNumberOffset = 42,
    OutputLevel=DEBUG)

from Configurables import ApplicationMgr
ApplicationMgr(
    TopAlg = [
              eventHeaderCreator,
              ],
    EvtSel = 'NONE',
    EvtMax   = 2,
    StopOnSignal = True)
