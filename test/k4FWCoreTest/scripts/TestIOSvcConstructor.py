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

"""Constructor names must be forwarded and properties normalized by the wrapper."""

from pathlib import Path
import tempfile

from k4FWCore import IOSvc

for args, kwargs, expected_name in [
    ((), {}, "IOSvc"),
    (("PositionalIO",), {}, "PositionalIO"),
    ((), {"name": "KeywordIO"}, "KeywordIO"),
]:
    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "nested" / "output.root"
        svc = IOSvc(*args, **kwargs, Input="input.root", Output=str(output))
        assert svc.getName() == expected_name
        assert svc.Input == ["input.root"]
        assert svc.Output == str(output)
        # Configuring an output path must not touch the filesystem.
        assert not output.parent.exists()
