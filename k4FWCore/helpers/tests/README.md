# Running the generateFunctional tests

The tests run entirely in bash — no CTest or Python test framework needed.
Each script generates a C++ algorithm, runs `cmake`, and compiles it.

---

## Prerequisites

A Key4hep environment and the installed `generateFunctional` script.
`uv` is not required — the tests invoke `python3` directly.

---

## Setup

```bash
# 1. Clone and check out the branch
git clone https://github.com/key4hep/k4FWCore.git
cd k4FWCore
git remote add ianna https://github.com/ianna/k4FWCore.git
git fetch ianna
git checkout ianna/gaudi_functional_generator

# 2. Source the Key4hep environment (use zsh, not bash/tcsh)
zsh
source /cvmfs/sw.hsf.org/key4hep/setup.sh

# 3. Build and install (skip the test suite — it has an unrelated build error)
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DBUILD_TESTING=OFF
make -j$(nproc) install

# 4. Put generateFunctional on PATH
export PATH="${PWD}/../install/bin:${PATH}"

# 5. Since uv is not available on lxplus, alias the script to use python3
alias generateFunctional="python3 $(which generateFunctional)"

# 6. Go back to the repo root
cd ..
```

---

## Run all tests

```bash
bash k4FWCore/helpers/tests/run_all_tests.sh
```

Expected output:

```
  test_consumer ...                  PASS
  test_event_context ...             PASS
  test_filter ...                    PASS
  test_gaudi_framework ...           PASS
  test_multitransformer ...          PASS
  test_producer ...                  PASS
  test_runtime_consumer ...          PASS
  test_runtime_transformer ...       PASS
  test_transformer ...               PASS

9 passed, 0 failed
```

Each test takes ~1–3 minutes on lxplus (cmake configure + compile per test).

---

## Run a single test

```bash
bash k4FWCore/helpers/tests/test_producer.sh
```

---

## What each test covers

| Script | Feature tested |
|---|---|
| `test_producer.sh` | Single output, property |
| `test_consumer.sh` | Single input, property |
| `test_transformer.sh` | Single in/out, `--private-properties` |
| `test_multitransformer.sh` | Multiple in/out, `--type-aliases`, `podio::UserDataCollection` |
| `test_filter.sh` | `FilterPredicate` |
| `test_runtime_consumer.sh` | `--runtime-inputs` / `KeyValues` vector input |
| `test_runtime_transformer.sh` | `--runtime-outputs` / `std::vector<T>` return |
| `test_event_context.sh` | `--event-context`, `finalize()` placement |
| `test_gaudi_framework.sh` | `--framework gaudi`, `--namespace` |

---

## Notes

- Each test creates an isolated `mktemp -d` sandbox, cleaned up automatically on exit.
- `_test_common.sh` must not be run directly — it is sourced by the other scripts.
- If `generateFunctional` is not on `PATH`, the scripts fall back to `../generateFunctional` (one level above `tests/`).
- Tests require the full Key4hep environment on `CMAKE_PREFIX_PATH`. Without it, cmake will fail to find `k4FWCore` or `Gaudi`.
