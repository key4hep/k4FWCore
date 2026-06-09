#!/usr/bin/env bash
# _test_common.sh — sourced by every test_*.sh script.
# Provides: GENERATOR path, SANDBOX temp dir, and run_cmake_build().
#
# Usage in a test script:
#   source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"
#   run_cmake_build ClassName [generateFunctional args...]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Prefer the installed command; fall back to the source copy one level up.
if command -v generateFunctional &>/dev/null; then
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
