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
import os


class ApplicationMgr:
    def __init__(self, **kwargs):
        self._mgr = AppMgr(**kwargs)

        for conf in frozenset(self._mgr.allConfigurables.values()):
            if isinstance(conf, IOSvc):
                props = conf.getPropertiesWithDescription()
                reader = writer = None
                add_reader = add_writer = False
                for alg in self._mgr.TopAlg:
                    if isinstance(alg, Reader):
                        reader = alg
                    elif isinstance(alg, Writer):
                        writer = alg
                if reader is None and props["input"][0]:
                    reader = Reader("k4FWCore__Reader")
                    add_reader = True
                # It seems for a single string the default without a value is '<no value>'
                # while for a list it's an empty list
                if writer is None and props["output"][0] and props["output"][0] != "<no value>":
                    writer = Writer("k4FWCore__Writer")
                    add_writer = True
                # Let's tell the Reader one of the input files so it can
                # know which collections it's going to read
                if reader is not None:
                    if os.path.exists(props["input"][0][0]):
                        reader.Input = props["input"][0][0]
                    else:
                        reader.Input = os.getcwd() + "/" + props["input"][0][0]
                self._mgr.TopAlg = (
                    ([reader] if add_reader else [])
                    + self._mgr.TopAlg
                    + ([writer] if add_writer else [])
                )
