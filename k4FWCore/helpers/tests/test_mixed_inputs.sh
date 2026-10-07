#!/usr/bin/env bash
##
## Copyright (c) 2014-2024 Key4hep-Project.
##
## This file is part of Key4hep.
## See https://key4hep.github.io/key4hep-doc/ for further info.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

# test_mixed_inputs.sh — build-test: k4FWCore Transformer that mixes a fixed
# (scalar) input with a runtime (variable-length) input in the same algorithm.
# The scalar input keeps its key out of --runtime-inputs; the runtime input is
# named in both -i and --runtime-inputs so it is promoted to a KeyValues vector.
source "$(dirname "${BASH_SOURCE[0]}")/_test_common.sh"

run_cmake_build MyMixedInputs \
    -i 'edm4hep::MCParticleCollection:Fixed' 'edm4hep::TrackCollection:RuntimeTracks' \
    --runtime-inputs 'edm4hep::TrackCollection:RuntimeTracks:Tracks0,Tracks1' \
    -o 'edm4hep::MCParticleCollection:Output'

echo "PASS: mixed_inputs"
