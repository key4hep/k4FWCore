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
import logging
import sys
from typing import Union
from importlib.machinery import SourceFileLoader
import importlib.util
from pathlib import Path

import warnings


def check_wrong_imports(code: str) -> None:
    """Check for wrong imports in the given code.

    This function checks the given code for any imports of IOSvc or ApplicationMgr
    from Configurables instead of k4FWCore. If such an import is found, an ImportError
    is raised.

    If IOSvc and ApplicationMgr are not imported from k4FWCore it's possible
    that k4run will fail, for example, by not adding the reader and writer
    algorithms manually, which is done by the wrapper in k4FWCore. The checks
    are not comprehensive, it's still possible to import IOSvc or ApplicationMgr
    from Configurables.

    Args:
        code (str): The code to check for wrong imports.

    Raises:
        ImportError: If the code contains an import of IOSvc or ApplicationMgr from Configurables.

    """
    # Check first that IOSvc is being used, in that case
    # Importing either ApplicationMgr or IOSvc from Configurables is not allowed
    iosvc_regex = re.compile(
        r"^\s*from\s+(Configurables|k4FWCore)\s+import\s+\(?.*IOSvc.*\)?", re.MULTILINE
    )
    regex = re.compile(
        r"^\s*from\s+Configurables\s+import\s+\(?.*(ApplicationMgr|IOSvc).*\)?", re.MULTILINE
    )
    if re.search(iosvc_regex, code) and re.search(regex, code):
        raise ImportError("Importing ApplicationMgr or IOSvc from Configurables is not allowed.")


def load_file(opt_file: Union[str, os.PathLike]) -> None:
    """Loads and executes the content of a given file in the current interpreter session.

    This function takes a path to a file, reads its content, and then executes
    it as Python code within the global scope of the current interpreter
    session.

    Args:
        opt_file (Union[str, os.PathLike]): A file object or a path to the file
                                            that contains Python code to be
                                            executed.

    Raises:
        FileNotFoundError: If `opt_file` is a path and no file exists at that path.
        IOError: If there's an error opening or reading the file.
        SyntaxError: If there's a syntax error in the code being executed.
        Exception: Any exception raised by the executed code will be propagated.

    """
    # Cannot simply deepcopy globals. Hence, populate the necessary stuff
    namespace = {
        "__file__": __file__,
        "__builtins__": __builtins__,
        "__loader__": __loader__,
    }

    if isinstance(opt_file, (str, os.PathLike)):
        with open(opt_file, "r") as file:
            code = file.read()
            filename = file.name

        module_name = Path(opt_file).stem
        loader = SourceFileLoader(module_name, str(opt_file))

        namespace.update(
            {
                "__file__": os.path.realpath(opt_file),
                "__spec__": importlib.util.spec_from_loader(loader.name, loader),
            }
        )
    else:
        warnings.warn(
            "load_file will remove support for handling TextIOWrapper. Please switch to pasing os.PathLike",
            FutureWarning,
        )
        code = opt_file.read()
        filename = opt_file.name
        namespace.update({"__file__": filename})
    check_wrong_imports(str(code))
    code = compile(code, filename, "exec")

    exec(code, namespace)


_logger = None
logging.VERBOSE = 5
logging.addLevelName("VERBOSE", logging.VERBOSE)

LOG_LEVELS = ("VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR")


def set_log_level(level: str):
    """Set the passed log level to the k4run logger"""
    level_up = level.upper()
    if level_up not in LOG_LEVELS:
        raise ValueError(
            f"{level} is not a valid log level. Valid levels: {LOG_LEVELS} (and lowercase versions)"
        )
    logger = get_logger()
    logger.setLevel(getattr(logging, level_up))


def get_logger() -> logging.Logger:
    """Get the logger also used by k4run"""
    global _logger
    if _logger is not None:
        return _logger
    _logger = logging.getLogger()
    _logger.setLevel(logging.INFO)
    formatter = logging.Formatter("[k4run - %(levelname)s] %(module)s.%(funcName)s: %(message)s")
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(formatter)
    _logger.handlers = [handler]

    return _logger
