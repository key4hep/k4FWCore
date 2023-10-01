# This is an example reading from a file and using a consumer with several inputs
# to check that the contents of the file are the expected ones

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
    "MCParticles1",
    "MCParticles2",
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
