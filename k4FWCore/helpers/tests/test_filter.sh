#!/usr/bin/env bash
# test_filter.sh — build-test: k4FWCore FilterPredicate
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyFilter filter \
    -i 'edm4hep::MCParticleCollection:InputCollection'

echo "PASS: filter"
