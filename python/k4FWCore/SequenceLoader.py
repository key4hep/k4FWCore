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

import os
from typing import Optional, Dict, Any, Union
from k4FWCore.utils import import_from, get_logger

logger = get_logger()


class SequenceLoader:
    """A class for loading algorithm sequences onto a list of algorithms

    It dynamically loads algorithms from Python files based on the given
    sequence names. In the import process it will look for a Sequence of
    algorithms which might have configuration constants that depend on some
    global calibration configuration. These constants are provided during the
    import of a sequence, such that the imported python files do not need to
    define all of them.
    """

    def __init__(self, alg_list: list, global_vars: Optional[Dict[str, Any]] = None) -> None:
        """Initialize the SequenceLoader

        This initializes a SequenceLoader with the list of algorithms to which
        dynamically loaded algorithms should be appended to. It optionally takes
        some global calibration constants that should be injected during import
        of the sequence files

        Args:
            alg_list (List): A list to store loaded sequence algorithms.
            global_vars (Optional[Dict[str, Any]]): A dictionary of global
                variables for the sequences. Defaults to None. The keys in this
                dictionary will be the available variables in the imported
                module and the values will be the values of these variables.
        """
        logger.info(f"Creating SequenceLoader with {len(alg_list)} algorithms already defined")
        self.alg_list = alg_list
        self.global_vars = global_vars

    def load_from(self, module_path: Union[str, os.PathLike], sequence_name: str) -> None:
        """Load a sequence of algorithms from a specified Python file and append
        it to the algorithm list

        Args:
            module_path (Union[str, os.PathLike]): The path to the python module
                (file) from which to load the sequence. The path is interpreted
                to be relative to the execution directory of the process from
                which this method is called unless an absolute path is passed.
            sequence_name (str): The name of the sequence to load from the
                specified python module

        Examples:
            >>> alg_list = []
            >>> seq_loader = SequenceLoader(alg_list)
            >>> seq_loader.load_from("Tracking/TrackingDigi.py",
                                     "TrackingDigiSequence")

            This will import the file `Tracking/TrackingDigi.py` and add the
            sequence of algorithms that is defined in `TrackingDigiSequence` in
            that file to the alg_list

        """
        logger.info(f"Loading '{sequence_name} from '{module_path}'")
        seq_module = import_from(
            module_path,
            global_vars=self.global_vars,
        )

        seq = getattr(seq_module, sequence_name)
        logger.debug(f"Adding {len(seq)} algorithms contained in '{sequence_name}'")
        self.alg_list.extend(seq)

    def load(self, sequence: str) -> None:
        """Loads a sequence algorithm from a specified Python file and appends
        it to the algorithm list

        This is a convenience overload for load_from that constructs the
        filename from the sequence parameter name and imports the sequence from
        the imported module.

        Args:
            sequence (str): The name of the sequence to load. The sequence name
                should correspond to a Python file and class name following the
                pattern `{sequence}.py` and `{sequence}Sequence`, respectively.
                The sequence is interpreted to be relative to the path from
                which the process is launched, unless it's an absolute path.

        Examples:
            >>> alg_list = []
            >>> seq_loader = SequenceLoader(alg_list)
            >>> seq_loader.load("Tracking/TrackingDigi")

            This will import the file `Tracking/TrackingDigi.py` and add the
            sequence of algorithms that is defined in `TrackingDigiSequence` in
            that file to the alg_list

        """
        filename = f"{sequence}.py"
        seq_name = f"{sequence.split('/')[-1]}Sequence"
        return self.load_from(filename, seq_name)
