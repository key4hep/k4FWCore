
from Gaudi.Configuration import *
from Configurables import ApplicationMgr

app = ApplicationMgr()
app.EvtSel = "NONE"
app.EvtMax = 10;

from Configurables import k4DataSvc
dataservice = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [dataservice]

from Configurables import k4TestFloatProducer
fprod = k4TestFloatProducer()
fprod.FloatValue = 5.4321
fprod.Float.Path = "floatvalue"
ApplicationMgr().TopAlg += [fprod]

from Configurables import PodioOutput
out = PodioOutput()
out.filename = "output_fwtest1.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]
