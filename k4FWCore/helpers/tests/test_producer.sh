#!/usr/bin/env bash
# test_producer.sh — build-test: k4FWCore Producer (single output, one property)
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyProducer \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    -p 'int:ExampleInt:3:An example integer property'

echo "PASS: producer"
