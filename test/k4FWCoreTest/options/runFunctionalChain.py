# This is an example of how to run several functional algorithms in a chain
# The producer produces a collection, the first consumer checks that the collection
# is the expected one, the transformer transforms the collection and creates a new
# and the second consumer checks that the new collection is the expected one

from Gaudi.Configuration import INFO
from Configurables import ExampleFunctionalProducer
from Configurables import ExampleFunctionalConsumer
from Configurables import ExampleFunctionalTransformer
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput

event_data_svc = k4DataSvc("EventDataSvc")

out = PodioOutput("out")
out.filename = "functional_chain.root"

producer = ExampleFunctionalProducer("ExampleFunctionalProducer")
consumer = ExampleFunctionalConsumer("ExampleFunctionalConsumer",
                                     InputCollection="MCParticles",
                                     )
transformer = ExampleFunctionalTransformer("ExampleFunctionalTransformer",
                                           InputCollection="MCParticles",
                                           OutputCollection="NewMCParticles")
new_consumer = ExampleFunctionalConsumer("ExampleFunctionalConsumer2",
                                         InputCollection="NewMCParticles",
                                         )
new_consumer.PossibleOffset = 10

ApplicationMgr(TopAlg=[producer, consumer, transformer, new_consumer, out],
               EvtSel="NONE",
               EvtMax=10,
               ExtSvc=[event_data_svc],
               OutputLevel=INFO,
               )
