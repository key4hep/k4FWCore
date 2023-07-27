from Gaudi.Configuration import INFO
from Gaudi import Configurables
from Configurables import ExampleFunctionalConsumer
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioInput

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer.root"

from Configurables import PodioInput

inp = PodioInput()
inp.collections = [
    "ExampleInt",
]

consumer = ExampleFunctionalConsumer("ExampleFunctionalConsumer",
                                     InputLocation="/Event/ExampleInt",
                                     )

ApplicationMgr(TopAlg=[inp, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[podioevent],
               OutputLevel=INFO,
               )
