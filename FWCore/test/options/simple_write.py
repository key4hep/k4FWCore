
from Gaudi.Configuration import *
from Configurables import ApplicationMgr

app = ApplicationMgr()
app.EvtSel = "None"
app.EvtMax = 10;

from Configurables import K4DataSvc
ApplicationMgr().ExtSvc += [K4DataSvc("EventDataSvc")]

from Configurables import FWFloatProducer

fprod = FWFloatProducer()
fprod.FloatValue = 5.4321
fprod.FloatValueHandle.Path = "floatvalue"
ApplicationMgr().TopAlg += [fprod]

from Configurables import PodioOutput
out = PodioOutput()
out.filename = "output_fwtest1.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]
