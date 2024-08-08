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
from Configurables import IOSvc as IO, MetadataSvc
import os


class IOSvc:
    def __init__(self, *args, **kwargs):
        self._svc = IO(**kwargs, ImportedFromk4FWCore=True)
        MetadataSvc("MetadataSvc")

    def __getattr__(self, attr):
        return getattr(self._svc, attr)

    def __setattr__(self, attr, value):
        if attr == "_svc":
            super().__setattr__(attr, value)
            return

        # Allow to specify a single string for input when what we want is a list
        if attr == "input":
            if isinstance(value, str):
                value = [value]
        if attr == "output":
            if os.path.dirname(value) and not os.path.exists(os.path.dirname(value)):
                os.makedirs(os.path.dirname(value))
        setattr(self._svc, attr, value)
