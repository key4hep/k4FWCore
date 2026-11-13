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

"""Output directories are created when writing, not during configuration."""

from pathlib import Path
import subprocess
import sys
import tempfile

from podio.reading import get_reader


def run_job(work, output, mode, *, dry_run=False, expected_error=None):
    options = Path(__file__).resolve().parents[1] / "options/TestIOSvcOutputDirectory.py"
    command = [sys.argv[1], str(options), "--output", output, "--mode", mode]
    if dry_run:
        command.append("--dry-run")
    result = subprocess.run(
        command, cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    )
    if expected_error is not None:
        assert result.returncode != 0, result.stdout
        assert expected_error in result.stdout, result.stdout
    elif result.returncode:
        raise RuntimeError(result.stdout)


with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    cases = [
        ("assignment", "relative/nested/output.root", work / "relative/nested/output.root"),
        (
            "constructor",
            str(work / "absolute/nested/output.root"),
            work / "absolute/nested/output.root",
        ),
        (
            "constructor",
            (work / "url/nested/output.root").as_uri(),
            work / "url/nested/output.root",
        ),
        ("assignment", "output.root", work / "output.root"),
        ("constructor", "existing/output.root", work / "existing/output.root"),
    ]
    (work / "existing").mkdir()
    for mode, output, local_output in cases:
        parent_existed = local_output.parent.exists()
        run_job(work, output, mode, dry_run=True)
        assert local_output.parent.exists() == parent_existed
        assert not local_output.exists()
        run_job(work, output, mode)
        assert local_output.is_file()
        reader = get_reader(str(local_output))
        assert len(reader.get("events")) == 1
        del reader

    # Remote URLs must not become local directories, even during configuration.
    for mode in ["constructor", "assignment"]:
        run_job(work, "root://example.invalid//nested/output.root", mode, dry_run=True)
        assert not (work / "root:").exists()

    # A non-directory path component should produce a useful service error.
    (work / "blocked").write_text("not a directory")
    run_job(
        work,
        str(work / "blocked/nested/output.root"),
        "constructor",
        expected_error="Cannot create output directory",
    )
