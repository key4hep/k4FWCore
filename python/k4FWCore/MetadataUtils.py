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


def putMetadata(parameters, default_float_type="double", set_at_finalize=False):
    if default_float_type not in ("float", "double"):
        raise ValueError(
            f"default_float_type must be 'float' or 'double', got '{default_float_type}'"
        )

    from Configurables import MetadataSvc

    svc = MetadataSvc("MetadataSvc")
    int_params = dict(svc.IntParameters)
    float_params = dict(svc.FloatParameters)
    double_params = dict(svc.DoubleParameters)
    str_params = dict(svc.StringParameters)

    for key, value in parameters.items():
        if isinstance(value, bool):
            raise TypeError(
                f"Metadata parameter '{key}' has type bool which is not supported; use int, float, or str"
            )
        elif isinstance(value, int):
            int_params[key] = value
        elif isinstance(value, float):
            if default_float_type == "float":
                float_params[key] = value
            else:
                double_params[key] = value
        elif isinstance(value, str):
            str_params[key] = value
        else:
            raise TypeError(
                f"Metadata parameter '{key}' has unsupported type {type(value).__name__}; expected int, float, or str"
            )

    svc.IntParameters = int_params
    svc.FloatParameters = float_params
    svc.DoubleParameters = double_params
    svc.StringParameters = str_params
    if set_at_finalize:
        svc.SetAtFinalize = True
