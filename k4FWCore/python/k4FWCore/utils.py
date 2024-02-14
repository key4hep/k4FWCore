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
from io import TextIOWrapper
from typing import Union, Optional, Dict, Any
import importlib.util
import importlib.abc
from importlib.machinery import SourceFileLoader


def import_from(
    filename: os.PathLike,
    module_name: Optional[str] = None,
    global_vars: Optional[Dict[str, Any]] = None,
) -> Any:
    """Dynamically imports a module from the specified file path.

    This function imports a module from a given filename, with the option to
    specify the module's name and inject global variables into the module before
    it is returned. If `module_name` is not provided, the filename is used as
    the module name after replacing '.' with '_'. Global variables can be passed
    as a dictionary to `global_vars`, which will be injected into the module's
    namespace.

    Args:
        filename (str): The path to the file from which to import the module.
        module_name (Optional[str]): The name to assign to the module. Defaults
                                     to None, in which case the filename is used
                                     as the module name.
        global_vars (Optional[Dict[str, Any]]): A dictionary of global variables
                                                to inject into the module's
                                                namespace. Defaults to None.

    Returns:
        Any: The imported module with the specified modifications.

    Raises:
        FileNotFoundError: If the specified file does not exist.
        ImportError: If there is an error during the import process.

    """
    filename = os.path.abspath(filename)
    module_name = module_name or os.path.basename(filename).replace(".", "_")
    loader = SourceFileLoader(module_name, filename)
    spec = importlib.util.spec_from_loader(loader.name, loader)
    module = importlib.util.module_from_spec(spec)
    if global_vars:
        module.__dict__.update(global_vars)
    loader.exec_module(module)
    return module


def load_file(opt_file: Union[TextIOWrapper, os.PathLike]) -> None:
    """Load the file content and run it in the current interpreter session"""
    if isinstance(opt_file, os.PathLike):
        opt_file = open(opt_file, "r")
    code = compile(opt_file.read(), opt_file.name, "exec")
    exec(code, globals())
