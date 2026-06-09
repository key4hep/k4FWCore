#!/usr/bin/env bash
# test_multitransformer.sh — build-test: k4FWCore MultiTransformer (multiple outputs, type aliases)
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyMultiTransformer \
    -i 'edm4hep::MCParticleCollection:InputParticles' \
       'edm4hep::SimTrackerHitCollection:InputHits' \
    -o 'edm4hep::MCParticleCollection:OutputParticles' \
       'podio::UserDataCollection<int>:Counter' \
    --type-aliases \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: multitransformer"
