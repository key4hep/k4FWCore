# generateFunctional tests

Each test generates a C++ algorithm with `generateFunctional`, configures it
with `cmake`, and compiles it. The tests are registered with CTest and run as
part of the standard k4FWCore test suite.

---

## Running via CTest (standard)

```bash
# Source the Key4hep environment
source /cvmfs/sw.hsf.org/key4hep/setup.sh

# Build (tests are enabled by default)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run only the generateFunctional tests
ctest -R GenerateFunctional --output-on-failure
```

Each test takes ~1–3 minutes on lxplus (cmake configure + compile per test).
Run them in parallel with `ctest -j9 -R GenerateFunctional`.

---

## Running a single test manually

The bash scripts can also be run directly without building or installing,
as long as the Key4hep environment is sourced:

```bash
source /cvmfs/sw.hsf.org/key4hep/setup.sh
bash k4FWCore/helpers/tests/test_producer.sh
```

The script finds `generateFunctional` via (in order):
1. `GENERATEFUNCTIONAL` env var (set automatically by CTest)
2. Installed `generateFunctional` on `PATH`
3. `../generateFunctional` relative to the `tests/` directory

---

## What each test covers

| Script | CTest name | Feature |
|---|---|---|
| `test_producer.sh` | `GenerateFunctional_producer` | Single output, property |
| `test_consumer.sh` | `GenerateFunctional_consumer` | Single input, property |
| `test_transformer.sh` | `GenerateFunctional_transformer` | Single in/out, `--private-properties` |
| `test_multitransformer.sh` | `GenerateFunctional_multitransformer` | Multiple in/out, `--type-aliases`, `podio::UserDataCollection` |
| `test_filter.sh` | `GenerateFunctional_filter` | `FilterPredicate` |
| `test_runtime_consumer.sh` | `GenerateFunctional_runtime_consumer` | `--runtime-inputs` / `KeyValues` vector input |
| `test_runtime_transformer.sh` | `GenerateFunctional_runtime_transformer` | `--runtime-outputs` / `std::vector<T>` return |
| `test_event_context.sh` | `GenerateFunctional_event_context` | `--event-context`, `finalize()` placement |
| `test_gaudi_framework.sh` | `GenerateFunctional_gaudi_framework` | `--framework gaudi`, `--namespace` |

---

## Notes

- Each test creates an isolated `mktemp -d` sandbox, cleaned up automatically on exit.
- `_test_common.sh` is sourced by all test scripts — do not run it directly.
- Tests require the Key4hep environment on `CMAKE_PREFIX_PATH`. Source
  `setup.sh` before building or running tests manually.
- `uv` is not required — the tests invoke `python3` directly.
