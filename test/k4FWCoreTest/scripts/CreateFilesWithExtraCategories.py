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

# Writes input files that contain, besides events and metadata, frame categories
# that the framework does not handle itself (like the "runs" frame written by
# ddsim), to test IOSvc.CopyInputCategories
#
#   extra_categories_<i>.root  for i in range(N_FILES)
#     events                  N_EVENTS frames with an MCParticles collection
#     metadata                1 frame
#     configuration_metadata  1 frame (as written by a previous k4FWCore job)
#     runs                    1 frame with RUN_NUMBER_OFFSET + i and a string parameter
#     other                   OTHER_ENTRIES frames with a parameter and a collection

import podio
import edm4hep

N_FILES = 2
N_EVENTS = 3
OTHER_ENTRIES = 2
RUN_NUMBER_OFFSET = 100


def file_name(i):
    return f"extra_categories_{i}.root"


def run_command_line(i):
    return f"ddsim --random.seed {42 + i}"


def other_tag(i, j):
    return 10 * i + j


def main():
    for i in range(N_FILES):
        writer = podio.root_io.Writer(file_name(i))
        for iev in range(N_EVENTS):
            frame = podio.Frame()
            particles = edm4hep.MCParticleCollection()
            particle = particles.create()
            particle.setPDG(iev)
            frame.put(particles, "MCParticles")
            writer.write_frame(frame, "events")

        metadata = podio.Frame()
        metadata.put_parameter("InputMetadataParam", 7)
        writer.write_frame(metadata, "metadata")

        config = podio.Frame()
        config.put_parameter("gaudiConfigOptions", ["PreviousJob.Option = 'value'\n"])
        writer.write_frame(config, "configuration_metadata")

        run = podio.Frame()
        run.put_parameter("runNumber", RUN_NUMBER_OFFSET + i)
        run.put_parameter("CommandLine", run_command_line(i))
        writer.write_frame(run, "runs")

        for j in range(OTHER_ENTRIES):
            other = podio.Frame()
            other.put_parameter("tag", other_tag(i, j))
            # Frames of other categories can also hold collections, which have to be copied too
            coll = edm4hep.MCParticleCollection()
            coll.create().setPDG(other_tag(i, j))
            other.put(coll, "OtherParticles")
            writer.write_frame(other, "other")

        del writer


if __name__ == "__main__":
    main()
