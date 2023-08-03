from Gaudi.Configuration import *

from Configurables import EventHeaderFiller
eventHeaderFiller = EventHeaderFiller("eventHeaderFiller",
    runNumber = 42,
    eventNumberOffset = 42,
    OutputLevel=DEBUG)

from Configurables import ApplicationMgr
ApplicationMgr(
    TopAlg = [
              eventHeaderFiller,
              ],
    EvtSel = 'NONE',
    EvtMax   = 2,
    StopOnSignal = True)
