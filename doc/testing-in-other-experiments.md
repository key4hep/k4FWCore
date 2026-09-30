<!--
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
-->
# How ATLAS and LHCb provide test infrastructure to downstream packages

This note summarizes how ATLAS (athena) and LHCb let packages declare and run
tests. It is meant as input for designing the test helpers that k4FWCore exports
to downstream Key4hep repositories.

Checkouts inspected:

- athena at `3ba61768d6c`
- LHCb at `1bf6f80a42`
- Gaudi at `c35e879dd`

File paths are relative to the root of the corresponding repository.

## ATLAS (athena)

### CMake API

All tests go through `atlas_add_test`, which has about 1700 call sites. The
function is not defined in athena. It comes from AtlasCMake (atlasexternals),
loaded through `find_package( AtlasCMake QUIET )` in
`Projects/Athena/CMakeLists.txt:75`. `atlas_ctest_setup()` is also external.
Its internals described below are inferred from usage.

Keywords seen in use:

| Keyword | Purpose |
|---|---|
| `SOURCES`, `LINK_LIBRARIES`, `INCLUDE_DIRS` | Build a C++ test executable |
| `SCRIPT <cmd...>` | Run a shell or Python command instead |
| `ENVIRONMENT "K=V"` | Extra environment, e.g. `JOBOPTSEARCHPATH`, `ATLAS_REFERENCE_TAG` |
| `PRE_EXEC_SCRIPT "<cmd>"` | Run before the test, e.g. `"rm -f test.data*"` |
| `POST_EXEC_SCRIPT <script>` | Validation step. Default `post.sh`; also `noerror.sh`, `nopost.sh` or a custom script (about 970 uses) |
| `LOG_SELECT_PATTERN`, `LOG_IGNORE_PATTERN` | Regexes that filter the log before comparison |
| `PROPERTIES <prop> <val>` | Any CTest property (`TIMEOUT`, `WILL_FAIL`, `SKIP_RETURN_CODE`, `FIXTURES_*`, ...) |
| `PRIVATE_WORKING_DIRECTORY` | Run in `CMakeFiles/unitTestRun_<name>/` instead of the shared `CMakeFiles/unitTestRun/` |
| `DEPENDS <test>` | Ordering between tests |

Conventions (inferred):

- The CTest name is `<Package>_<name>_ctest`.
- The log is written to `<name>.log`.
- The generated wrapper script is `test-bin/<name>.exe` (`AtlasTest/CITest/README.md:103-105`).

Other helpers defined inside athena:

- **`atlas_add_citest`** (`AtlasTest/CITest/cmake/CITestFunctions.cmake:16-57`) wraps
  `atlas_add_test`. It adds a dedicated working directory, `TIMEOUT 3600`, a
  `CITest_<name>_fixture` setup fixture, generated pre/post scripts, and a
  `DEPENDS_SUCCESS` keyword that maps to `FIXTURES_REQUIRED`. It is enabled by
  `ATLAS_ENABLE_CI_TESTS`, which WorkDir turns on. The tests are grouped per
  project in `AtlasTest/CITest/{Athena,AnalysisBase,AthGeneration,AthSimulation}.cmake`.
- Domain-specific wrappers:
  - `run_tpcnv_test` (`Database/AthenaPOOL/AthenaPoolUtilities/cmake/AthenaPoolUtilitiesTestConfig.cmake`)
  - `datamodel_run_test` (`Control/DataModelTest/DataModelRunTests/CMakeLists.txt`)
  - trigger menu tests (`Trigger/TriggerCommon/TriggerMenuMT/CMakeLists.txt:58-83`)

### Runtime environment

- The generated wrapper sources the build or release environment. This part
  lives in AtlasCMake and is not visible in athena.
- Post-exec scripts receive:
  - `ATLAS_CTEST_TESTNAME`
  - `ATLAS_CTEST_TESTSTATUS` (the test's exit code)
  - `ATLAS_CTEST_PACKAGE`
  - `ATLAS_CTEST_LOG_SELECT_PATTERN` and `ATLAS_CTEST_LOG_IGNORE_PATTERN`

  They are documented in `AtlasTest/TestTools/share/post.sh:14-30`.
- Packages add environment through `<Pkg>EnvironmentConfig.cmake` modules, e.g.
  `Tools/ART/ARTEnvironmentConfig.cmake` appends to `DATAPATH`.

### Reference and log comparison

`AtlasTest/TestTools/share/post.sh` is the default validation step:

1. On exit status 0 it takes `../share/<testname>.ref`. If that file is missing,
   it fetches the reference through `ATLAS_REFERENCE_TAG` (`get_files -data` or
   `${ATLAS_REFERENCE_DATA}`).
2. Both the log and the reference are normalized with `sed`.
3. Lines matching the select pattern are kept. `ERROR` and `FATAL` are always
   selected.
4. Lines matching the ignore pattern are dropped: about 170 defaults plus the
   user's patterns.
5. The results are compared with `diff -a -b -E -B -u`. The test fails if the
   reference is missing, or empty after filtering.

Other validation scripts:

- `noerror.sh` fails on `ERROR`, `FATAL`, FPE warnings and `runtime error:`,
  without any reference.
- `nopost.sh` just returns the exit code.

Updating references:

- For unit tests, by hand: copy `unitTestRun/<name>.log-todiff` to
  `share/<name>.ref`.
- For CI workflow tests, `Tools/PROCTools/python/update_ci_reference_files.py`
  does it from a merge request's CI results.

### C++ unit-test support

- **`AtlasTest/TestTools`:**
  - `Athena_test::initGaudi(...)`, which can load job options through
    `JOBOPTSEARCHPATH`, and an RAII `InitGaudi` fixture
  - assertion and utility headers: `expect.h`, `expect_exception.h`,
    `SGassert.h`, `FLOATassert.h`, `leakcheck.h`, ...
  - CppUnit drivers
- **`AtlasTest/GoogleTestTools`:** an `Athena_test::InitGaudiGoogleTest` fixture
  for GoogleTest. `Control/AthToolSupport/AsgTesting` provides the same for
  dual-use analysis code.
- **Boost.Test** is also used, in about 56 CMakeLists.

### Python tests

- The dominant pattern (about 330 tests) runs a configuration module as a script:

  ```cmake
  atlas_add_test( LuminosityCondAlgConfig_test
                  SCRIPT python -m LumiBlockComps.LuminosityCondAlgConfig
                  LOG_SELECT_PATTERN "ComponentAccumulator|^---|^IOVDbSvc" )
  ```

  The module's `__main__` block builds a ComponentAccumulator and prints it. The
  printout is then diffed against a `.ref` file.
- `AthenaConfiguration/TestDefaults.py` provides standard test input files,
  geometry tags and conditions tags.
- `unittest` is used in about 110 files, e.g. `python -m unittest discover`.
  pytest is not used.

### CI and large-scale tests

- CITest tests are ordinary CTest tests with the label `CITest`
  (`ctest -L CITest`).
- `Tools/WorkflowTestRunner` runs full workflows and checks them against
  versioned references on cvmfs: FrozenTier0 policy, AOD digest and content,
  metadata, warnings and FPEs.
- ART (the tool itself is external) runs about 1300 scripts with `# art-*`
  headers. They are installed with `atlas_install_scripts(test/*.sh)` and report
  results with `art-result:` lines.

### Examples

```cmake
# AtlasTest/ControlTest/CMakeLists.txt: C++ test compared to share/ProxyProviderSvc_test.ref
atlas_add_test( ProxyProviderSvc_test
   SOURCES test/ProxyProviderSvc_test.cxx
   LINK_LIBRARIES TestTools ToyConversionLib AthenaKernel SGTools StoreGateLib GaudiKernel CxxUtils
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )
```

- `Control/AthenaExamples/AthExCUDA/CMakeLists.txt` skips the test when there is
  no GPU: `PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"` together with
  `PROPERTIES SKIP_RETURN_CODE 2`.
- `Database/AthenaRoot/AthenaRootComps/CMakeLists.txt` combines a reference tag,
  a custom post script, `PRIVATE_WORKING_DIRECTORY` and `DEPENDS`. A dependent
  test reads its predecessor's output from `../unitTestRun_<name>/`.

### Assessment

Pros:

- A single, well-documented entry point for every kind of test.
- Validation is on by default: every test catches `ERROR` and `FATAL` lines.
- Pre/post hooks and private working directories come for free.
- Smoke tests and heavy CI tests share the CTest mechanism and differ only by label.

Cons:

- The core function lives in a separate, heavy CMake framework.
- Line-by-line log diffs are brittle. They need a huge default ignore list,
  references churn, and updating them is a manual copy.
- Everything is bash-based, and Python tests use `unittest` rather than pytest.

## LHCb

### CMake API

LHCb mostly uses what upstream Gaudi provides in `cmake/GaudiToolbox.cmake`.

**`gaudi_add_pytest([paths] ROOT_DIR PREFIX LABELS OPTIONS WORKING_DIRECTORY PROPERTIES DEPENDS COVERAGE COVERAGE_OPTIONS)`**
(Gaudi `GaudiToolbox.cmake:763-1020`, used in 44 LHCb CMakeLists):

- It generates a CTest include file that runs
  `run python -m pytest --collect-only` when ctest starts. This means tests are
  discovered without reconfiguring, and the result is cached by an MD5 of the
  test files.
- Test names default to `<pkg>.pytest.<rootdir>.<file>`.

**`gaudi_add_executable(... TEST)`** (Gaudi `GaudiToolbox.cmake:654-704`):

- Links Catch2 and calls `catch_discover_tests`, or else does
  `add_test(NAME <pkg>.<exe> COMMAND run $<TARGET_FILE:exe>)`.
- Test executables are not installed.

**`gaudi_add_tests(pytest)`** is deprecated in favour of `gaudi_add_pytest`.
Gaudi no longer supports QMTest.

LHCb additions in `cmake/`:

- **`qmtest_support.cmake`** redefines `gaudi_add_tests(QMTest)` to bring QMTest
  back. It globs `tests/qmtest/**/*.qmt` and runs each file with
  `run python -m GaudiConf.QMTest.Run`. A generated metadata step
  (`extract_qmtest_metadata.py`) turns QMTest prerequisites into CTest fixtures
  and suites into labels.
- **`lhcb_env([PRIVATE] SET|PREPEND|APPEND|DEFAULT ...)`**
  (`LHCbConfigUtils.cmake:50-82`) injects `export` lines into Gaudi's generated
  `env.sh`. Non-private entries are recorded and replayed in the installed
  `<Project>Config.cmake`, so downstream projects inherit them.
- There are no `lhcb_add_*test*` wrappers.
- `LHCbConfigUtils.cmake`, `qmtest_support.cmake` and the metadata script are
  installed for downstream projects (`CMakeLists.txt:190-207`).

### Test formats and usage

| Item | Count |
|---|---|
| `test_*.py` files | 297 |
| Classes deriving from `LHCbExeTest` | 223 |
| YAML reference files | 128 |
| `.qmt` files | 12, all in `GaudiConf`, mostly fixtures for testing the QMTest support itself |

`utils/qmt2pytest` converts `.qmt` files to pytest.

**pytest (current).** The machinery is Gaudi's `GaudiPolicy/python/GaudiTesting/`,
registered as the pytest plugins `collect_for_ctest`,
`ctest_measurements_reporter` and `gaudi_fixtures`:

- `SubprocessBaseTest` has the attributes `command`, `reference`,
  `environment`, `timeout=600`, `returncode=0` and `popen_kwargs`.
- `GaudiExeTest` adds:
  - `options` (a callable, dict or string, written to a temporary options file)
  - `preprocessor`
  - built-in checks: `test_stdout`, `test_stderr`, `test_ttrees`, `test_histos`,
    `test_count_messages` (default: no `ERROR` or `FATAL`) and `test_record_options`
- Fixtures: `fixture_result`, `stdout`, `stderr`, `returncode`, `cwd`,
  `reference_path`, `reference`.
- Markers:
  - `ctest_fixture_setup` / `ctest_fixture_required`, which become CTest fixtures
  - `shared_cwd`
  - `do_not_collect_source`

LHCb's `GaudiConf/python/LHCbTesting/` adds:

- `LHCbExeTest(GaudiExeTest)`, with `test_counters`, `exclude_counters`,
  `fluctuating_counters`, an LHCb-specific preprocessor, and timeout scaling for
  sanitizer builds
- extra fixtures: `counters`, `histos`, `json_dump_filename`

**QMTest (legacy).** Tests are XML `.qmt` files with fields such as `program`,
`args`, `options`, `reference`, `validator`, `environment`, `timeout`,
`exit_code` and `prerequisites`. The runner is re-implemented in
`GaudiConf/python/GaudiConf/QMTest/`.

### Reference handling

- **YAML references (pytest):**
  - One file holds several keys: `stdout`, `messages_count`, `counters`, ...
  - Platform variants such as `<ref>.detdesc.yaml` or `<ref>.x86_64_v2.yaml` are
    picked when their flags are a subset of the current platform
    (`GaudiTesting/utils.py:119-165`).
  - On a mismatch, `<ref>.new` is written. With
    `GAUDI_TEST_IGNORE_STDOUT_VALIDATION=1`, the reference is overwritten instead.
- **Legacy `.ref` files:** handled by `BaseTest.validateWithReference` and
  `countErrorLines`, with counter comparison added in `LHCbTest`. Only one such
  file remains.
- **Updating references:** done by an external GitLab reference-update bot
  (`lhcb-rta/reference-update-bot`, included from `.gitlab-ci.yml`).

### Runtime environment

- Gaudi generates `<project>env.sh` in the build directory. It contains `PATH`,
  `LD_LIBRARY_PATH`, `PYTHONPATH`, `ROOT_INCLUDE_PATH` and `GAUDI_PLUGIN_PATH`,
  built from the runtime paths registered by `gaudi_add_module`,
  `gaudi_add_executable`, etc., plus the environment at configure time.
- It also generates the `run` wrapper (`. env.sh; exec "$@"`), available as the
  imported target `run`.
- Every test, whether C++, pytest or QMTest, is launched through `run`.
- Project-wide variables are added with `lhcb_env`, e.g. `OPENBLAS_NUM_THREADS`,
  `STDOPTS` and the QMTest class. They are exported to downstream projects
  through `LHCbConfig.cmake`.
- Individual tests rarely need their own `ENVIRONMENT`.

### C++ unit tests

- `gaudi_add_executable(... TEST)` is used in 21 CMakeLists.
- Boost.Test is used in about 48 files and Catch2 in about 51.
- Many helper executables are not marked `TEST` and are instead run from pytest
  tests.

### Examples

```cmake
# Kernel/LHCbMath/CMakeLists.txt
gaudi_add_pytest(tests/pytest)
gaudi_add_executable(TestTruncate SOURCES tests/truncate.cpp LINK LHCb::LHCbMathLib)
foreach(test IN ITEMS test_md5 ...)
  gaudi_add_executable(${test} SOURCES tests/${test}.cpp
      LINK LHCb::LHCbMathLib Boost::unit_test_framework TEST)
endforeach()
```

```python
# Kernel/LHCbMath/tests/pytest/test_truncate.py
class Test(LHCbExeTest):
    command = ["TestTruncate"]
    reference = "../refs/TestTruncate.yaml"
```

```python
# GaudiAlg/tests/pytest/evtcolsex/test_write.py: chaining through CTest fixtures
@pytest.mark.ctest_fixture_required("gaudialg.evtcolsex.prepare")
@pytest.mark.ctest_fixture_setup("gaudialg.evtcolsex.write")
@pytest.mark.shared_cwd("GaudiAlg")
class Test(LHCbExeTest):
    command = ["gaudirun.py", "-v", "../../options/EvtColsEx/Write.py"]
    reference = "../refs/EvtColsEx/Write.yaml"
    environment = ["GAUDIAPPNAME=", "GAUDIAPPVERSION="]
```

### Assessment

Pros:

- Almost no custom CMake: the machinery is upstream Gaudi, which Key4hep already
  ships.
- Tests are Python classes with fixtures, so they can check structured content
  (counters, ROOT files) rather than only diffing text.
- Tests are discovered when ctest runs, so adding one needs no reconfiguration.
- The build-tree environment comes for free from `run`, and `lhcb_env` passes
  the environment on to downstream projects.

Cons:

- Harder to learn: GaudiTesting, pytest plugins, markers.
- pytest must be available, and collecting tests adds time when ctest starts.
- stdout references still churn; the external bot is what makes this manageable.
- Some QMTest legacy remains.

## Relevance for k4FWCore

- Gaudi's `run` script is already created for every project that calls
  `find_package(Gaudi)`, and `k4FWCoreConfig.cmake` does so through
  `find_dependency(Gaudi)`. Downstream Key4hep packages could therefore drop
  their hand-written `set_test_env` functions and run tests through `run`. This
  has not been verified yet.
- `gaudi_add_pytest` and `GaudiTesting.GaudiExeTest` are available in Key4hep
  today. A thin `K4RunTest(GaudiExeTest)` layer with podio/EDM4hep-aware checks
  (collections, event counts, metadata) would follow the LHCb model.
- The ATLAS model suggests two defaults worth keeping in any k4FWCore helper:
  - fail on `ERROR` and `FATAL` messages by default
  - give each test its own working directory
