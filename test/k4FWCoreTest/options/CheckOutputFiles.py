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


def check_collections(filename, names):
    print(f'Checking file "{filename}" for collections {names}')
    podio_reader = podio.root_io.Reader(filename)
    frames = podio_reader.get("events")
    if not len(frames) and len(names):
        print(f"File {filename} is empty but {names} are expected")
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
check_collections("/tmp/a/b/c/output_k4test_exampledata_producer.root", ["MCParticles"])
check_collections(
    "functional_transformer_runtime_empty.root", ["MCParticles0", "MCParticles1", "MCParticles2"]
)
