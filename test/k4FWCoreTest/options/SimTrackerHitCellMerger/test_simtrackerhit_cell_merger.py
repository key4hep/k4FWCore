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
"""
Minimalistic test for the SimTrackerHitCellMerger Gaudi multi transformer.

What it does
------------
1. (setup step) Creates a small EDM4hep input file containing four MCParticles and one
   SimTrackerHitCollection with hits in three cells:

     cell 1 : particle 0 (the most primary one) with 2 steps of 1.0 and 2.0 mm
              particle 1                        with 1 step  of 0.5 mm
              particle 2                        with 1 step  of 0.25 mm
     cell 2 : particle 1                        with 1 step  of 4.0 mm
     cell 3 : particle 3, overlay               with 2 steps of 1.5 and 2.5 mm

   So cell 1 is crossed by three tracks and cell 2 by a single one. Cell 3 holds the only hits
   flagged isOverlay(), so that ExcludeOverlayHits can be checked in both directions: it must be
   absent from every collection produced with true and present with it set to false. The hits are deliberately
   written in a scrambled order so that the per-cell and per-track grouping are exercised.
   Each hit carries a position and a momentum derived from its
   time, so that both picking one of them and averaging over them give a predictable answer.

   SimTrackerHitCellMerger itself is run separately by CTest via
   `k4run test_simtrackerhit_cell_merger_steer.py`, which runs one instance per value of the
   MultiTrackCellHandling property, plus one more for the non-default "EarliestHit"
   RepresentativeKinematics and one more with ExcludeOverlayHits switched off, so that every
   choice of the two string properties is covered in a single job. The instances that do not set
   RepresentativeKinematics use its default, "Average", so their expected kinematics below are
   means over the hits that were summed.

2. (check step) Reads the output file and asserts the accumulated path lengths, energy deposits,
   MCParticle relations and representative kinematics of each output collection.

Usage
-----
    python3 test_simtrackerhit_cell_merger.py setup   --input input.root
    k4run test_simtrackerhit_cell_merger_steer.py     --input input.root --output output.root
    python3 test_simtrackerhit_cell_merger.py check   --output output.root

Requirements
------------
  pip install podio edm4hep   (or load the key4hep stack)
"""

import argparse
from collections import defaultdict

import edm4hep
import podio

# ---------------------------------------------------------------------------
# Constants that mirror the steering file.
# Must be kept in sync with the constants in test_simtrackerhit_cell_merger_steer.py.
# ---------------------------------------------------------------------------
PARTICLE_COLL = "MCParticles"
INPUT_COLL = "SimTrackerHits"
OUT_COLL_ALL = "MergedHitsAll"
OUT_COLL_MOST_PRIMARY = "MergedHitsMostPrimaryInCell"
OUT_COLL_PER_TRACK = "MergedHitsPerTrack"
OUT_COLL_SINGLE_TRACK = "MergedHitsSingleTrackCells"
OUT_COLL_EARLIEST_HIT = "MergedHitsAllEarliestHit"
OUT_COLL_WITH_OVERLAY = "MergedHitsAllWithOverlay"

CELL_A = 1
CELL_B = 2
CELL_C = 3

# (cellID, particle index, pathLength [mm], eDep [GeV], time [ns]), in scrambled order
HITS = [
    (CELL_A, 0, 2.0, 0.002, 2.0),
    (CELL_B, 1, 4.0, 0.004, 3.0),
    (CELL_A, 2, 0.25, 0.00025, 0.5),
    (CELL_A, 0, 1.0, 0.001, 1.0),
    (CELL_A, 1, 0.5, 0.0005, 1.5),
]

# Same, for the hits that are flagged isOverlay(). They are kept in their own cell so that
# switching ExcludeOverlayHits does not change anything expected of cells 1 and 2.
OVERLAY_HITS = [
    (CELL_C, 3, 1.5, 0.0015, 5.0),
    (CELL_C, 3, 2.5, 0.0025, 6.0),
]


def hit_position(time: float) -> tuple:
    """Position a hit at a point that is a unique function of its time."""
    return (time, 2.0 * time, 3.0 * time)


def hit_momentum(time: float) -> tuple:
    """Give a hit a momentum that is a unique function of its time."""
    return (time, 0.0, 0.0)


# ---------------------------------------------------------------------------
# Step 1 - write the input file
# ---------------------------------------------------------------------------
def write_input_file(path: str) -> None:
    writer = podio.root_io.Writer(path)

    frame = podio.Frame()

    particles = edm4hep.MCParticleCollection()
    # None of these is flagged as created in simulation, so SimTrackerHitCellMerger ranks them by
    # their index within the collection, which DD4hep writes in order of increasing Geant4 trackID.
    # Particle 0 therefore plays the role of the primary muon, 1 and 2 those of two later tracks.
    for pdg in (13, 11, 11):
        particle = particles.create()
        particle.setPDG(pdg)
        particle.setMomentum(edm4hep.Vector3d(1.0, 0.0, 0.0))

    # Particle 3 stands in for a track of an overlaid event
    overlay_particle = particles.create()
    overlay_particle.setPDG(13)
    overlay_particle.setMomentum(edm4hep.Vector3d(1.0, 0.0, 0.0))
    overlay_particle.setOverlay(True)

    hits = edm4hep.SimTrackerHitCollection()
    for cell_id, particle_index, path_length, edep, time in HITS + OVERLAY_HITS:
        hit = hits.create()
        hit.setCellID(cell_id)
        hit.setPathLength(path_length)
        hit.setEDep(edep)
        hit.setTime(time)
        hit.setPosition(edm4hep.Vector3d(*hit_position(time)))
        hit.setMomentum(edm4hep.Vector3f(*hit_momentum(time)))
        hit.setParticle(particles[particle_index])
        hit.setOverlay((cell_id, particle_index, path_length, edep, time) in OVERLAY_HITS)

    frame.put(particles, PARTICLE_COLL)
    frame.put(hits, INPUT_COLL)

    writer.write_frame(frame, "events")
    writer.finish()
    print(f"[setup] Wrote input file: {path}")


# ---------------------------------------------------------------------------
# Helper: compare one merged hit against its expectation
# ---------------------------------------------------------------------------
def hits_by_cell(collection):
    """cellID -> list of hits in that cell, in whatever order the collection has them."""
    by_cell = defaultdict(list)
    for hit in collection:
        by_cell[hit.getCellID()].append(hit)
    return by_cell


def check_hit(
    coll_name: str,
    hit,
    cell_id: int,
    path_length: float,
    edep: float,
    particle_index: int,
    time: float,
) -> None:
    """Assert the accumulated quantities, the MCParticle relation and the representative kinematics.

    `time` is the expected time of the merged hit; its position and momentum are expected to be the
    ones that go with that time, which holds both for a copy of a single input hit and for an
    average, because both are linear in the time.
    """
    prefix = f"{coll_name}[cell {cell_id}]"
    assert (
        hit.getCellID() == cell_id
    ), f"{prefix}: expected cellID {cell_id}, got {hit.getCellID()}"
    assert (
        abs(hit.getPathLength() - path_length) < 1e-5
    ), f"{prefix}: expected path length {path_length} mm, got {hit.getPathLength()}"
    assert (
        abs(hit.getEDep() - edep) < 1e-8
    ), f"{prefix}: expected eDep {edep} GeV, got {hit.getEDep()}"
    assert (
        hit.getParticle().getObjectID().index == particle_index
    ), f"{prefix}: expected MCParticle {particle_index}, got {hit.getParticle().getObjectID().index}"
    assert (
        abs(hit.getTime() - time) < 1e-5
    ), f"{prefix}: expected time {time} ns, got {hit.getTime()}"

    expected_position = hit_position(time)
    position = (hit.getPosition().x, hit.getPosition().y, hit.getPosition().z)
    assert all(
        abs(a - b) < 1e-4 for a, b in zip(position, expected_position)
    ), f"{prefix}: expected position {expected_position} mm, got {position}"

    expected_momentum = hit_momentum(time)
    momentum = (hit.getMomentum().x, hit.getMomentum().y, hit.getMomentum().z)
    assert all(
        abs(a - b) < 1e-4 for a, b in zip(momentum, expected_momentum)
    ), f"{prefix}: expected momentum {expected_momentum} GeV, got {momentum}"


# ---------------------------------------------------------------------------
# Helper: assert that a collection contains no hit of the overlay cell
# ---------------------------------------------------------------------------
def check_no_overlay(coll_name: str, collection) -> None:
    overlay_cells = [hit.getCellID() for hit in collection if hit.getCellID() == CELL_C]
    assert not overlay_cells, (
        f"{coll_name}: ExcludeOverlayHits defaults to true, so the overlay cell {CELL_C} should "
        f"have been dropped, but {len(overlay_cells)} hit(s) of it were written"
    )


# ---------------------------------------------------------------------------
# Step 2 - read back & assert
# ---------------------------------------------------------------------------
def check_output(output_file: str) -> None:
    reader = podio.root_io.Reader(output_file)

    frames = list(reader.get("events"))
    assert len(frames) == 1, f"Expected 1 event frame, got {len(frames)}"

    frame = frames[0]
    available = frame.getAvailableCollections()
    for coll_name in (
        OUT_COLL_ALL,
        OUT_COLL_MOST_PRIMARY,
        OUT_COLL_PER_TRACK,
        OUT_COLL_SINGLE_TRACK,
        OUT_COLL_EARLIEST_HIT,
        OUT_COLL_WITH_OVERLAY,
    ):
        assert coll_name in available, f"Output collection '{coll_name}' not found in output file"

    # --- MultiTrackCellHandling = "SumAll" ---------------------------------
    # One hit per cell, summing every track. Cell 1 gets 1.0 + 2.0 + 0.5 + 0.25 mm and is attributed
    # to the most primary contributor (particle 0). With the default "Average" kinematics it sits at
    # the mean of all four times 2.0, 0.5, 1.0 and 1.5 ns, i.e. at 1.25 ns.
    merged_all = frame.get(OUT_COLL_ALL)
    assert (
        len(merged_all) == 2
    ), f"'SumAll' should give one hit per cell, i.e. 2, got {len(merged_all)}"
    # dictionary with keys 'cellID' and items 'list of every hit in that cell'
    by_cell = hits_by_cell(merged_all)
    assert set(by_cell.keys()) == {
        CELL_A,
        CELL_B,
    }, f"'SumAll' expected cells {CELL_A} and {CELL_B}, got {set(by_cell.keys())}"
    assert all(
        len(by_cell[cell]) == 1 for cell in (CELL_A, CELL_B)
    ), f"'SumAll' expected all cells to have exactly 1 hit"

    check_hit(OUT_COLL_ALL, by_cell[CELL_A][0], CELL_A, 3.75, 0.00375, 0, 1.25)
    check_hit(OUT_COLL_ALL, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)

    # --- MultiTrackCellHandling = "MostPrimaryInCell" ----------------------
    # One hit per cell, but only the steps of the most primary contributor are summed, so cell 1
    # keeps only particle 0's 1.0 + 2.0 mm and the 0.5 + 0.25 mm of the other two tracks are dropped.
    # The average is therefore over particle 0's two hits only, at (2.0 + 1.0) / 2 = 1.5 ns.
    merged_most_primary = frame.get(OUT_COLL_MOST_PRIMARY)
    assert (
        len(merged_most_primary) == 2
    ), f"'MostPrimaryInCell' should give one hit per cell, i.e. 2 in total, got {len(merged_most_primary)}"
    by_cell = hits_by_cell(merged_most_primary)
    assert set(by_cell.keys()) == {
        CELL_A,
        CELL_B,
    }, f"'MostPrimaryInCell' expected cells {CELL_A} and {CELL_B}, got {set(by_cell.keys())}"
    assert all(len(by_cell[cell]) == 1 for cell in (CELL_A, CELL_B))
    check_hit(OUT_COLL_MOST_PRIMARY, by_cell[CELL_A][0], CELL_A, 3.0, 0.003, 0, 1.5)
    check_hit(OUT_COLL_MOST_PRIMARY, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)

    # --- MultiTrackCellHandling = "PerTrack" -------------------------------
    # Same data type, but cell 1 now appears three times, once per contributing track and ordered
    # from the most to the least primary one. Only particle 0 contributed more than one hit, so only
    # its entry is an average, again at 1.5 ns; the others carry the time of their single hit.
    merged_per_track = frame.get(OUT_COLL_PER_TRACK)
    assert len(merged_per_track) == 4, (
        f"'PerTrack' should give 3 hits for cell {CELL_A} and 1 for cell {CELL_B}, i.e. 4, "
        f"got {len(merged_per_track)}"
    )
    by_cell = hits_by_cell(merged_per_track)
    assert set(by_cell.keys()) == {
        CELL_A,
        CELL_B,
    }, f"'PerTrack' expected cells {CELL_A} and {CELL_B}, got {set(by_cell.keys())}"
    assert (
        len(by_cell[CELL_A]) == 3
    ), f"'PerTrack' cell {CELL_A}, expected 3 hits, got {len(by_cell[CELL_A])}"
    assert (
        len(by_cell[CELL_B]) == 1
    ), f"'PerTrack' cell {CELL_B}, expected 1 hit, got {len(by_cell[CELL_B])}"
    check_hit(OUT_COLL_PER_TRACK, by_cell[CELL_A][0], CELL_A, 3.0, 0.003, 0, 1.5)
    check_hit(OUT_COLL_PER_TRACK, by_cell[CELL_A][1], CELL_A, 0.5, 0.0005, 1, 1.5)
    check_hit(OUT_COLL_PER_TRACK, by_cell[CELL_A][2], CELL_A, 0.25, 0.00025, 2, 0.5)
    check_hit(OUT_COLL_PER_TRACK, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)

    # --- MultiTrackCellHandling = "SkipMultiTrackCells" --------------------
    # Cell 1 was crossed by three tracks and is dropped entirely; only the unambiguous cell 2 survives.
    merged_single_track = frame.get(OUT_COLL_SINGLE_TRACK)
    assert len(merged_single_track) == 1, (
        f"'SkipMultiTrackCells' should drop cell {CELL_A} and keep only cell {CELL_B}, i.e. 1 hit, "
        f"got {len(merged_single_track)}"
    )
    by_cell = hits_by_cell(merged_single_track)
    assert set(by_cell.keys()) == {
        CELL_B
    }, f"'SkipMultiTrackCells' expected cell {CELL_B} exclusively, got {set(by_cell.keys())}"
    assert (
        len(by_cell[CELL_B]) == 1
    ), f"'SkipMultiTrackCells' cell {CELL_B}, expected 1 hit, got {len(by_cell[CELL_B])}"
    check_hit(OUT_COLL_SINGLE_TRACK, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)

    # --- RepresentativeKinematics = "EarliestHit" --------------------------
    # Same sums as "SumAll", but the kinematics are now copied from the earliest of the summed hits
    # instead of averaged, so cell 1 takes them from the globally earliest hit of the cell at
    # 0.5 ns, which belongs to particle 2. Cell 2 has a single hit and is therefore unchanged.
    merged_earliest_hit = frame.get(OUT_COLL_EARLIEST_HIT)
    assert (
        len(merged_earliest_hit) == 2
    ), f"'EarliestHit' should give one hit per cell, i.e. 2, got {len(merged_earliest_hit)}"
    by_cell = hits_by_cell(merged_earliest_hit)
    assert set(by_cell.keys()) == {
        CELL_A,
        CELL_B,
    }, f"Kinematics 'EarliestHit' expected cells {CELL_A} and {CELL_B}, got {set(by_cell.keys())}"
    assert all(
        len(by_cell[cell]) == 1 for cell in (CELL_A, CELL_B)
    ), f"Kinematics 'EarliestHit' with 'SumAll' expected all cells to have exactly 1 hit"
    check_hit(OUT_COLL_EARLIEST_HIT, by_cell[CELL_A][0], CELL_A, 3.75, 0.00375, 0, 0.5)
    check_hit(OUT_COLL_EARLIEST_HIT, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)

    # --- ExcludeOverlayHits, default true ----------------------------------
    # None of the collections above was configured with ExcludeOverlayHits, so all of them ran with
    # its default and must have dropped the overlay cell entirely. Their length assertions already
    # pin this down, but check it explicitly so a leak names the culprit.
    for coll_name in (
        OUT_COLL_ALL,
        OUT_COLL_MOST_PRIMARY,
        OUT_COLL_PER_TRACK,
        OUT_COLL_SINGLE_TRACK,
        OUT_COLL_EARLIEST_HIT,
    ):
        check_no_overlay(coll_name, frame.get(coll_name))

    # --- ExcludeOverlayHits = False ----------------------------------------
    # With the flag switched off the overlay hits are booked like any other, so cell 3 appears with
    # both of its steps summed, 1.5 + 2.5 mm, attributed to the overlay particle 3, and at the mean
    # of the times 5.0 and 6.0 ns. Cells 1 and 2 are unaffected, since the overlay hits sit in a
    # cell of their own.
    merged_with_overlay = frame.get(OUT_COLL_WITH_OVERLAY)
    assert len(merged_with_overlay) == 3, (
        f"'ExcludeOverlayHits=False' should keep the overlay cell {CELL_C} as well, i.e. give 3 "
        f"hits, got {len(merged_with_overlay)}"
    )
    by_cell = hits_by_cell(merged_with_overlay)
    assert set(by_cell.keys()) == {
        CELL_A,
        CELL_B,
        CELL_C,
    }, f"'ExcludeOverlayHits=False' expected cells {CELL_A}, {CELL_B} and {CELL_C}, got {set(by_cell.keys())}"
    assert all(
        len(by_cell[cell]) == 1 for cell in (CELL_A, CELL_B, CELL_C)
    ), f"'ExcludeOverlayHits=False' with 'SumAll' expected all cells to have exactly 1 hit"
    check_hit(OUT_COLL_WITH_OVERLAY, by_cell[CELL_A][0], CELL_A, 3.75, 0.00375, 0, 1.25)
    check_hit(OUT_COLL_WITH_OVERLAY, by_cell[CELL_B][0], CELL_B, 4.0, 0.004, 1, 3.0)
    check_hit(OUT_COLL_WITH_OVERLAY, by_cell[CELL_C][0], CELL_C, 4.0, 0.004, 3, 5.5)

    print("[check] All assertions passed.")
    for coll_name in (
        OUT_COLL_ALL,
        OUT_COLL_MOST_PRIMARY,
        OUT_COLL_PER_TRACK,
        OUT_COLL_SINGLE_TRACK,
        OUT_COLL_EARLIEST_HIT,
        OUT_COLL_WITH_OVERLAY,
    ):
        print(f"        {coll_name}: {len(frame.get(coll_name))} hits")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="step", required=True)

    setup_parser = subparsers.add_parser("setup", help="Write the input file")
    setup_parser.add_argument("--input", default="input.root")

    check_parser = subparsers.add_parser(
        "check", help="Check the SimTrackerHitCellMerger output file"
    )
    check_parser.add_argument("--output", default="output.root")

    args = parser.parse_args()

    if args.step == "setup":
        write_input_file(args.input)
    elif args.step == "check":
        check_output(args.output)


if __name__ == "__main__":
    main()
