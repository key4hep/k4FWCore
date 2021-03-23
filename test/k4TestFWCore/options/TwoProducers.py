from Gaudi.Configuration import *

from Configurables import ApplicationMgr
ApplicationMgr(
                EvtSel="NONE",
                EvtMax=100,
                OutputLevel=INFO,
                StopOnSignal=True,
                )


from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

from Configurables import CreateExampleEventData
producer1 = CreateExampleEventData()
ApplicationMgr().TopAlg += [producer1]


from Configurables import CreateExampleEventData
producer2 = CreateExampleEventData("Producer2")
producer2.mcparticles.Path = "mcparticles2"
producer2.trackhits.Path = "trackhits2"
producer2.tracks.Path = "tracks2"
producer2.singleint.Path = "singleint2"
producer2.singlefloat.Path = "singlefloat2"
producer2.vectorfloat.Path = "vectorfloat2"
ApplicationMgr().TopAlg += [producer2]

from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "output_k4test_exampledata_twoproducer.root"
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [producer2]



