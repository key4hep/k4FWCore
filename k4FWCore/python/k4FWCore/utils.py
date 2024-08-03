#!/usr/bin/env python3
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
import re
from io import TextIOWrapper
from typing import Union


def check_wrong_imports(code: str) -> None:
    """Check for wrong imports in the given code.

    This function checks the given code for any imports of IOSvc or ApplicationMgr
    from Configurables instead of k4FWCore. If such an import is found, an ImportError
    is raised.

    Args:
        code (str): The code to check for wrong imports.

    Raises:
        R

    """
    iosvc_regex = re.compile(
        r"^\s*from\s+(Configurables|k4FWCore)\s+import\s+\(?.*IOSvc.*\)?", re.MULTILINE
    )
    regex = re.compile(
        r"^\s*from\s+Configurables\s+import\s+\(?.*(ApplicationMgr|IOSvc).*\)?", re.MULTILINE
    )
    if re.search(iosvc_regex, code) and re.search(regex, code):
        raise ImportError("Importing ApplicationMgr or IOSvc from Configurables is not allowed.")


def load_file(opt_file: Union[TextIOWrapper, str, os.PathLike]) -> None:
    """Loads and executes the content of a given file in the current interpreter session.

    This function takes a file object or a path to a file, reads its content,
    and then executes it as Python code within the global scope of the current
    interpreter session. If `opt_file` is a file handle it will not be closed.

    Args:
        opt_file (Union[TextIOWrapper, str, os.PathLike]): A file object or a
                                                           path to the file that
                                                           contains Python code
                                                           to be executed.

    Raises:
        FileNotFoundError: If `opt_file` is a path and no file exists at that path.
        IOError: If there's an error opening or reading the file.
        SyntaxError: If there's a syntax error in the code being executed.
        Exception: Any exception raised by the executed code will be propagated.

    """
    if isinstance(opt_file, (str, os.PathLike)):
        with open(opt_file, "r") as file:
            code = file.read()
    else:
        code = opt_file.read()
    check_wrong_imports(str(code))

    exec(code, globals())
