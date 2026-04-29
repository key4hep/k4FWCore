# gaudi_gen.py — Gaudi Functional C++ Class Generator

`gaudi_gen.py` writes the boilerplate for a Gaudi Functional algorithm: the
`#include`s, the constructor with `KeyValue` / `KeyValues` wiring, the
`operator()` signature, a placeholder body, optional `Gaudi::Property`
members, and (optionally) a matching `CMakeLists.txt`. It supports both the
**k4FWCore** flavour used by Key4hep / FCC and the native
**Gaudi::Functional** flavour.

The script is opinionated: it parses every CLI argument once, builds a
single `AlgorithmSpec`, and renders Jinja2 templates from it. There is no
in-place string surgery on the output, so the generated code is consistent
across the matrix of options.

---

## Requirements

- Python 3.9+
- `Jinja2` (`pip install jinja2`)

---

## Quick start

```bash
# k4FWCore producer (functional type inferred from --outputs)
python3 gaudi_gen.py MyProducer \
    -o 'edm4hep::MCParticleCollection:MCParticles'
```

That writes `MyProducer.cpp` in the current directory. Add `--cmake` to also
emit a `CMakeLists.txt`:

```bash
python3 gaudi_gen.py MyProducer \
    -o 'edm4hep::MCParticleCollection:MCParticles' \
    --cmake
```

---

## File-overwrite policy

`gaudi_gen.py` **never silently overwrites an existing file**. If the target
`.cpp` or `CMakeLists.txt` already exists, the script prints a diagnostic
and exits non-zero:

```
Refusing to overwrite existing CMakeLists.txt at 'CMakeLists.txt'.
  Re-run with --force (or remove the file) if you really want to replace it.
```

Pass `--force` to allow overwriting. The check applies to both the source
file and the CMake file independently, so partial regeneration is fine
(e.g. delete just the `.cpp` and re-run without `--force`).

---

## Arguments

### Positional

| Argument          | Description                                                                                                                                |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `class_name`      | Name of the C++ class to generate (e.g. `MyProducer`).                                                                                     |
| `functional_type` | Optional. One of `consumer`, `producer`, `transformer`, `filter`. If omitted, the type is inferred from the number of inputs and outputs. |

### Inputs / outputs

| Flag                | Format                          | Notes                                                                                                                  |
| ------------------- | ------------------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| `-i`, `--inputs`    | `TYPE:KEY` (one or more)        | If `:KEY` is omitted, a default key is derived from the type name (e.g. `edm4hep::MCParticleCollection` → `MCParticles`). |
| `-o`, `--outputs`   | `TYPE:KEY` (one or more)        | Multiple outputs trigger `MultiTransformer` and a `std::tuple` return type. Mutually exclusive with `--runtime-outputs`. |
| `--runtime-outputs` | `TYPE`                          | Dynamic output collections; `operator()` returns `std::vector<TYPE>`. **k4FWCore-only.**                               |

### Vector / runtime inputs (k4FWCore)

| Flag                  | Format                              | Notes                                                                                                                                       |
| --------------------- | ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| `--runtime-inputs`    | `TYPE:KEY:DEF1[,DEF2,...]`          | Promotes the matching `--inputs` entry to a runtime `KeyValues` vector with the given default location names.                               |
| `--keyvalues-inputs`  | `KEY[:LABEL]`                       | Per-input override: turn the named `--inputs` KEY(s) into vector inputs while leaving the others scalar.                                    |
| `--all-keyvalues`     | flag                                | Treat every `--inputs` entry as a `KeyValues` vector.                                                                                       |

### Properties

| Flag                  | Format                          | Notes                                                                                                  |
| --------------------- | ------------------------------- | ------------------------------------------------------------------------------------------------------ |
| `-p`, `--properties`  | `type:name:default[:description]` | Emits `Gaudi::Property<type> m_<name>{this, "<name>", <default>, "<description>"}` for each entry. |
| `--private-properties`| flag                            | Place `Gaudi::Property` members under a `private:` access label.                                       |

### Class shape & framework

| Flag             | Notes                                                                                              |
| ---------------- | -------------------------------------------------------------------------------------------------- |
| `-n`, `--namespace` | Wrap the generated class in `namespace <name> { ... }`.                                          |
| `--framework`    | `k4fwcore` (default) or `gaudi` (vanilla `Gaudi::Functional`).                                      |
| `--use-class`    | Generate `class ... { public: ... }` instead of the default `struct`.                              |
| `--type-aliases` | Emit `using XxxColl = ...;` aliases for input collection types and use them in the operator signature. |
| `--event-context`| Add `const EventContext&` as the first `operator()` argument and scaffold a `finalize()` override.  |

### Output

| Flag                | Notes                                                                                  |
| ------------------- | -------------------------------------------------------------------------------------- |
| `-f`, `--output-file` | Path for the generated `.cpp`. Default: `<ClassName>.cpp` in the current directory. |
| `--cmake`           | Also emit `CMakeLists.txt` next to the source.                                         |
| `--force`           | Allow overwriting existing files. Without this flag, the script refuses to clobber.    |

---

## Functional-type inference

When the positional `functional_type` is omitted, the script picks one from
the I/O counts:

| inputs | outputs | inferred type      |
| -----: | ------: | ------------------ |
|  > 0   |  == 0   | `consumer`         |
|  == 0  |  >= 1   | `producer`         |
|  >= 1  |  == 1   | `transformer`      |
|  >= 1  |  > 1    | `multitransformer` |

`filter` is never inferred — supply it explicitly.

If you write `transformer` but pass multiple `--outputs`, the script
auto-promotes to `multitransformer`.

---

## Examples

### Producer with multiple outputs and a property

```bash
python3 gaudi_gen.py MyProducer \
    -o 'edm4hep::MCParticleCollection:MCParticles' \
       'edm4hep::TrackCollection:Tracks' \
    -p 'int:ExampleInt:3:An example integer property'
```

The output uses a `retType = std::tuple<...>` alias for readability.

### Native Gaudi transformer with namespace

```bash
python3 gaudi_gen.py MySum \
    -i 'Input1:Loc1' 'Input2:Loc2' \
    -o 'Output:OutLoc' \
    --framework gaudi \
    -n MyNamespace
```

This emits a `BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>`
typedef and uses `Gaudi::Functional::Transformer<...>` as the base.

### Variable-length / runtime inputs (k4FWCore)

```bash
python3 gaudi_gen.py MyVarConsumer \
    -i 'edm4hep::MCParticleCollection:Inputs' \
    --runtime-inputs 'edm4hep::MCParticleCollection:Inputs:MCParticles0,MCParticles1'
```

`Inputs` is wired as `KeyValues("Inputs", {"MCParticles0", "MCParticles1"})`
and `operator()` receives `const std::vector<const edm4hep::MCParticleCollection*>&`.

### Dynamic output collections

```bash
python3 gaudi_gen.py MyDynProducer \
    --runtime-outputs 'edm4hep::MCParticleCollection'
```

The constructor wires `KeyValues("OutputCollections", {"MCParticles"})` and
`operator()` returns `std::vector<edm4hep::MCParticleCollection>`.

### Filter

```bash
python3 gaudi_gen.py MyFilter filter \
    -i 'edm4hep::MCParticleCollection:MCParticles'
```

Returns `bool`.

---

## What ends up in the generated `.cpp`

- A header banner with the exact command line used to generate the file.
- Framework header (`k4FWCore/<Base>.h` or `Gaudi/Functional/<Base>.h`).
  For k4FWCore, `MultiTransformer` is included from `Transformer.h`.
- Auto-detected `edm4hep/<Type>.h` headers and `podio/UserDataCollection.h`
  when applicable.
- An optional `using BaseClass_t = ...;` for native Gaudi.
- Optional `using retType = std::tuple<...>;` (k4FWCore multi-output).
- Optional `using XxxColl = ...;` aliases (`--type-aliases`).
- The class itself: constructor, `initialize()` (when there are vector
  inputs), `operator()`, properties, optional `finalize()` and bookkeeping
  members (`--event-context`).
- `DECLARE_COMPONENT(<ClassName>)` at the bottom.

## What ends up in `CMakeLists.txt`

- `find_package(k4FWCore REQUIRED)` or `find_package(Gaudi REQUIRED)`.
- `find_package(EDM4HEP REQUIRED)` if any collection type is from `edm4hep`.
- `find_package(podio REQUIRED)` if `podio::UserDataCollection` is used.
- `gaudi_add_module(<ClassName>Plugin SOURCES <ClassName>.cpp LINK ...)`
  with the matching link libraries.

---

## Exit codes

| Code | Meaning                                                              |
| ---: | -------------------------------------------------------------------- |
|  `0` | Generation succeeded.                                                |
|  `1` | A target file already existed and `--force` was not supplied.        |
|  `2` | argparse error (bad flag, mutually exclusive options, etc.).         |

## Common pitfalls

- **`--runtime-outputs` + `--outputs`** — these are mutually exclusive.
- **`--runtime-outputs` + `--framework gaudi`** — k4FWCore-only.
- **Inferring `filter`** — the script will not infer this; pass `filter`
  explicitly as the second positional argument.
- **Type keys** — keys like `MCParticles` are derived from the type name by
  stripping `Collection` and adding `s`. Override with the explicit
  `TYPE:KEY` form when that doesn't match your project's conventions.
