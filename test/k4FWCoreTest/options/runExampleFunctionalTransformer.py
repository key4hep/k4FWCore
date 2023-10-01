# This is an example reading from a file and using a consumer with several inputs
# to check that the contents of the file are the expected ones

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalTransformer
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput
from Configurables import PodioInput

podioevent = k4DataSvc("EventDataSvc")
podioevent.input = "output_k4test_exampledata_producer.root"

inp = PodioInput()
inp.collections = [
    "MCParticles",
]

out = PodioOutput("out")
out.filename = "output_k4test_exampledata_transformer.root"
# The collections that we don't drop will also be present in the output file
out.outputCommands = ["drop MCParticles"]

transformer = ExampleFunctionalTransformer("ExampleFunctionalTransformer",
                                           InputCollection="MCParticles",
                                           OutputCollection="NewMCParticles")

ApplicationMgr(TopAlg=[inp, transformer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
