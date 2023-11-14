# Test that we can get reproducible random numbers in a multithreaded environment
# given that the seed is the same (obtained, for example, with the UniqueIDGenSvc)

from Configurables import AlgResourcePool, AvalancheSchedulerSvc
from Configurables import HiveSlimEventLoopMgr, HiveWhiteBoard
from Configurables import ApplicationMgr, Gaudi__Sequencer
from Configurables import ExampleFunctionalProducer, ExampleFunctionalConsumer, Reader, IOSvc, SaveToFile, StoreSnifferAlg
from Gaudi.Configuration import INFO, DEBUG, WARNING

evtslots = 2
threads = 2
# -------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            ForceLeaves=True,
                            )

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=WARNING
)

# AvalancheScheduler -----------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=DEBUG)

a1 = ExampleFunctionalProducer("First")
a2 = ExampleFunctionalConsumer("Consumer")
# a2 = ExampleFunctionalProducer("Second", OutputCollection="MySecondCollection")
# a3 = ExampleFunctionalProducer("Third", OutputCollection="MyThirdCollection")


svc = IOSvc("IOSvc")
svc.FileNames = ['/home/juanmi/Key4hep/Algorithm-validation/Digitisation/sim1.root']
svc.CollectionNames = ['MCParticle', 'EventHeader']

io = Reader("Reader",
           OutputLocations=['MCParticles', 'EventHeader'])
io.Cardinality = 1

save = SaveToFile("SaveToFile")
save.CollectionNames = ['MCParticles', 'EventHeader']
save.Cardinality = 1

node = Gaudi__Sequencer("Node", Members=[io, save], Sequential=True, OutputLevel=INFO)

app = ApplicationMgr(
    EvtMax=2,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[node],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)


print(app)
