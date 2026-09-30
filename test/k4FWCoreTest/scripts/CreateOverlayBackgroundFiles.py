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
#     groupA/README.txt                      not a .root file, must be ignored
#     groupA/nested/bkg_000.root             group 9, subdirectories are not scanned
#     groupB/bkg_000.root ... bkg_002.root   group 2, files 0-2
#     groupB_extra.root                      group 2, file 3, meant to be listed explicitly
#
# Every object in a file carries the tag 100 * group + file, so that each
# overlaid object can be traced back to the file it was read from. The event
# holds a parent particle and its daughter; the tracker hit and the calorimeter
# contribution are linked to the daughter, so a relation that ignores the
# offset of the overlaid block points at the wrong particle. The momentum of
# the hit is deliberately different from that of its particle.

import argparse
import os
import shutil

import podio
import edm4hep

GROUP_A = 1
GROUP_B = 2
GROUP_NESTED = 9
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
    parent.setGeneratorStatus(1)
    daughter = particles.create()
    daughter.setPDG(-tag)
    daughter.setGeneratorStatus(1)
    daughter.setMomentum(edm4hep.Vector3d(*particle_momentum(tag)))
    parent.addToDaughters(daughter)
    daughter.addToParents(parent)

    tracker_hits = edm4hep.SimTrackerHitCollection()
    hit = tracker_hits.create()
    hit.setCellID(tag)
    hit.setEDep(float(tag))
    hit.setTime(1.0)
    hit.setPathLength(2.0)
    hit.setQuality(3)
    hit.setPosition(edm4hep.Vector3d(3, 4, 5))
    hit.setMomentum(edm4hep.Vector3f(*HIT_MOMENTUM))
    hit.setParticle(daughter)

    calo_hits = edm4hep.SimCalorimeterHitCollection()
    contributions = edm4hep.CaloHitContributionCollection()
    contribution = contributions.create()
    contribution.setPDG(tag)
    contribution.setEnergy(float(tag))
    contribution.setTime(1.0)
    contribution.setStepPosition(edm4hep.Vector3f(3, 4, 5))
    contribution.setParticle(daughter)
    calo_hit = calo_hits.create()
    # The signal calorimeter hits use cellIDs 1-3, so background hits are never
    # merged into them
    calo_hit.setCellID(tag)
    calo_hit.setEnergy(float(tag))
    calo_hit.setPosition(edm4hep.Vector3f(0, 0, 0))
    calo_hit.addToContributions(contribution)

    frame = podio.Frame()
    frame.put(particles, "MCParticles")
    frame.put(tracker_hits, "SimTrackerHits")
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
    nested = os.path.join(group_a, "nested")
    for directory in (group_a, group_b, nested):
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
    write_background_file(os.path.join(nested, "bkg_000.root"), background_tag(GROUP_NESTED, 0))
    with open(os.path.join(group_a, "README.txt"), "w") as readme:
        readme.write("Not a background file, OverlayTiming has to skip it\n")


if __name__ == "__main__":
    main()
