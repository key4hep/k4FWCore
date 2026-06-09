#!/usr/bin/env bash
# run_all_tests.sh — run every test_*.sh script and report results.
# Requires a Key4hep / k4FWCore environment (k4FWCore, EDM4HEP, Gaudi on
# CMAKE_PREFIX_PATH). Source the Key4hep setup script before running:
#
#   source /cvmfs/sw.hsf.org/key4hep/setup.sh
#   bash k4FWCore/helpers/tests/run_all_tests.sh

set -uo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PASS=0
FAIL=0
FAILED_TESTS=()

for test_script in "${TESTS_DIR}"/test_*.sh; do
    name="$(basename "${test_script}" .sh)"
    printf "  %-35s" "${name} ..."
    if output="$(bash "${test_script}" 2>&1)"; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        echo "${output}" | sed 's/^/    /'
        FAIL=$((FAIL + 1))
        FAILED_TESTS+=("${name}")
    fi
done

echo ""
echo "Results: ${PASS} passed, ${FAIL} failed"

if [[ ${FAIL} -gt 0 ]]; then
    echo "Failed tests:"
    for t in "${FAILED_TESTS[@]}"; do
        echo "  - ${t}"
    done
    exit 1
fi
