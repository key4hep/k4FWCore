import os
from Gaudi.Configuration import *

# Workflow Steering
from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 100
ApplicationMgr().StopOnSignal = True

## Data event model based on Podio
from Configurables import K4DataSvc
podioEvent = K4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioEvent]
ApplicationMgr().OutputLevel = INFO

from Configurables import ParticleGunPtEta
gun = ParticleGunPtEta()
ApplicationMgr().TopAlg += [gun]


## Delphes simulator -> define objects to be written out
from Configurables import DelphesSimulation
delphessim = DelphesSimulation()
## Define Delphes card
delphessim.DelphesCard = "Sim/SimDelphes/data/FCChh_DelphesCard_Baseline_v01.tcl"
ApplicationMgr().TopAlg += [delphessim]



## FCC event-data model output -> define objects to be written out
from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "DelphesOutput.root"
out.outputCommands = [
                       "keep *", 
                     ]
ApplicationMgr().TopAlg += [out]

