from Gaudi.Configuration import INFO
from Gaudi import Configurables
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
# Use this to keep all the existing collections in the input file
out.outputCommands = ["drop MCParticles"]

transformer = ExampleFunctionalTransformer("ExampleFunctionalTransformer",
                                           InputLocation="MCParticles",
                                           OutputLocation="NewMCParticles")

ApplicationMgr(TopAlg=[inp, transformer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[k4DataSvc("EventDataSvc")],
               OutputLevel=INFO,
               )
