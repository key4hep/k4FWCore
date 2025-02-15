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
import os
from Configurables import ApplicationMgr as AppMgr
from Configurables import Reader, Writer, IOSvc, Gaudi__Sequencer, EventLoopMgr


class ApplicationMgr:
    """ApplicationMgr is a class that wraps the Gaudi ApplicationMgr class to

    - Give the reader the collections it's going to read so that
      the scheduler can know which algorithms can't run until the
      collections are available
    - When running multithreaded and EvtMax is -1, set the number of events to
      be run to the number of events in the file so that no more events than
      necessary are scheduled
    - Wrap inside a sequencer the set of algorithms and a Writer (if any) so that
      when running multithreaded the writer runs after the algorithms
    """

    def __init__(self, **kwargs):
        self._mgr = AppMgr(**kwargs)

    def fix_properties(self):
        # If there isn't an EventLoopMgr then it's the default
        # This will suppress two warnings about not using external input
        try:
            self._mgr.EventLoop
        except AttributeError:
            self._mgr.EventLoop = EventLoopMgr(Warnings=False)

        if "MetadataSvc" in self._mgr.allConfigurables:
            self._mgr.ExtSvc.append(self._mgr.allConfigurables["MetadataSvc"])

        if "IOSvc" not in self._mgr.allConfigurables:
            return
        if not isinstance(self._mgr.allConfigurables["IOSvc"], IOSvc):
            raise TypeError("The IOSvc is not an instance of IOSvc")
        conf = self._mgr.allConfigurables["IOSvc"]

        props = conf.getPropertiesWithDescription()
        reader = writer = None
        add_reader = False
        for alg in self._mgr.TopAlg:
            if isinstance(alg, Reader):
                reader = alg
            elif isinstance(alg, Writer):
                writer = alg
        # Remove "input" when the corresponding property is removed from IOSvc
        if reader is None and (props["input"][0] or props["Input"][0]):
            reader = Reader("k4FWCore__Reader")
            add_reader = True
        # It seems for a single string the default without a value is '<no value>'
        # while for a list it's an empty list
        # Remove "output" when the corresponding property is removed from IOSvc
        if (
            writer is None
            and (props["output"][0] and props["output"][0] != "<no value>")
            or (props["Output"][0] and props["Output"][0] != "<no value>")
        ):
            writer = Writer("k4FWCore__Writer")
        # Let's tell the Reader one of the input files so it can
        # know which collections it's going to read
        if reader is not None:
            # Open the files and get the number of events. This is necessary to
            # avoid errors when running multithreaded since if we have, for
            # example, 10 events and we are running 9 at the same time, then
            # (possibly) the first 9 complete and 9 more are scheduled, out of
            # which only one will be finished without errors. If we know the
            # number of events in advance then we can just schedule those.
            inp = None
            if props["input"][0]:
                inp = "input"
            elif props["Input"][0]:
                inp = "Input"
            if inp:
                # Import here to avoid always importing ROOT which is slow
                from podio.root_io import Reader as PodioReader

                podio_reader = PodioReader(props[inp][0])
                if self._mgr.EvtMax == -1:
                    self._mgr.EvtMax = podio_reader._reader.getEntries("events")
                try:
                    frame = podio_reader.get("events")[0]
                except IndexError:
                    print("Warning, the events category wasn't found in the input file")
                    raise
                collections = list(frame.getAvailableCollections())
                reader.InputCollections = collections
        self._mgr.TopAlg = ([reader] if add_reader else []) + self._mgr.TopAlg
        # Assume the writer is at the end
        # Algorithms are wrapped with Sequential=False so that they can run in parallel
        # The algorithms and Writer are wrapped with Sequential=True so that they can not
        # run in parallel
        if writer:
            self._mgr.TopAlg = [
                Gaudi__Sequencer(
                    "k4FWCore__Sequencer",
                    Sequential=True,
                    Members=[
                        Gaudi__Sequencer(
                            "k4FWCore__Algs",
                            Members=self._mgr.TopAlg,
                            Sequential=False,
                            ShortCircuit=False,
                        ),
                        writer,
                    ],
                )
            ]

    def __getattr__(self, name):
        return getattr(self._mgr, name)

    def __setattr__(self, name, value):
        if name == "_mgr":
            super().__setattr__(name, value)
        else:
            setattr(self._mgr, name, value)
