from Gaudi.Configuration import *

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")

from Configurables import k4FWCoreTest_cellID_writer, k4FWCoreTest_cellID_reader
producer = k4FWCoreTest_cellID_writer()
consumer = k4FWCoreTest_cellID_reader()

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata_cellid.root"
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg=[producer, consumer, out],
                EvtSel="NONE",
                EvtMax=10,
                ExtSvc=[podioevent],
                OutputLevel=INFO,
                StopOnSignal=True,
                )


