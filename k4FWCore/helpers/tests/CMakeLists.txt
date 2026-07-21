#[[
Copyright (c) 2014-2024 Key4hep-Project.

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
]]

set(GENERATE_FUNCTIONAL "${PROJECT_SOURCE_DIR}/k4FWCore/helpers/generateFunctional")
set(GF_TESTS_DIR "${CMAKE_CURRENT_LIST_DIR}")

foreach(test
    producer
    consumer
    transformer
    multitransformer
    filter
    runtime_consumer
    runtime_transformer
    event_context
    gaudi_framework)
  add_test(NAME GenerateFunctional_${test}
    COMMAND bash "${GF_TESTS_DIR}/test_${test}.sh"
  )
  # Forward the parent build's generator and C++ compiler so the inner
  # cmake invocation in the test scripts doesn't fall back to "Unix Makefiles"
  # (which fails in CI where only Ninja is available and make is not on PATH).
  set_tests_properties(GenerateFunctional_${test} PROPERTIES
    ENVIRONMENT "GENERATEFUNCTIONAL=${GENERATE_FUNCTIONAL};CMAKE_GENERATOR=${CMAKE_GENERATOR};CXX=${CMAKE_CXX_COMPILER}"
  )
endforeach()
