from Gaudi.Configuration import *
from GaudiKernel import SystemOfUnits as units

from Configurables import ApplicationMgr
ApplicationMgr(
               EvtSel='NONE',
               EvtMax=100,
               OutputLevel=INFO,
              )

from Configurables import K4DataSvc
podioevent = K4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]



from Configurables import ParticleGunPtEta
gun = ParticleGunPtEta()
gun.MCParticles.Path = "MCParticles"
ApplicationMgr().TopAlg += [gun]



from Configurables import PodioOutput
out = PodioOutput("out", filename = "output_particlegun.root")
out.outputCommands = ["keep *"]



