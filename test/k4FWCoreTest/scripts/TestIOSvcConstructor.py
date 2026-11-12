# Copyright (c) 2026 Key4hep-Project.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0

"""Constructor keywords must use the wrapper's property normalization."""

from pathlib import Path
import tempfile

from k4FWCore import IOSvc

with tempfile.TemporaryDirectory() as directory:
    output = Path(directory) / "nested" / "output.root"
    svc = IOSvc(Input="input.root", Output=str(output))
    assert svc.Input == ["input.root"]
    assert svc.Output == str(output)
    assert output.parent.is_dir()
