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
Steering file for the SimTrackerHitCellMerger test.

Run with:

    k4run test_simtrackerhit_cell_merger_steer.py --input input.root --output output.root

One instance is scheduled per value of the MultiTrackCellHandling property, plus one more for the
non-default "EarliestHit" RepresentativeKinematics and one more with ExcludeOverlayHits switched
off, so that every choice of the two string properties is exercised in a single job. The instances
that do not set RepresentativeKinematics use its default, "Average", and every instance but the
last uses the default ExcludeOverlayHits, true.

The collection name properties are declared by the k4FWCore KeyValues of the transformer and are
therefore list valued, so they have to be given as lists rather than as bare strings.

Input/output file names default to sensible values but can be overridden via CLI, e.g. by the
CTest setup in CMakeLists.
"""

from Configurables import SimTrackerHitCellMerger
from k4FWCore import ApplicationMgr, IOSvc
from k4FWCore.parseArgs import parser

# ---------------------------------------------------------------------------
# Collection names.
# Must be kept in sync with the constants in test_simtrackerhit_cell_merger.py.
# ---------------------------------------------------------------------------
INPUT_COLL = "SimTrackerHits"
OUT_COLL_ALL = "MergedHitsAll"
OUT_COLL_MOST_PRIMARY = "MergedHitsMostPrimaryInCell"
OUT_COLL_PER_TRACK = "MergedHitsPerTrack"
OUT_COLL_SINGLE_TRACK = "MergedHitsSingleTrackCells"
OUT_COLL_EARLIEST_HIT = "MergedHitsAllEarliestHit"
OUT_COLL_WITH_OVERLAY = "MergedHitsAllWithOverlay"

parser.add_argument("--input", default="input.root", help="Input EDM4hep file")
parser.add_argument("--output", default="output.root", help="Output EDM4hep file")
args = parser.parse_known_args()[0]

iosvc = IOSvc()
iosvc.Input = args.input
iosvc.Output = args.output

merger_all = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerAll",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_ALL],
    MultiTrackCellHandling="SumAll",
    ExcludeOverlayHits=True,
)

merger_most_primary = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerMostPrimaryInCell",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_MOST_PRIMARY],
    MultiTrackCellHandling="MostPrimaryInCell",
    ExcludeOverlayHits=True,
)

# Deliberately left without MultiTrackCellHandling: "PerTrack" is its default, so this instance
# covers that value and pins the default at the same time. A change of default would show up as a
# different hit count in the checks for this collection.
merger_per_track = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerPerTrack",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_PER_TRACK],
    ExcludeOverlayHits=True,
)

merger_single_track = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerSingleTrackCells",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_SINGLE_TRACK],
    MultiTrackCellHandling="SkipMultiTrackCells",
    ExcludeOverlayHits=True,
)

merger_earliest_hit = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerAllEarliestHit",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_EARLIEST_HIT],
    MultiTrackCellHandling="SumAll",
    RepresentativeKinematics="EarliestHit",
    ExcludeOverlayHits=True,
)

merger_with_overlay = SimTrackerHitCellMerger(
    "SimTrackerHitCellMergerAllWithOverlay",
    InputSimTrackerHits=[INPUT_COLL],
    OutputSimTrackerHits=[OUT_COLL_WITH_OVERLAY],
    MultiTrackCellHandling="SumAll",
    ExcludeOverlayHits=False,
)

ApplicationMgr(
    TopAlg=[
        merger_all,
        merger_most_primary,
        merger_per_track,
        merger_single_track,
        merger_earliest_hit,
        merger_with_overlay,
    ],
    EvtSel="NONE",
    EvtMax=1,
    ExtSvc=[iosvc],
)
