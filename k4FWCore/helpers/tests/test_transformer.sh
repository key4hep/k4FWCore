#!/usr/bin/env bash
# test_transformer.sh — build-test: k4FWCore Transformer (single in/out, private property)
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyTransformer \
    -i 'edm4hep::MCParticleCollection:InputCollection' \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    --private-properties \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: transformer"
