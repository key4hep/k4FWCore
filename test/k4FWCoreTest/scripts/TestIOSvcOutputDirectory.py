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

"""Output directories are created when IOSvc opens a writer."""

from pathlib import Path
import subprocess
import sys
import tempfile

from Configurables import ApplicationMgr, EventDataSvc, MetadataSvc
from Gaudi.Main import gaudimain
from k4FWCore import IOSvc
from podio.reading import get_reader


def run_job(work, output, *flags, expected_error=None):
    options = Path(__file__).resolve().parents[1] / "options/TestIOSvcOutputDirectory.py"
    command = [sys.argv[1], str(options), "--output", output, *flags]
    result = subprocess.run(
        command, cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    )
    if expected_error is not None:
        assert result.returncode != 0, result.stdout
        assert expected_error in result.stdout, result.stdout
    elif result.returncode:
        raise RuntimeError(result.stdout)


def run_without_writer(output):
    svc = IOSvc(Output=str(output))
    # The k4FWCore ApplicationMgr would schedule a Writer for this output path.
    ApplicationMgr(
        EvtSel="NONE", EvtMax=0, ExtSvc=[EventDataSvc(), MetadataSvc(), svc.getFullName()]
    )
    returncode = gaudimain().run(attach_debugger=False)
    assert returncode == 0, f"The job without a writer exited with status {returncode}"


with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    relative = "relative/nested/output.root"

    run_job(work, relative, "--dry-run")
    assert not (work / "relative").exists(), "Dry run created the output directory"

    url_output = work / "url/nested/output.root"
    for output, local_output in [(relative, work / relative), (url_output.as_uri(), url_output)]:
        run_job(work, output)
        assert local_output.is_file(), local_output
        assert len(get_reader(str(local_output)).get("events")) == 1, local_output

    # Check that Python configuration leaves remote URLs alone.
    run_job(work, "root://example.invalid//nested/output.root", "--dry-run")
    assert not (work / "root:").exists()

    # A non-directory path component should produce a useful service error.
    (work / "blocked").write_text("not a directory")
    run_job(
        work,
        str(work / "blocked/nested/output.root"),
        expected_error="Cannot create output directory",
    )

    unused = work / "unused"
    run_without_writer(unused / "nested/output.root")
    assert not unused.exists(), "IOSvc created the directory without a writer"
