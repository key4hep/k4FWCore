#!/usr/bin/env bash
# test_runtime_consumer.sh — build-test: k4FWCore Consumer with runtime (variable-length) inputs
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyRuntimeConsumer \
    -i 'edm4hep::MCParticleCollection:InputCollections' \
    --runtime-inputs 'edm4hep::MCParticleCollection:InputCollections:MCParticles0,MCParticles1' \
    --private-properties \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: runtime_consumer"
