#!/usr/bin/env bash
##
## Copyright (c) 2014-2024 Key4hep-Project.
##
## This file is part of Key4hep.
## See https://key4hep.github.io/key4hep-doc/ for further info.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

# _test_common.sh — sourced by every test_*.sh script.
# Provides: GENERATOR path, SANDBOX temp dir, and run_cmake_build().
#
# Usage in a test script:
#   source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"
#   run_cmake_build ClassName [generateFunctional args...]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Resolve the generator:
#   1. GENERATEFUNCTIONAL env var (set by CTest via CMakeLists.txt)
#   2. Installed command on PATH
#   3. Source copy one level above this directory
if [[ -n "${GENERATEFUNCTIONAL:-}" ]]; then
    GENERATOR="${GENERATEFUNCTIONAL}"
elif command -v generateFunctional &>/dev/null; then
    GENERATOR="$(command -v generateFunctional)"
else
    GENERATOR="${SCRIPT_DIR}/../generateFunctional"
fi

if [[ ! -f "${GENERATOR}" ]]; then
    echo "ERROR: generateFunctional not found (tried PATH and ${GENERATOR})" >&2
    exit 1
fi

SANDBOX="$(mktemp -d)"
trap 'rm -rf "${SANDBOX}"' EXIT

# run_cmake_build <ClassName> [generateFunctional args...]
#   1. Generates <ClassName>.cpp + CMakeLists.txt via generateFunctional --cmake
#   2. Configures with cmake
#   3. Builds with cmake --build
run_cmake_build() {
    local class="$1"; shift
    (
        cd "${SANDBOX}"
        python3 "${GENERATOR}" "${class}" "$@" --cmake --force
        cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build
    )
}
