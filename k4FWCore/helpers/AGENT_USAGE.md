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
# Using an AI Agent with generateFunctional

You can ask an AI agent (such as Claude in Cowork or via the API) to run
`generateFunctional` for you. Instead of memorising flags, describe your
algorithm in plain language and the agent handles the rest.

---

## How it works

1. You describe the algorithm you need.
2. The agent translates your description into a `generateFunctional` command.
3. The agent runs the command and shows you the generated `.cpp` (and
   optionally `CMakeLists.txt`).
4. You ask for changes; the agent re-runs with updated flags.

---

## What to tell the agent

The more detail you provide, the closer the first attempt will be to what you
want. Cover these points:

| What | Example |
|---|---|
| **Algorithm name** | `MyParticleSelector` |
| **Inputs** — type and key name | `edm4hep::MCParticleCollection` named `InputParticles` |
| **Outputs** — type and key name | `edm4hep::MCParticleCollection` named `SelectedParticles` |
| **Properties** — C++ type, name, default, description | `float` named `MinPt`, default `0.5`, "Minimum transverse momentum" |
| **Private properties?** | Yes / No |
| **EventContext needed?** | Yes / No |
| **Type aliases?** | Yes / No |
| **CMake file too?** | Yes / No |
| **Framework** | `k4fwcore` (default) or `gaudi` |
| **C++ namespace** | e.g. `MyExperiment` |

You do not need to know any flags — just describe what you want.

---

## Example prompts

### Minimal — let the agent fill in the gaps

> Generate a transformer called `TrackFilter` that reads
> `edm4hep::TrackCollection` and writes a filtered
> `edm4hep::TrackCollection`.

### With properties

> Generate a transformer `EnergyThresholdFilter` that takes
> `edm4hep::MCParticleCollection:InputParticles` as input and returns
> `edm4hep::MCParticleCollection:OutputParticles`. Add a float property
> `MinEnergy` with default `1.0` and description "Minimum particle energy in
> GeV". Put properties under `private:`. Also emit a `CMakeLists.txt`.

### Multiple inputs and outputs

> I need a MultiTransformer `JetBuilder` with two inputs —
> `edm4hep::MCParticleCollection:Particles` and
> `edm4hep::TrackCollection:Tracks` — and two outputs —
> `edm4hep::ReconstructedParticleCollection:Jets` and
> `podio::UserDataCollection<float>:JetPt`. Use type aliases.

### Runtime (variable-length) inputs

> Generate a consumer `MultiCollectionReader` that reads a variable number
> of `edm4hep::MCParticleCollection` inputs at runtime, with default names
> `MCParticles0` and `MCParticles1`.

### From an existing example

> Look at `ExampleFunctionalTransformerRuntimeCollections.cpp` in the test
> folder and generate something similar for `edm4hep::TrackCollection`.

### Refinement after seeing the output

> That looks good. Can you add an `int` property `MaxParticles` with default
> `100`, and regenerate with `--force`?

---

## What the agent can do automatically

- Infer the functional type (`Consumer`, `Producer`, `Transformer`,
  `MultiTransformer`) from your inputs and outputs.
- Derive default key names from collection types when you don't specify them
  (e.g. `edm4hep::MCParticleCollection` → key `MCParticles`).
- Add the correct `#include` directives for all edm4hep and podio types.
- Emit `DECLARE_COMPONENT()` and a ready-to-build `CMakeLists.txt`.
- Re-run with `--force` to overwrite after you request changes.

---

## Tips

- **`filter` must be explicit.** The agent cannot infer `FilterPredicate` from
  I/O counts alone — say "FilterPredicate" or "filter type" in your prompt.
- **Key names matter.** If your steering file already names the collections,
  tell the agent the exact keys so the generated `KeyValue` strings match.
- **Iterate freely.** Generated code is cheap to redo. Ask the agent to tweak
  property types, add an `EventContext`, switch to `--use-class`, or change
  the namespace — it will re-run the generator rather than hand-editing the
  output.
- **Review before committing.** Check the generated constructor argument order
  and `operator()` signature against your project's conventions before adding
  the file to git.
