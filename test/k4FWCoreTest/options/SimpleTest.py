# Test that we can get reproducible random numbers in a multithreaded environment
# given that the seed is the same (obtained, for example, with the UniqueIDGenSvc)

from Configurables import AlgResourcePool, AvalancheSchedulerSvc
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard
from Configurables import ApplicationMgr, Gaudi__Sequencer
from Configurables import ExampleFunctionalProducer, ExampleFunctionalConsumer, Reader, IOSvc, Writer, EventDataSvc
from Configurables import ExampleFunctionalProducerMultiple, ExampleFunctionalConsumerMultiple
from Gaudi.Configuration import INFO, DEBUG, WARNING

evtslots = 1
threads = 1
# -------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

# whiteboard = HiveWhiteBoard("EventDataSvc",
#                             EventSlots=evtslots,
#                             ForceLeaves=True,
#                             )

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

# slimeventloopmgr = HiveSlimEventLoopMgr(
#     SchedulerName="AvalancheSchedulerSvc", OutputLevel=WARNING
# )

# AvalancheScheduler -----------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=DEBUG)

a1 = ExampleFunctionalProducer("FirstSingle")
a2 = ExampleFunctionalConsumer("ConsumerSingle")
b1 = ExampleFunctionalProducerMultiple("First")
b2 = ExampleFunctionalConsumerMultiple("Consumer")
# a2 = ExampleFunctionalProducer("Second", OutputCollection="MySecondCollection")
# a3 = ExampleFunctionalProducer("Third", OutputCollection="MyThirdCollection")

datasvc = EventDataSvc("EventDataSvc")


svc = IOSvc("IOSvc")
# svc.FileNames = ['/home/juanmi/Key4hep/Algorithm-validation/Digitisation/output_k4test_exampledata_producer.root']
svc.FileNames = ['/home/juanmi/Key4hep/Workarea/build/output_k4test_exampledata_producer.root']
svc.CollectionNames = ['MCParticles']

io = Reader("Reader",
           OutputLocations=['MCParticles'])
io.Cardinality = 1

save = Writer("Writer")
save.CollectionNames = ['MCParticles']
save.Cardinality = 1

# node = Gaudi__Sequencer("Node", Members=[io, a1], Sequential=True, OutputLevel=INFO)

app = ApplicationMgr(
    EvtMax=10,
    EvtSel="NONE",
    ExtSvc=[datasvc],
    TopAlg=[b1, b2],
    # TopAlg=[io, a2],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)


print(app)
