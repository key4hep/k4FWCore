from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalConsumerMultiple
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioInput

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer_multiple.root"

inp = PodioInput()
inp.collections = [
    "VectorFloat",
    "MCParticles",
    "SimTrackerHits",
    "TrackerHits",
    "Tracks",
]

consumer = ExampleFunctionalConsumerMultiple("ExampleFunctionalConsumerMultiple",
                                     )

ApplicationMgr(TopAlg=[inp, consumer],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[podioevent],
               OutputLevel=INFO,
               )
