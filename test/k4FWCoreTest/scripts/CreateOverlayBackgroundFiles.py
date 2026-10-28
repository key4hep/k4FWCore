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

# Writes the background input for the OverlayTiming tests: groups of
# single-event files, laid out the way beam-induced background usually comes.
#
#   overlay_background/
#     groupA/bkg_000.root ... bkg_007.root   group 1, files 0-7
#     groupB/bkg_000.root ... bkg_002.root   group 2, files 0-2
#     groupB_extra.root                      group 2, file 3, listed next to the glob of groupB
#     calo_merge_signal.root                 signal and background of the calorimeter
#     calo_merge_background.root             merging test, see CALO_MERGE_WINDOW
#
# Each file is identified by the tag 100 * group + file, so that every overlaid
# object can be traced back to the file it was read from. Each object stores
# the tag in a different field:
#
#   MCParticle (parent)    PDG = tag
#   MCParticle (daughter)  PDG = -tag, momentum = particle_momentum(tag)
#   SimTrackerHit          cellID = tag
#   SimCalorimeterHit      cellID = tag
#   CaloHitContribution    PDG = tag
#
# The tracker hit and the calorimeter contribution are linked to the daughter,
# so a relation that ignores the offset of the overlaid block points at the
# wrong particle. The momentum of the hit (HIT_MOMENTUM) is deliberately
# different from that of its particle.

import argparse
import os
import shutil

import podio
import edm4hep

GROUP_A = 1
GROUP_B = 2
N_FILES_GROUP_A = 8
N_FILES_GROUP_B = 3


def background_tag(group, file_index):
    return 100 * group + file_index


def particle_momentum(tag):
    return (float(tag), 0.5 * tag, -1.0 * tag)


HIT_MOMENTUM = (-1.0, -2.0, -3.0)


def write_background_file(path, tag):
    particles = edm4hep.MCParticleCollection()
    parent = particles.create()
    parent.setPDG(tag)
    daughter = particles.create()
    daughter.setPDG(-tag)
    daughter.setMomentum(edm4hep.Vector3d(*particle_momentum(tag)))
    parent.addToDaughters(daughter)
    daughter.addToParents(parent)

    tracker_hits = edm4hep.SimTrackerHitCollection()
    hit = tracker_hits.create()
    hit.setCellID(tag)
    hit.setTime(1.0)
    hit.setMomentum(edm4hep.Vector3f(*HIT_MOMENTUM))
    hit.setParticle(daughter)

    calo_hits = edm4hep.SimCalorimeterHitCollection()
    contributions = edm4hep.CaloHitContributionCollection()
    contribution = contributions.create()
    contribution.setPDG(tag)
    contribution.setTime(1.0)
    contribution.setParticle(daughter)
    calo_hit = calo_hits.create()
    # OverlayTiming merges calorimeter hits by cellID: a background hit landing
    # on an occupied cell only adds its contributions to the existing hit. Using
    # the tag as cellID gives one output hit per drawn file, holding one
    # contribution per draw. The tags never coincide with the signal cellIDs
    # (1-3), which keeps the checks simple; merging into signal hits is
    # tested with the calo_merge files instead.
    calo_hit.setCellID(tag)
    calo_hit.addToContributions(contribution)

    frame = podio.Frame()
    frame.put(particles, "MCParticles")
    frame.put(tracker_hits, "SimTrackerHits")
    frame.put(calo_hits, "SimCalorimeterHits")
    frame.put(contributions, "CaloHitContributions")

    writer = podio.root_io.Writer(path)
    writer.write_frame(frame, "events")


# The calo_merge files hold one event each, with calorimeter hits at the origin,
# so there is no time of flight. They are overlaid in a single bunch crossing,
# so the background times are not shifted either. A hit is overlaid whole if at
# least one of its contributions is inside CALO_MERGE_WINDOW, and a background
# hit on an occupied cell adds its contributions and their energies to the hit
# there. Each hit is given as (cellID, [(contribution PDG, time in ns, energy), ...]),
# and its energy is the sum of the energies of its contributions. Signal PDGs
# are 1xx, background PDGs 2xx. The energies are powers of two, to have exact sums.
CALO_MERGE_WINDOW = [0.0, 10.0]  # ns
CALO_MERGE_SIGNAL_HITS = [
    (1, [(101, 5.0, 1.0), (102, 20.0, 2.0)]),  # partly inside: kept whole
    (2, [(103, 20.0, 4.0)]),  # outside: dropped
    (3, [(104, 5.0, 8.0)]),  # inside
]
CALO_MERGE_BACKGROUND_HITS = [
    (1, [(201, 5.0, 16.0), (202, 30.0, 32.0)]),  # partly inside: merged whole into cell 1
    (2, [(203, 5.0, 64.0)]),  # inside, on the cell of a dropped signal hit: new hit
    (3, [(204, 40.0, 128.0)]),  # outside: not merged into cell 3
    (4, [(205, 30.0, 256.0)]),  # outside: dropped
    (5, [(206, 5.0, 512.0), (207, 50.0, 1024.0)]),  # partly inside: new hit, kept whole
]
# The overlaid hits, as {cellID: ([contribution PDG, ...], hit energy)}
CALO_MERGE_EXPECTED = {
    1: ([101, 102, 201, 202], 51.0),
    2: ([203], 64.0),
    3: ([104], 8.0),
    5: ([206, 207], 1536.0),
}


def write_calo_merge_file(path, hits):
    """Writes one event with the calorimeter hits, all contributions pointing at
    the only MCParticle"""
    particles = edm4hep.MCParticleCollection()
    particle = particles.create()
    calo_hits = edm4hep.SimCalorimeterHitCollection()
    contributions = edm4hep.CaloHitContributionCollection()
    for cell_id, hit_contributions in hits:
        calo_hit = calo_hits.create()
        calo_hit.setCellID(cell_id)
        calo_hit.setEnergy(sum(energy for _, _, energy in hit_contributions))
        for pdg, time, energy in hit_contributions:
            contribution = contributions.create()
            contribution.setPDG(pdg)
            contribution.setTime(time)
            contribution.setEnergy(energy)
            contribution.setParticle(particle)
            calo_hit.addToContributions(contribution)

    frame = podio.Frame()
    frame.put(particles, "MCParticles")
    frame.put(calo_hits, "SimCalorimeterHits")
    frame.put(contributions, "CaloHitContributions")

    writer = podio.root_io.Writer(path)
    writer.write_frame(frame, "events")


def main():
    parser = argparse.ArgumentParser(
        description="Write the background input for the OverlayTiming tests"
    )
    parser.add_argument("--output-dir", default="overlay_background")
    args = parser.parse_args()

    shutil.rmtree(args.output_dir, ignore_errors=True)
    group_a = os.path.join(args.output_dir, "groupA")
    group_b = os.path.join(args.output_dir, "groupB")
    for directory in (group_a, group_b):
        os.makedirs(directory)

    for i in range(N_FILES_GROUP_A):
        write_background_file(
            os.path.join(group_a, f"bkg_{i:03d}.root"), background_tag(GROUP_A, i)
        )
    for i in range(N_FILES_GROUP_B):
        write_background_file(
            os.path.join(group_b, f"bkg_{i:03d}.root"), background_tag(GROUP_B, i)
        )
    write_background_file(
        os.path.join(args.output_dir, "groupB_extra.root"),
        background_tag(GROUP_B, N_FILES_GROUP_B),
    )
    write_calo_merge_file(
        os.path.join(args.output_dir, "calo_merge_signal.root"), CALO_MERGE_SIGNAL_HITS
    )
    write_calo_merge_file(
        os.path.join(args.output_dir, "calo_merge_background.root"),
        CALO_MERGE_BACKGROUND_HITS,
    )


if __name__ == "__main__":
    main()
