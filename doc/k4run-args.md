<!--
Copyright (c) 2014-2023 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->
# Adding custom arguments to `k4run`

It is possible to extend `k4run` with custom arguments from a steering file using `k4FWCore.parseArgs`.

Example:

```python
from k4FWCore.parseArgs import parser
parser.add_argument("--trackingOnly", action="store_true", help="Run only track reconstruction", default=False)
my_opts = parser.parse_known_args()[0]

# later
if my_opts.trackingOnly:
    # only run track reconstruction
```

Behind the scenes parser is just a normal instance of pythons [`argparse.ArgumentParser`](https://docs.python.org/3/library/argparse.html), please refer to its documentation for usage details. The only important thing to keep in mind is to always use `parse_known_args()` instead of `parse_args()` so that the normal `k4run` arguments keep working.
