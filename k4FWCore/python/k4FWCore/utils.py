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
from typing import Union, Optional
import importlib.util
import importlib.abc
from importlib.machinery import SourceFileLoader, ModuleSpec


def import_from(filename: os.PathLike, module_name: Optional[str] = None) -> ModuleSpec:
    """Import a module by filename optionally giving it a module name

    The module name is what will be visible in error messages, e.g. when trying
    to get an attribute from the imported module

    Args:
        filename: file to load
        module_name: name of the module that should be used internally. If not
            provided this will be computed from the filename

    Return:
        The module imported from the provided file
    """
    filename = os.path.abspath(filename)
    module_name = module_name or os.path.basename(filename).replace(".", "_")
    loader = SourceFileLoader(module_name, filename)
    spec = importlib.util.spec_from_loader(loader.name, loader)
    module = importlib.util.module_from_spec(spec)
    loader.exec_module(module)
    return module


def load_file(opt_file: Union[TextIOWrapper, os.PathLike]) -> None:
    """Load the file content and run it in the current interpreter session"""
    if isinstance(opt_file, os.PathLike):
        opt_file = open(opt_file, "r")
    code = compile(opt_file.read(), opt_file.name, "exec")
    exec(code, globals())
