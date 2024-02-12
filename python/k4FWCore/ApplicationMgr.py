from Configurables import ApplicationMgr as AppMgr
from Configurables import Reader, Writer, IOSvc

# seq = Gaudi__Sequencer("Node", Members=[reader, transformer, writer], Sequential=True, OutputLevel=INFO)

class ApplicationMgr:

    def __init__(self, **kwargs):
        self._mgr = AppMgr(**kwargs)

        for conf in frozenset(self._mgr.allConfigurables.values()):
            if isinstance(conf, IOSvc):
                props = conf.getPropertiesWithDescription()
                if 'input' in props:
                    self._mgr.TopAlg =  [Reader("Reader")] + self._mgr.TopAlg
                if 'output' in props:
                    self._mgr.TopAlg = self._mgr.TopAlg + [Writer("Writer")]
