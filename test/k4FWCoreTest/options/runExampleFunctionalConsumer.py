from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalConsumer
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioInput

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer.root"

inp = PodioInput()
inp.collections = [
    "MCParticles",
]

consumer = ExampleFunctionalConsumer("ExampleFunctionalConsumer",
                                     InputCollection="MCParticles",
                                     )

ApplicationMgr(TopAlg=[inp, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[podioevent],
               OutputLevel=INFO,
               )
