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

# The parameters of the OverlayTiming run in TestOverlayTimingRandomMix.py that
# the checks in CheckOutputFiles.py depend on. They are kept here so that the
# options file and the checks cannot drift apart. The background files
# themselves are described in CreateOverlayBackgroundFiles.py.

# Number of signal events that are processed
N_EVENTS = 3
# Number of bunch crossings in the bunch train
N_BX = 6
# Time between two bunch crossings in ns
DELTA_T = 0.5
# Number of background events overlaid per bunch crossing, for every group in
# the order of BackgroundFileNames (group A, group B)
NUMBER_BACKGROUND = [3, 1]
