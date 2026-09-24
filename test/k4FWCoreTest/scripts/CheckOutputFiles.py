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

    print(f"PYTHONPATH={os.environ['PYTHONPATH']}")
    raise
import ROOT

from collections import Counter

from CreateOverlayBackgroundFiles import (
    GROUP_A,
    GROUP_B,
    HIT_MOMENTUM,
    N_FILES_GROUP_A,
    N_FILES_GROUP_B,
    background_tag,
    particle_momentum,
)


def check_collections(filename, names):
    print(f'Checking file "{filename}" for collections {names}')
    podio_reader = podio.reading.get_reader(filename)
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
    podio_reader = podio.reading.get_reader(filename)
    frames = podio_reader.get("events")
    if len(frames) != number:
        print(f"File {filename} has {len(frames)} events but {number} are expected")
        raise RuntimeError("Number of events does not match expected number")


def check_metadata(filename, expected_metadata):
    print(f'Checking file "{filename}" for metadata')
    podio_reader = podio.reading.get_reader(filename)
    metadata = podio_reader.get("metadata")[0]
    for key, value in expected_metadata.items():
        if (metaval := metadata.get_parameter(key)) != value:
            raise RuntimeError(
                f"Metadata parameter {key} does not match the expected value, got {metaval} but expected {value}"
            )


functional_transformer_collections = ["EventHeader", "MCParticles", "NewMCParticles"]

functional_transformer_multiple_collections = [
    "VectorFloat",
    "MCParticles1",
    "MCParticles2",
    "SimTrackerHits",
    "SimTrackerHitsWithoutParticleRelations",
    "SimCalorimeterHits",
    "CaloHitContributions",
    "SimCalorimeterHitsWithoutParticleRelations",
    "CaloHitContributionsWithoutParticleRelations",
    "TrackerHits",
    "Tracks",
    "Counter",
    "NewMCParticles",
    "RecoParticles",
    "Links",
    "NewLinks",
]
functional_transformer_multiple_output_commands_collections = [
    collection
    for collection in functional_transformer_multiple_collections
    if collection not in {"Tracks", "Counter", "RecoParticles", "NewLinks"}
]
functional_transformer_mt_collections = [
    collection
    for collection in functional_transformer_multiple_collections
    if collection not in {"Counter", "NewLinks"}
]
functional_limited_input_collections = ["MCParticles", "Links"]

for filename in [
    "functional_transformer.root",
    "gaudi_functional.root",
    "functional_transformer_cli.root",
]:
    check_collections(filename, functional_transformer_collections)

check_collections(
    "functional_transformer_multiple.root",
    functional_transformer_multiple_collections,
)
check_collections(
    "functional_transformer_multiple_output_commands.root",
    functional_transformer_multiple_output_commands_collections,
)
check_collections("/tmp/a/b/c/functional_producer.root", ["MCParticles"])
check_collections(
    "functional_transformer_runtime_empty.root",
    ["MCParticles0", "MCParticles1", "MCParticles2"],
)
check_collections(
    "functional_transformerMT.root",
    functional_transformer_mt_collections,
)

for filename in [
    "functional_limited_input.root",
    "functional_limited_input_all_events.root",
]:
    check_collections(filename, functional_limited_input_collections)

mix_collections = [
    # From file
    "VectorFloat",
    "MCParticles1",
    "MCParticles2",
    "SimTrackerHits",
    "SimTrackerHitsWithoutParticleRelations",
    "SimCalorimeterHits",
    "CaloHitContributions",
    "SimCalorimeterHitsWithoutParticleRelations",
    "CaloHitContributionsWithoutParticleRelations",
    "TrackerHits",
    "Tracks",
    "RecoParticles",
    "Links",
    # Produced by functional
    "FunctionalVectorFloat",
    "FunctionalMCParticles",
    "FunctionalMCParticles2",
    "FunctionalSimTrackerHits",
    "FunctionalSimTrackerHitsWithoutParticleRelations",
    "FunctionalSimCalorimeterHits",
    "FunctionalCaloHitContributions",
    "FunctionalSimCalorimeterHitsWithoutParticleRelations",
    "FunctionalCaloHitContributionsWithoutParticleRelations",
    "FunctionalTrackerHits",
    "FunctionalTracks",
    "FunctionalRecos",
    "FunctionalLinks",
    # Produced by an old algorithm
    "OldAlgorithmMCParticles",
    "OldAlgorithmSimTrackerHits",
    "OldAlgorithmTrackerHits",
    "OldAlgorithmTracks",
    "OldAlgorithmVectorFloat",
    "OldAlgorithmRecoParticles",
    "OldAlgorithmLinks",
    # Produced by the last transformer
    "Counter",
    "TransformedFunctionalMCParticles1",
    "NewLinks",
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
        != f"Name: ExampleFunctionalTransformerHist{i + 1} Title: ExampleFunctionalTransformerHist{i + 1}"
    ):
        raise RuntimeError(
            "Directory structure does not match expected for functional_transformer_hist.root"
        )

check_collections(
    "functional_merged_collections.root",
    [
        "MCParticles1",
        "MCParticles2",
        "MCParticles3",
        "NewMCParticles",
        "SimTrackerHits",
        "Links",
        "NewLinks",
    ],
)

podio_reader = podio.reading.get_reader("functional_merged_collections.root")
frames = podio_reader.get("events")
ev = frames[0]
new_mcs = ev.get("NewMCParticles")
merged_mc_colls = [ev.get(f"MCParticles{i}") for i in range(1, 4)]
merged_mcs = [mcc[i] for mcc in merged_mc_colls for i in range(len(mcc))]
if len(new_mcs) != len(merged_mcs):
    raise RuntimeError(f"Expected {len(merged_mcs)} NewMCParticles but got {len(new_mcs)}")
for new_mc, orig_mc in zip(new_mcs, merged_mcs):
    if new_mc.id() != orig_mc.id():
        raise RuntimeError(
            f"merged mcs do not match, expected [{new_mc.id().collectionID}, {new_mc.id().index}], actual [{orig_mc.id().collectionID}, {orig_mc.id().index}]"
        )
links = ev.get("Links")
merged_links = ev.get("NewLinks")
if len(links) * 2 != len(merged_links):
    raise RuntimeError(f"Expected {len(links)} NewLinks but got {len(merged_links)}")
for i in range(len(merged_links)):
    link = links[i % len(links)]
    merged_link = merged_links[i]
    if link.id() != merged_link.id():
        raise RuntimeError(
            f"merged links do not match, expected [{link.id().collectionID}, {link.id().index}], actual [{merged_link.id().collectionID}, {merged_link.id().index}]"
        )


check_collections("functional_metadata.root", ["MCParticles", "MCParticles2"])

check_metadata(
    "functional_metadata.root",
    {
        "NumberOfParticles": 3,
        "ParticleTime": 1.5,
        "PDGValues": [1, 2, 3, 4],
        "MetadataString": "hello",
        "FinalizeMetadataInt": 10,
        "PythonIntParam": 42,
        "PythonFloatParam": 4.0,
        "PythonDoubleParam": 3.14,
        "PythonStringParam": "hello from python",
        "PythonFinalizeParam": 99,
    },
)


for rntuple in [
    "functional_producer_rntuple.root",
    "functional_producer_rntuple_file.root",
    "functional_producer_rntuple_converted.root",
]:
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

reader = podio.reading.get_reader("eventHeaderConcurrent.root")
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
    reader = podio.reading.get_reader(filename)
    configuration_metadata = reader.get("configuration_metadata")[0].get_parameter(
        "gaudiConfigOptions"
    )
    configuration_metadata = [elem.strip(" ,;\n") for elem in configuration_metadata]
    # Split each element into key and value, remove the surrounding quotes from the value
    configuration_metadata = {
        elem.split("=")[0]: elem.split("=")[1][2:-1] for elem in configuration_metadata
    }

    props_and_values = {
        "intProp": 42,
        "intProp2": 69,
        "floatProp": 3.14,
        "floatProp2": 2.71828,
        "doubleProp": 3.14,
        "doubleProp2": 2.7182818,
        "stringProp": "Hello",
        "stringProp2": "Hello, World!",
        "vectorIntProp": [1, 2, 3],
        "vectorIntProp2": [1, 2, 3, 4],
        "vectorFloatProp": [1.1, 2.2, 3.3],
        "vectorFloatProp2": [1.1, 2.2, 3.3, 4.4],
        "vectorDoubleProp": [1.1, 2.2, 3.3],
        "vectorDoubleProp2": [1.1, 2.2, 3.3, 4.4],
        "vectorStringProp": ["one", "two", "three"],
        "vectorStringProp2": ["one", "two", "three", "four"],
    }

    alg_name = "CellIDWriter" if i < 2 else "Producer"
    for prop, value in props_and_values.items():
        print(prop, value)
        if eval(configuration_metadata[f"{alg_name}.{prop} "]) != value:
            raise RuntimeError(
                f"Property {prop} has value {configuration_metadata[f'CellIDWriter.{prop} ']}, expected {value} (after eval)"
            )


check_collections(
    "overlay_output.root",
    [
        "EventHeader",
        "MCParticles1",
        "MCParticles2",
        "VectorFloat",
        "SimTrackerHits",
        "SimTrackerHitsWithoutParticleRelations",
        "TrackerHits",
        "Tracks",
        "RecoParticles",
        "Links",
        "OverlayMCParticles",
        "OverlaySimTrackerHits",
        "OverlaySimTrackerHitsWithoutParticleRelations",
        "OverlayCaloHitContributions",
        "OverlayCaloHitContributionsWithoutParticleRelations",
        "OverlaySimCalorimeterHits",
        "OverlaySimCalorimeterHitsWithoutParticleRelations",
        "SimCalorimeterHits",
        "CaloHitContributions",
        "SimCalorimeterHitsWithoutParticleRelations",
        "CaloHitContributionsWithoutParticleRelations",
    ],
)


def mc_relation_indices(particle):
    """Indices of the parents and daughters of an MCParticle, as sorted lists."""
    return (
        sorted(p.getObjectID().index for p in particle.getParents()),
        sorted(d.getObjectID().index for d in particle.getDaughters()),
    )


reader = podio.reading.get_reader("overlay_output.root")
n_signal_mc = 2
n_background_mc = 2
n_signal_sim = 1
n_background_sim = 1
for frame in reader.get("events"):
    overlaid_particles = frame.get("OverlayMCParticles")
    if len(overlaid_particles) != n_signal_mc + n_background_mc:
        raise RuntimeError(
            f"Expected {n_signal_mc + n_background_mc} particles in OverlayMCParticles, got {len(overlaid_particles)}"
        )
    particle_overlay_flags = [
        overlaid_particles[i].isOverlay() for i in range(len(overlaid_particles))
    ]
    if particle_overlay_flags[:n_signal_mc] != [False] * n_signal_mc:
        raise RuntimeError("Signal particles should not be flagged as overlay")
    if particle_overlay_flags[n_signal_mc:] != [True] * n_background_mc:
        raise RuntimeError("Background particles should be flagged as overlay")

    # The producer makes part2 a daughter of part1 without filling in part2's own
    # parent list. Both blocks, signal and overlaid background, have to come out
    # with that same one-sided topology, with the background relation pointing
    # inside the background block only.
    for block_start in (0, n_signal_mc):
        relations = {i: mc_relation_indices(overlaid_particles[block_start + i]) for i in range(2)}
        expected = {
            0: ([], [block_start + 1]),
            1: ([], []),
        }
        if relations != expected:
            raise RuntimeError(
                f"Unexpected MCParticle relations in block starting at {block_start}: "
                f"got {relations}, expected {expected}"
            )

    overlaid_sim_hits = frame.get("OverlaySimTrackerHits")
    if len(overlaid_sim_hits) != n_signal_sim + n_background_sim:
        raise RuntimeError(
            f"Expected {n_signal_sim + n_background_sim} hits in OverlaySimTrackerHits, got {len(overlaid_sim_hits)}"
        )
    sim_hit_overlay_flags = [
        overlaid_sim_hits[i].isOverlay() for i in range(len(overlaid_sim_hits))
    ]
    if sim_hit_overlay_flags[:n_signal_sim] != [False] * n_signal_sim:
        raise RuntimeError("Signal sim tracker hits should not be flagged as overlay")
    if sim_hit_overlay_flags[n_signal_sim:] != [True] * n_background_sim:
        raise RuntimeError("Background sim tracker hits should be flagged as overlay")


def check_random_mix_overlay(filename):
    """Checks an output of the OverlayTimingRandomMix configuration and returns,
    for every event, the background tags in the order they were drawn."""
    print(f'Checking file "{filename}" for the random mix of background files')
    n_bx = 6
    # (group, number of files, number of draws per event) in the order of BackgroundFileNames
    groups = [
        (GROUP_A, N_FILES_GROUP_A, n_bx * 3),
        (GROUP_B, N_FILES_GROUP_B + 1, n_bx * 1),
    ]
    n_draws = sum(n_group_draws for _, _, n_group_draws in groups)
    n_signal_mc = 2
    n_signal_sim = 1
    n_signal_calo = 3

    draws_per_event = []
    group_a_reshuffled = False
    for frame in podio.reading.get_reader(filename).get("events"):
        # The background hits are appended in the order they are drawn, one hit
        # per background event, after the signal hit
        hits = frame.get("OverlaySimTrackerHits")
        if len(hits) != n_signal_sim + n_draws:
            raise RuntimeError(
                f"Expected {n_signal_sim + n_draws} hits in OverlaySimTrackerHits, got {len(hits)}"
            )
        if hits[0].isOverlay() or hits[0].getParticle().getObjectID().index != 0:
            raise RuntimeError("The signal hit should come first and point to the signal particle")
        background_hits = [hits[i] for i in range(n_signal_sim, len(hits))]
        tags = [hit.getCellID() for hit in background_hits]
        draws_per_event.append(tags)

        # Each group only draws from its own files, and only from the .root
        # files directly inside its directories. Files are drawn from a
        # shuffled list that is reshuffled once exhausted, so every full pass
        # over a group uses each of its files exactly once.
        start = 0
        for group, n_files, n_group_draws in groups:
            files = [tag - background_tag(group, 0) for tag in tags[start : start + n_group_draws]]
            start += n_group_draws
            if any(not 0 <= f < n_files for f in files):
                raise RuntimeError(
                    f"Group {group} drew files outside its {n_files} files: {files}"
                )
            passes = [files[p : p + n_files] for p in range(0, n_group_draws, n_files)]
            for draw_pass in passes:
                if len(set(draw_pass)) != len(draw_pass) or (
                    len(draw_pass) == n_files and sorted(draw_pass) != list(range(n_files))
                ):
                    raise RuntimeError(
                        f"Group {group} did not draw each file once per pass: {passes}"
                    )
            if group == GROUP_A and passes[0] != passes[1]:
                group_a_reshuffled = True

        # The background particles are appended in the same order, as a parent
        # and its daughter per background event. The hit has to point to the
        # daughter copied for the same draw.
        particles = frame.get("OverlayMCParticles")
        if len(particles) != n_signal_mc + 2 * n_draws:
            raise RuntimeError(
                f"Expected {n_signal_mc + 2 * n_draws} particles in OverlayMCParticles, got {len(particles)}"
            )
        if any(particles[i].isOverlay() for i in range(n_signal_mc)):
            raise RuntimeError("Signal particles should not be flagged as overlay")
        for draw, (hit, tag) in enumerate(zip(background_hits, tags)):
            parent_index = n_signal_mc + 2 * draw
            parent = particles[parent_index]
            daughter = particles[parent_index + 1]
            if (parent.getPDG(), daughter.getPDG()) != (tag, -tag) or not (
                parent.isOverlay() and daughter.isOverlay()
            ):
                raise RuntimeError(f"Unexpected background particles for draw {draw} of tag {tag}")
            relations = (mc_relation_indices(parent), mc_relation_indices(daughter))
            expected = (([], [parent_index + 1]), ([parent_index], []))
            if relations != expected:
                raise RuntimeError(
                    f"Unexpected MCParticle relations for draw {draw}: got {relations}, expected {expected}"
                )
            momentum = daughter.getMomentum()
            if (momentum.x, momentum.y, momentum.z) != particle_momentum(tag):
                raise RuntimeError(f"Unexpected momentum of the background particle of tag {tag}")

            if not hit.isOverlay() or hit.getEDep() != tag:
                raise RuntimeError(f"Unexpected background hit for draw {draw} of tag {tag}")
            if hit.getParticle().getObjectID().index != parent_index + 1:
                raise RuntimeError(
                    f"The hit of draw {draw} points to particle {hit.getParticle().getObjectID().index}, "
                    f"expected {parent_index + 1}"
                )
            # The particles are merged, so the hit keeps its own momentum
            momentum = hit.getMomentum()
            if (momentum.x, momentum.y, momentum.z) != HIT_MOMENTUM:
                raise RuntimeError(f"The momentum of the hit of draw {draw} was changed")

        # Background calorimeter hits are merged by cellID, so a file drawn
        # several times gives one hit with a contribution per draw, each
        # pointing to the daughter copied for that draw
        draws_per_tag = Counter(tags)
        calo_hits = frame.get("OverlaySimCalorimeterHits")
        if len(calo_hits) != n_signal_calo + len(draws_per_tag):
            raise RuntimeError(
                f"Expected {n_signal_calo + len(draws_per_tag)} hits in OverlaySimCalorimeterHits, got {len(calo_hits)}"
            )
        contributions = frame.get("OverlayCaloHitContributions")
        if len(contributions) != n_signal_calo + n_draws:
            raise RuntimeError(
                f"Expected {n_signal_calo + n_draws} contributions in OverlayCaloHitContributions, got {len(contributions)}"
            )
        background_calo_hits = {hit.getCellID(): hit for hit in calo_hits if hit.getCellID() > 3}
        if set(background_calo_hits) != set(draws_per_tag):
            raise RuntimeError(
                f"Background calorimeter cells {sorted(background_calo_hits)} do not match the drawn files {sorted(draws_per_tag)}"
            )
        for tag, calo_hit in background_calo_hits.items():
            contribs = calo_hit.getContributions()
            if any(c.getPDG() != tag for c in contribs):
                raise RuntimeError(f"Unexpected contributions in the calorimeter hit of tag {tag}")
            indices = sorted(c.getParticle().getObjectID().index for c in contribs)
            expected = [n_signal_mc + 2 * d + 1 for d, t in enumerate(tags) if t == tag]
            if indices != expected:
                raise RuntimeError(
                    f"The contributions of tag {tag} point to particles {indices}, expected {expected}"
                )

    # Without the reshuffle every pass of an event would replay the same order
    if not group_a_reshuffled:
        raise RuntimeError("The files were not reshuffled once all of them had been drawn")
    if all(tags == draws_per_event[0] for tags in draws_per_event):
        raise RuntimeError("Every event overlaid the same sequence of background files")
    return draws_per_event


random_mix_draws = {}
for filename in (
    "overlay_random_mix.root",
    "overlay_random_mix_repeat.root",
    "overlay_random_mix_other_seed.root",
):
    check_events(filename, 3)
    random_mix_draws[filename] = check_random_mix_overlay(filename)

# The draws only depend on the seed of UniqueIDGenSvc, the event and run
# numbers and the algorithm name, so rerunning the same configuration has to
# overlay the same files in the same order, and a different seed must not
if (
    random_mix_draws["overlay_random_mix_repeat.root"]
    != random_mix_draws["overlay_random_mix.root"]
):
    raise RuntimeError("Rerunning with the same seed overlaid a different sequence of files")
if (
    random_mix_draws["overlay_random_mix_other_seed.root"]
    == random_mix_draws["overlay_random_mix.root"]
):
    raise RuntimeError("Running with a different seed overlaid the same sequence of files")


def check_no_mcparticle_merge_overlay(filename):
    """Checks the output of OverlayTimingNoMCParticleMerge, where the background
    particles are left out and the background hits keep everything else."""
    print(f'Checking file "{filename}" for background hits without background particles')
    n_bx = 6
    # (group, number of files, number of draws per bunch crossing) in the order of BackgroundFileNames
    groups = [(GROUP_A, N_FILES_GROUP_A, 3), (GROUP_B, N_FILES_GROUP_B + 1, 1)]
    n_draws = n_bx * sum(n_per_bx for _, _, n_per_bx in groups)
    n_signal_calo = 3
    # The background hits have a time of 1 ns, shifted by 0.5 ns per bunch crossing
    bx_times = [1.0 + 0.5 * bx for bx in range(n_bx)]

    for frame in podio.reading.get_reader(filename).get("events"):
        particles = frame.get("OverlayMCParticles")
        if len(particles) != 2 or any(particle.isOverlay() for particle in particles):
            raise RuntimeError(
                f"Only the 2 signal particles should be in OverlayMCParticles, got {len(particles)}"
            )
        if mc_relation_indices(particles[0]) != ([], [1]):
            raise RuntimeError("The relations between the signal particles were not kept")

        hits = frame.get("OverlaySimTrackerHits")
        if len(hits) != 1 + n_draws:
            raise RuntimeError(
                f"Expected {1 + n_draws} hits in OverlaySimTrackerHits, got {len(hits)}"
            )
        if hits[0].isOverlay() or hits[0].getParticle().getObjectID().index != 0:
            raise RuntimeError("The signal hit should still point to the signal particle")
        start = 1
        for group, n_files, n_per_bx in groups:
            group_hits = [hits[i] for i in range(start, start + n_bx * n_per_bx)]
            start += n_bx * n_per_bx
            for hit in group_hits:
                tag = hit.getCellID()
                if not 0 <= tag - background_tag(group, 0) < n_files:
                    raise RuntimeError(f"Group {group} overlaid a hit of tag {tag}")
                if not hit.isOverlay() or hit.getParticle().isAvailable():
                    raise RuntimeError(
                        f"The hit of tag {tag} should be flagged as overlay and have no particle"
                    )
                momentum = hit.getMomentum()
                if (momentum.x, momentum.y, momentum.z) != particle_momentum(tag):
                    raise RuntimeError(
                        f"The hit of tag {tag} should carry the momentum of its particle "
                        f"{particle_momentum(tag)}, got {(momentum.x, momentum.y, momentum.z)}"
                    )
                position = hit.getPosition()
                # Bit 31 of the quality holds the overlay flag
                if (
                    hit.getEDep(),
                    hit.getPathLength(),
                    hit.getQuality() & 0x7FFFFFFF,
                    (position.x, position.y, position.z),
                ) != (float(tag), 2.0, 3, (3.0, 4.0, 5.0)):
                    raise RuntimeError(f"The hit of tag {tag} was not copied faithfully")
            # The events of one bunch crossing share its time shift, and every
            # bunch crossing of the train is used once
            times = [group_hits[bx * n_per_bx].getTime() for bx in range(n_bx)]
            if sorted(times) != bx_times or any(
                hit.getTime() != times[bx]
                for bx in range(n_bx)
                for hit in group_hits[bx * n_per_bx : (bx + 1) * n_per_bx]
            ):
                raise RuntimeError(
                    f"Unexpected times of the hits of group {group}: "
                    f"{[hit.getTime() for hit in group_hits]}"
                )

        contributions = frame.get("OverlayCaloHitContributions")
        if len(contributions) != n_signal_calo + n_draws:
            raise RuntimeError(
                f"Expected {n_signal_calo + n_draws} contributions in OverlayCaloHitContributions, got {len(contributions)}"
            )
        for calo_hit in frame.get("OverlaySimCalorimeterHits"):
            tag = calo_hit.getCellID()
            if tag <= n_signal_calo:
                if any(
                    c.getParticle().getObjectID().index != 0 for c in calo_hit.getContributions()
                ):
                    raise RuntimeError(
                        "The signal contributions should still point to the signal particle"
                    )
                continue
            for contrib in calo_hit.getContributions():
                position = contrib.getStepPosition()
                if (
                    contrib.getParticle().isAvailable()
                    or (
                        contrib.getPDG(),
                        contrib.getEnergy(),
                        (position.x, position.y, position.z),
                    )
                    != (tag, float(tag), (3.0, 4.0, 5.0))
                    or contrib.getTime() not in bx_times
                ):
                    raise RuntimeError(
                        f"The contributions of tag {tag} should have no particle and keep everything else"
                    )


check_events("overlay_no_mcparticle_merge.root", 3)
check_no_mcparticle_merge_overlay("overlay_no_mcparticle_merge.root")

reader = podio.reading.get_reader("functional_random_filter.root")
frames = reader.get("events")
for frame in frames:
    filtered = frame.get("FilteredMCParticles")
    if len(filtered) != 16:
        raise RuntimeError(f"Expected 16 particles in FilteredMCParticles, got {len(filtered)}")
    filtered_not_exact = frame.get("FilteredNotExactMCParticles")
    # Approximate number of particles in FilteredNotExactMCParticles
    if not 14 <= len(filtered_not_exact) <= 18:
        raise RuntimeError(
            f"Expected between 14 and 18 particles in FilteredNotExactMCParticles, got {len(filtered_not_exact)}"
        )
    filtered_links = frame.get("FilteredLinks")
    if len(filtered_links) != 16:
        raise RuntimeError(f"Expected 16 links in FilteredLinks, got {len(filtered_links)}")

check_metadata(
    "example_tool.root",
    {
        "ToolInitParam": 99,
        "ToolFinalizeParam": 42,
    },
)

check_events("output_TestAlgorithmWithTFile_framework.root", 100)

f_tfile = ROOT.TFile.Open("output_TestAlgorithmWithTFile_myTFile.root")
mytree = f_tfile.Get("mytree")
if mytree is None:
    raise RuntimeError("output_TestAlgorithmWithTFile_myTFile.root has no TTree named mytree")
if mytree.GetEntries() == 0:
    raise RuntimeError(
        "output_TestAlgorithmWithTFile_myTFile.root contains TTree mytree with no entries"
    )
