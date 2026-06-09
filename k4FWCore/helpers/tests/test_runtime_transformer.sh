#!/usr/bin/env bash
# test_runtime_transformer.sh — build-test: k4FWCore Transformer with runtime outputs
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyRuntimeTransformer \
    -i 'edm4hep::MCParticleCollection:InputCollections' \
    --runtime-inputs 'edm4hep::MCParticleCollection:InputCollections:MCParticles' \
    --runtime-outputs 'edm4hep::MCParticleCollection' \
    --private-properties \
    -p 'int:NumCollections:3:Number of output collections'

echo "PASS: runtime_transformer"
