# Copyright (c) 2026 Key4hep-Project.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0

"""Constructor and assignment forms must normalize IOSvc properties identically."""

from pathlib import Path
import subprocess
import sys
import tempfile

from k4FWCore import IOSvc
from podio.reading import get_reader

# Positional service names must also be forwarded rather than silently discarded.
assert IOSvc("NamedIO").getName() == "NamedIO"

input_file = Path("functional_producer.root").resolve()
with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    collections = []
    for mode in ["constructor", "assignment"]:
        output = work / mode / "nested" / "output.root"
        assert not output.parent.exists()
        if mode == "constructor":
            configure = f"svc = IOSvc('IOSvc', Input={str(input_file)!r}, Output={str(output)!r})"
        else:
            configure = f"""
svc = IOSvc('IOSvc')
svc.Input = {str(input_file)!r}
svc.Output = {str(output)!r}
"""
        options = work / f"{mode}.py"
        options.write_text(f"""
from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc
{configure}
assert svc.Input == [{str(input_file)!r}]
ApplicationMgr(TopAlg=[], EvtSel='NONE', EvtMax=1, ExtSvc=[EventDataSvc()])
""")
        subprocess.run([sys.argv[1], str(options)], check=True)
        reader = get_reader(str(output))
        frames = reader.get("events")
        assert len(frames) == 1
        frame = frames[0]
        collections.append(sorted(frame.getAvailableCollections()))
    assert collections[0] == collections[1]
