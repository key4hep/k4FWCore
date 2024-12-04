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


def check_metadata(filename, expected_metadata):
    print(f'Checking file "{filename}" for metadata')
    podio_reader = podio.root_io.Reader(filename)
    metadata = podio_reader.get("metadata")[0]
    for key, value in expected_metadata.items():
        if (metaval := metadata.get_parameter(key)) != value:
            raise RuntimeError(
                f"Metadata parameter {key} does not match the expected value, got {metaval} but expected {value}"
            )


check_collections("functional_transformer.root", ["MCParticles", "NewMCParticles"])
check_collections("functional_transformer_cli.root", ["MCParticles", "NewMCParticles"])
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

check_collections("functional_metadata.root", ["MCParticles"])

check_metadata(
    "functional_metadata.root",
    {
        "NumberOfParticles": 3,
        "ParticleTime": 1.5,
        "PDGValues": [1, 2, 3, 4],
        "MetadataString": "hello",
        "FinalizeMetadataInt": 10,
    },
)

reader = podio.root_io.Reader("functional_metadata.root")
metadata = reader.get("metadata")[0]
for key, value in zip(
    [
        "NumberOfParticles",
        "ParticleTime",
        "PDGValues",
        "MetadataString",
        "FinalizeMetadataInt",
    ],
    [3, 1.5, [1, 2, 3, 4], "hello", 10],
):
    if metadata.get_parameter(key) != value:
        raise RuntimeError(
            f"Metadata parameter {key} does not match the expected value, got {metadata.get_parameter(key)} but expected {value}"
        )
for rntuple in ["functional_producer_rntuple.root", "functional_producer_rntuple_converted.root"]:
    reader = podio.root_io.RNTupleReader(f"{rntuple}")
    frames = podio_reader.get("events")
    if len(frames) != 10:
        raise RuntimeError(f"Expected 10 events but got {len(frames)}")

check_metadata(
    "functional_metadata_propagate.root",
    {
        "NumberOfParticles": 3,
        "ParticleTime": 1.5,
        "PDGValues": [1, 2, 3, 4],
        "MetadataString": "hello",
        "FinalizeMetadataInt": 10,
    },
)

check_metadata(
    "functional_metadata_old_algorithm.root",
    {
        "SimTrackerHits__CellIDEncoding": "M:3,S-1:3,I:9,J:9,K-1:6",
    },
)

check_metadata(
    "functional_metadata_old_algorithm_propagate.root",
    {
        "SimTrackerHits__CellIDEncoding": "M:3,S-1:3,I:9,J:9,K-1:6",
    },
)

reader = podio.root_io.Reader("eventHeaderConcurrent.root")
events = reader.get("events")
expected_events_length = 10
expected_run_number = 42
expected_event_numbers = set(range(42, 42 + expected_events_length))
seen_event_numbers = set()
if len(events) != expected_events_length:
    raise RuntimeError("Number of events does not match expected number")
for frame in events:
    event_header = frame.get("EventHeader")[0]
    if (run_number := event_header.getRunNumber()) != expected_run_number:
        raise RuntimeError(
            f"Run number is not set correctly (expected {expected_run_number}, actual {run_number})"
        )
    event_number = event_header.getEventNumber()
    if event_number not in expected_event_numbers:
        raise RuntimeError(f"Event number {event_number} is not in expected numbers")
    if event_number in seen_event_numbers:
        raise RuntimeError(f"Event number {event_number} is duplicated")
    seen_event_numbers.add(event_number)

for name, events in {
    "functional_filter.root": 5,
    "functional_nth_event.root": 3,
    "two_events.root": 2,
    "functional_transformer_cli_multiple.root": 20,
}.items():
    check_events(name, events)


for i, filename in enumerate(
    [
        "output_k4test_exampledata_cellid.root",
        "functional_metadata_old_algorithm.root",
        "functional_metadata.root",
    ]
):
    reader = podio.root_io.Reader(filename)
    configuration_metadata = reader.get("configuration_metadata")[0].get_parameter(
        "gaudiConfigOptions"
    )
    configuration_metadata = [elem.strip(" ,;\n") for elem in configuration_metadata]
    configuration_metadata = {
        elem.split("=")[0]: elem.split("=")[1] for elem in configuration_metadata
    }

    props_and_values = {
        "intProp": '"42"',
        "intProp2": '"69"',
        "floatProp": '"3.14000"',
        "floatProp2": '"2.71828"',
        "doubleProp": '"3.1400000"',
        "doubleProp2": '"2.7182818"',
        "stringProp": "\"'Hello'\"",
        "stringProp2": "\"'Hello, World!'\"",
        "vectorIntProp": '"[ 1 , 2 , 3 ]"',
        "vectorIntProp2": '"[ 1 , 2 , 3 , 4 ]"',
        "vectorFloatProp": '"[ 1.10000 , 2.20000 , 3.30000 ]"',
        "vectorFloatProp2": '"[ 1.10000 , 2.20000 , 3.30000 , 4.40000 ]"',
        "vectorDoubleProp": '"[ 1.1000000 , 2.2000000 , 3.3000000 ]"',
        "vectorDoubleProp2": '"[ 1.1000000 , 2.2000000 , 3.3000000 , 4.4000000 ]"',
        "vectorStringProp": "\"[ 'one' , 'two' , 'three' ]\"",
        "vectorStringProp2": "\"[ 'one' , 'two' , 'three' , 'four' ]\"",
    }

    alg_name = "CellIDWriter" if i < 2 else "Producer"
    for prop, value in props_and_values.items():
        print(prop, value)
        if configuration_metadata[f"{alg_name}.{prop} "] != f" {value}":
            raise RuntimeError(
                f"Property {prop} has value {configuration_metadata[f'CellIDWriter.{prop} ']}, expected {value}"
            )
