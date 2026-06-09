#!/usr/bin/env bash
# test_gaudi_framework.sh — build-test: native Gaudi::Functional Transformer with namespace
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyGaudiTransformer \
    -i 'edm4hep::MCParticleCollection:InputCollection' \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    --framework gaudi \
    --namespace MyNamespace \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: gaudi_framework"
