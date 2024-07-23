#
# Copyright (c) 2014-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
try:
    import podio
except ImportError:
    import os

    print(f'PYTHONPATH={os.environ["PYTHONPATH"]}')
    raise
import ROOT


def check_collections(filename, names):
    print(f'Checking file "{filename}" for collections {names}')
    podio_reader = podio.root_io.Reader(filename)
    if "events" not in podio_reader.categories:
        raise RuntimeError(f"File {filename} has no events")
    frames = podio_reader.get("events")
    if not len(frames) and len(names):
        print(f"File {filename} is empty but {names} are expected")
        # Prevent a possible crash
        del podio_reader
        raise RuntimeError("File is empty but should not be")
    for frame in frames:
        available = set(frame.getAvailableCollections())
        if available != set(names):
            print(
                f"These collections should be in the frame but are not: {set(names) - available}"
            )
            print(
                f"These collections are in the frame but should not be: {available - set(names)}"
            )
            raise RuntimeError("Collections in frame do not match expected collections")


def check_events(filename, number):
    print(f'Checking file "{filename}" for {number} events')
    podio_reader = podio.root_io.Reader(filename)
    frames = podio_reader.get("events")
    if len(frames) != number:
        print(f"File {filename} has {len(frames)} events but {number} are expected")
        raise RuntimeError("Number of events does not match expected number")


check_collections("functional_transformer.root", ["MCParticles", "NewMCParticles"])
check_collections(
    "functional_transformer_multiple.root",
    [
        "VectorFloat",
        "MCParticles1",
        "MCParticles2",
        "SimTrackerHits",
        "TrackerHits",
        "Tracks",
        "Counter",
        "NewMCParticles",
    ],
)
check_collections(
    "functional_transformer_multiple_output_commands.root",
    ["VectorFloat", "MCParticles1", "MCParticles2", "SimTrackerHits", "TrackerHits"],
)
check_collections("/tmp/a/b/c/functional_producer.root", ["MCParticles"])
check_collections(
    "functional_transformer_runtime_empty.root",
    ["MCParticles0", "MCParticles1", "MCParticles2"],
)
check_collections(
    "functional_transformerMT.root",
    [
        "VectorFloat",
        "MCParticles1",
        "MCParticles2",
        "SimTrackerHits",
        "TrackerHits",
        "Tracks",
        "NewMCParticles",
    ],
)


mix_collections = [
    # From file
    "VectorFloat",
    "MCParticles1",
    "MCParticles2",
    "SimTrackerHits",
    "TrackerHits",
    "Tracks",
    # Produced by functional
    "FunctionalVectorFloat",
    "FunctionalMCParticles",
    "FunctionalMCParticles2",
    "FunctionalSimTrackerHits",
    "FunctionalTrackerHits",
    "FunctionalTracks",
    # Produced by an old algorithm
    "OldAlgorithmMCParticles",
    "OldAlgorithmSimTrackerHits",
    "OldAlgorithmTrackerHits",
    "OldAlgorithmTracks",
    "OldAlgorithmVectorFloat",
    # Produced by the last transformer
    "Counter",
    "TransformedFunctionalMCParticles1",
]


# Not working, collections produced by functional algorithms are not being written to the file
# check_collections(
#     "functional_mix.root",
#     mix_collections,
# )

check_collections(
    "functional_mix_iosvc.root",
    mix_collections,
)

f = ROOT.TFile.Open("functional_transformer_hist.root")
for i in range(2):
    if (
        str(f.GetListOfKeys()[i])
        != f"Name: ExampleFunctionalTransformerHist{i+1} Title: ExampleFunctionalTransformerHist{i+1}"
    ):
        raise RuntimeError(
            "Directory structure does not match expected for functional_transformer_hist.root"
        )

check_collections(
    "functional_merged_collections.root",
    ["MCParticles1", "MCParticles2", "MCParticles3", "NewMCParticles", "SimTrackerHits"],
)

podio_reader = podio.root_io.Reader("functional_merged_collections.root")
frames = podio_reader.get("events")
ev = frames[0]
if len(ev.get("NewMCParticles")) != 4:
    raise RuntimeError(f"Expected 4 NewMCParticles but got {len(ev.get('NewMCParticles'))}")

check_events(
    "functional_filter.root",
    5,
)

check_collections("functional_metadata.root", ["MCParticles"])

reader = podio.root_io.Reader("functional_metadata.root")
metadata = reader.get("metadata")[0]
for key, value in zip(
    [
        "NumberOfParticles",
        "ParticleTime",
        "PDGValues",
        "MetadataString",
        "EventMetadataInt",
        "FinalizeMetadataInt",
    ],
    [3, 1.5, [1, 2, 3, 4], "hello", 5, 10],
):
    if metadata.get_parameter(key) != value:
        raise RuntimeError(f"Metadata parameter {key} does not match expected value")
