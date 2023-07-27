from Gaudi.Configuration import *
from Configurables import FunctionalConsumer
from Configurables import EvtStoreSvc
from Configurables import ApplicationMgr

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer.root"

from Configurables import PodioInput

inp = PodioInput()
inp.collections = [
    "ExampleInt",
]

consumer = FunctionalConsumer("FunctionalConsumer",
                              InputLocation="/Event/ExampleInt",
                              )

ApplicationMgr(TopAlg=[inp, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[podioevent],
               OutputLevel=INFO,
               )
