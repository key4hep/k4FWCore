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

# Simple module that ensures that load_file injects the proper information

from k4FWCore.parseArgs import parser


parser.add_argument(
    "--with-error",
    action="store_true",
    default=False,
    help="Force a termination due to a syntax error",
)

args = parser.parse_known_args()[0]

if args.with_error:
    a = 32 / 0
else:
    assert __file__.endswith("checkLoadedFileProperties.py")
