#!/usr/bin/env bash
# test_event_context.sh — build-test: k4FWCore Transformer with EventContext and finalize()
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyEventContextTransformer \
    -i 'edm4hep::MCParticleCollection:InputCollection' \
    -o 'edm4hep::MCParticleCollection:OutputCollection' \
    --event-context \
    --private-properties \
    -p 'int:Offset:10:Integer to add to values'

echo "PASS: event_context"
