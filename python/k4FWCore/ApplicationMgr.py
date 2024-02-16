#
# Copyright (c) 2014-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
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
