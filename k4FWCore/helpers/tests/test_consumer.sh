#!/usr/bin/env bash
# test_consumer.sh — build-test: k4FWCore Consumer (single input, one property)
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyConsumer \
    -i 'edm4hep::MCParticleCollection:InputCollection' \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: consumer"
