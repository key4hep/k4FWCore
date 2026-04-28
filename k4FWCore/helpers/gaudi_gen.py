#!/usr/bin/env python3
"""
Gaudi Functional C++ Class Generator

Generates boilerplate for Gaudi Functional algorithms in both the
k4FWCore and native Gaudi::Functional frameworks.

"""


import argparse
import re
import shlex
import sys
from dataclasses import dataclass, field
from typing import List, Optional

from jinja2 import Environment, StrictUndefined

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

_FRAMEWORK_NS = {
    "k4fwcore": "k4FWCore",
    "gaudi":    "Gaudi::Functional",
}

_BASE_CLASS = {
    "consumer":         "Consumer",
    "producer":         "Producer",
    "transformer":      "Transformer",
    "multitransformer": "MultiTransformer",
    "filter":           "FilterPredicate",
}


# ---------------------------------------------------------------------------
# Data classes  (parsed once at the CLI boundary, never re-parsed)
# ---------------------------------------------------------------------------

@dataclass
class DataSpec:
    """One input or output collection: a C++ type and a collection-location key."""
    type_name: str
    key:       str           # collection-location name used in KeyValue / KeyValues
    is_vector: bool = False  # True for std::vector<const T*> variable-length inputs

    @staticmethod
    def _split_at_separator(spec: str) -> tuple:
        """
        Split  'TypeName:LocationKey'  at the *last* bare colon (not inside
        angle-brackets, not part of a C++ '::' token).
        Returns (type_str, key_str); key_str is '' when no separator is found.
        """
        depth, last_sep = 0, -1
        for i, ch in enumerate(spec):
            if ch == "<":
                depth += 1
            elif ch == ">":
                depth -= 1
            elif ch == ":" and depth == 0:
                if not (i > 0 and spec[i - 1] == ":") and \
                   not (i + 1 < len(spec) and spec[i + 1] == ":"):
                    last_sep = i
        return (spec, "") if last_sep == -1 else (spec[:last_sep], spec[last_sep + 1:])

    @staticmethod
    def _default_key(type_name: str) -> str:
        """edm4hep::MCParticleCollection  ->  MCParticles"""
        base = type_name.split("::")[-1]
        base = re.sub(r"<.*>", "", base)           # strip template params
        base = re.sub(r"Collection$", "", base)
        return base + "s"

    @classmethod
    def parse(cls, spec: str, is_vector: bool = False) -> "DataSpec":
        type_name, key = cls._split_at_separator(spec)
        if not key:
            key = cls._default_key(type_name)
        return cls(type_name=type_name, key=key, is_vector=is_vector)

    # Derived properties used in templates -----------------------------------

    @property
    def edm4hep_header(self) -> Optional[str]:
        """Return the edm4hep header filename for this type, or None."""
        m = re.search(r"edm4hep::(\w+Collection)", self.type_name)
        if m:
            return re.sub(r"Collection$", "", m.group(1)) + ".h"
        return None

    @property
    def needs_podio_header(self) -> bool:
        return "podio::UserDataCollection" in self.type_name

    @property
    def cpp_sig_type(self) -> str:
        """C++ type as it appears in the template signature."""
        if self.is_vector:
            return f"const std::vector<const {self.type_name}*>&"
        return f"const {self.type_name}&"


@dataclass
class RuntimeInputSpec:
    """An input declared with KeyValues and received as std::vector<const T*>&."""
    data:     DataSpec
    defaults: List[str]  # initial default location names

    @classmethod
    def parse(cls, spec: str) -> "RuntimeInputSpec":
        """
        Format:  TYPE:KEY:Default0[,Default1,...]
        The KEY must match the key of a regular --inputs entry.
        """
        depth, seps = 0, []
        for i, ch in enumerate(spec):
            if ch == "<":
                depth += 1
            elif ch == ">":
                depth -= 1
            elif ch == ":" and depth == 0:
                if not (i > 0 and spec[i - 1] == ":") and \
                   not (i + 1 < len(spec) and spec[i + 1] == ":"):
                    seps.append(i)

        if not seps:
            key = DataSpec._default_key(spec)
            return cls(data=DataSpec(spec, key, is_vector=True), defaults=[key])
        if len(seps) == 1:
            p = seps[0]
            type_name, key = spec[:p], spec[p + 1:]
            return cls(data=DataSpec(type_name, key, is_vector=True), defaults=[key])

        p0, p1 = seps[0], seps[1]
        type_name = spec[:p0]
        key       = spec[p0 + 1:p1]
        defaults  = [d.strip() for d in spec[p1 + 1:].split(",")]
        return cls(data=DataSpec(type_name, key, is_vector=True), defaults=defaults)


@dataclass
class PropertySpec:
    """A Gaudi::Property<T> member declaration."""
    type_name:   str
    name:        str
    default:     str
    description: str

    @classmethod
    def parse(cls, spec: str) -> "PropertySpec":
        """Format:  type:name:default[:description]"""
        parts = spec.split(":", 3)
        return cls(
            type_name   = parts[0],
            name        = parts[1] if len(parts) > 1 else parts[0],
            default     = parts[2] if len(parts) > 2 else "0",
            description = parts[3] if len(parts) > 3 else "",
        )

    @property
    def member_name(self) -> str:
        n = self.name
        return n if n.startswith("m_") else f"m_{n}"


@dataclass
class AlgorithmSpec:
    """
    Fully parsed, validated description of the algorithm to generate.
    This is the single object threaded through all generator methods.
    """
    class_name:       str
    functional_type:  str           # consumer | producer | transformer | multitransformer | filter
    inputs:           List[DataSpec]
    outputs:          List[DataSpec]
    runtime_output:   Optional[DataSpec]    # set when --runtime-outputs is used
    runtime_defaults: dict                  # key -> [default, ...] from --runtime-inputs
    properties:       List[PropertySpec]
    namespace:        Optional[str]
    framework:        str           # k4fwcore | gaudi
    use_class:        bool
    type_aliases:     bool
    private_props:    bool
    all_keyvalues:    bool
    event_context:    bool
    generate_cmake:   bool
    output_file:      str
    command_line:     str

    # --- Derived helpers (used by templates) --------------------------------

    @property
    def is_k4(self) -> bool:
        return self.framework == "k4fwcore"

    @property
    def is_runtime(self) -> bool:
        return self.runtime_output is not None

    @property
    def framework_ns(self) -> str:
        return _FRAMEWORK_NS[self.framework]

    @property
    def base_short(self) -> str:
        return _BASE_CLASS[self.functional_type]

    @property
    def base_full(self) -> str:
        return f"{self.framework_ns}::{self.base_short}"

    @property
    def use_ret_type_alias(self) -> bool:
        return self.is_k4 and not self.is_runtime and len(self.outputs) > 1

    @property
    def cpp_return_type(self) -> str:
        if self.functional_type == "consumer":
            return "void"
        if self.functional_type == "filter":
            return "bool"
        if self.is_runtime:
            return f"std::vector<{self.runtime_output.type_name}>"
        if self.use_ret_type_alias:
            return "retType"
        if len(self.outputs) == 1:
            return self.outputs[0].type_name
        return "std::tuple<{}>".format(", ".join(o.type_name for o in self.outputs))

    @property
    def template_signature(self) -> str:
        """ReturnType(const In1&, const In2&, ...)"""
        in_parts = [inp.cpp_sig_type for inp in self.inputs]
        if self.event_context:
            in_parts = ["const EventContext&"] + in_parts
        sig = "{}({})".format(self.cpp_return_type, ", ".join(in_parts))
        if not self.is_k4:
            sig += ", BaseClass_t"
        return sig

    @property
    def type_alias_pairs(self) -> List[tuple]:
        """[(alias_name, full_type), ...] for --type-aliases mode."""
        if not self.type_aliases:
            return []
        seen: dict = {}
        result = []
        used_aliases: set = set()
        for inp in self.inputs:
            t = inp.type_name
            if t in seen:
                continue
            inner = re.search(r"<([^>]+)>", t)
            if inner:
                base = inner.group(1).strip().split("::")[-1].capitalize()
            else:
                stem = re.sub(r"Collection$", "", t.split("::")[-1])
                if stem.endswith("Link"):
                    base = "Link"
                elif "Hit" in stem:
                    base = re.sub(r"\d+[A-Z]?$", "", stem) or stem
                elif stem.startswith("Reconstructed"):
                    base = "Reco"
                else:
                    words = re.findall(r"[A-Z][a-z0-9]*", stem)
                    base = words[-1] if words else stem
            alias = base + "Coll"
            suffix, candidate = 2, alias
            while candidate in used_aliases:
                candidate = f"{alias}{suffix}"; suffix += 1
            used_aliases.add(candidate)
            seen[t] = candidate
            result.append((candidate, t))
        return result

    def display_type(self, data: DataSpec) -> str:
        """Return alias name for a type if --type-aliases is active, else full type."""
        if not self.type_aliases:
            return data.type_name
        lookup = {t: a for a, t in self.type_alias_pairs}
        return lookup.get(data.type_name, data.type_name)


# ---------------------------------------------------------------------------
# Parsing helpers
# ---------------------------------------------------------------------------

def _infer_functional_type(
    inputs: List[DataSpec],
    outputs: List[DataSpec],
    runtime_output: Optional[DataSpec],
    explicit: Optional[str],
) -> str:
    """
    Determine the functional type.  When the caller supplies an explicit value
    it is honoured (after auto-promoting transformer -> multitransformer).
    Otherwise the type is inferred from the number of inputs and outputs.
    """
    n_in  = len(inputs)
    n_out = len(outputs) + (1 if runtime_output else 0)

    if explicit:
        ft = explicit.lower()
        # Auto-promote: user wrote 'transformer' but gave multiple outputs
        if ft == "transformer" and n_out > 1:
            ft = "multitransformer"
        return ft

    # Inference table
    if n_in > 0 and n_out == 0:
        return "consumer"
    if n_in == 0 and n_out > 0:
        return "producer"
    if n_in > 0 and n_out == 1:
        return "transformer"
    if n_in > 0 and n_out > 1:
        return "multitransformer"
    raise ValueError(
        "Cannot infer functional type: supply at least one --inputs or --outputs."
    )


def _build_spec(args: argparse.Namespace) -> AlgorithmSpec:
    """
    Convert the raw argparse namespace into a fully validated AlgorithmSpec.
    All string parsing happens here and nowhere else.
    """
    # --- runtime-inputs: parse first so we know which keys are vector -------
    runtime_input_specs: List[RuntimeInputSpec] = [
        RuntimeInputSpec.parse(s) for s in (args.runtime_inputs or [])
    ]
    runtime_input_keys = {rs.data.key for rs in runtime_input_specs}
    runtime_defaults   = {rs.data.key: rs.defaults for rs in runtime_input_specs}

    # --- keyvalues-inputs overrides -----------------------------------------
    kvi_map: dict = {}
    for s in (args.keyvalues_inputs or []):
        parts = s.split(":", 1)
        kvi_map[parts[0]] = parts[1] if len(parts) > 1 else parts[0]

    # --- inputs -------------------------------------------------------------
    inputs: List[DataSpec] = []
    for raw in (args.inputs or []):
        ds = DataSpec.parse(raw)
        if ds.key in runtime_input_keys:
            # Promote to the RuntimeInputSpec's DataSpec (is_vector=True)
            rs = next(r for r in runtime_input_specs if r.data.key == ds.key)
            inputs.append(rs.data)
        elif ds.key in kvi_map or getattr(args, "all_keyvalues", False):
            inputs.append(DataSpec(ds.type_name, ds.key, is_vector=True))
        else:
            inputs.append(ds)

    # --- outputs ------------------------------------------------------------
    outputs: List[DataSpec] = [DataSpec.parse(raw) for raw in (args.outputs or [])]

    # --- runtime output (dynamic vector return) -----------------------------
    runtime_output: Optional[DataSpec] = (
        DataSpec.parse(args.runtime_outputs) if args.runtime_outputs else None
    )

    # --- properties ---------------------------------------------------------
    properties = [PropertySpec.parse(p) for p in (args.properties or [])]

    # --- functional type (inferred or explicit) -----------------------------
    functional_type = _infer_functional_type(
        inputs, outputs, runtime_output,
        explicit=getattr(args, "functional_type", None),
    )

    return AlgorithmSpec(
        class_name      = args.class_name,
        functional_type = functional_type,
        inputs          = inputs,
        outputs         = outputs,
        runtime_output  = runtime_output,
        runtime_defaults= runtime_defaults,
        properties      = properties,
        namespace       = args.namespace or None,
        framework       = args.framework,
        use_class       = args.use_class,
        type_aliases    = getattr(args, "type_aliases", False),
        private_props   = getattr(args, "private_properties", False),
        all_keyvalues   = getattr(args, "all_keyvalues", False),
        event_context   = getattr(args, "event_context", False),
        generate_cmake  = getattr(args, "cmake", False),
        output_file     = args.output_file or f"{args.class_name}.cpp",
        command_line    = " ".join(shlex.quote(a) for a in sys.argv),
    )


# ---------------------------------------------------------------------------
# Jinja2 templates
# ---------------------------------------------------------------------------

_CPP_TEMPLATE = """\
// Generated by Gaudi Functional C++ Class Generator
// Command: {{ spec.command_line }}

{{ includes }}
{% if not spec.is_k4 %}
using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;
{% endif %}
{% if spec.use_ret_type_alias %}
// Which type of collections we are producing
using retType = std::tuple<
{% for out in spec.outputs %}
    {{ out.type_name }}{{ "" if loop.last else "," }}
{% endfor %}
>;
{% endif %}
{% if spec.type_alias_pairs %}
// Which type of collections we are reading
{% for alias, typ in spec.type_alias_pairs %}
using {{ alias }} = {{ typ }};
{% endfor %}
{% endif %}
{% if spec.namespace %}
namespace {{ spec.namespace }} {
{% endif %}
{{ class_kw }} {{ cls }} final : {{ spec.base_full }}<{{ spec.template_signature }}> {
{% if access_kw %}{{ access_kw }}{% endif %}
  // Constructor: KeyValues map to collection names, settable from Python
{{ constructor }}
{% if spec.inputs | selectattr('is_vector') | list %}

  StatusCode initialize() override {
    // Verify input locations are set from Python before the event loop
{% for inp in spec.inputs %}
{% if inp.is_vector %}
    // inputLocations("{{ inp.key }}") -> current list of collection names
{% endif %}
{% endfor %}
    return StatusCode::SUCCESS;
  }
{% endif %}

  // This is the function that will be called to produce the data
  {{ op_signature }} {
{{ op_body }}
  }
{% if spec.properties %}
{% if spec.private_props %}
private:
{% endif %}
{% for prop in spec.properties %}
  Gaudi::Property<{{ prop.type_name }}> {{ prop.member_name }}{
      this, "{{ prop.name }}", {{ prop.default }}{{ ', "' + prop.description + '"' if prop.description else '' }}};
{% endfor %}
{% endif %}
{% if spec.event_context %}

  StatusCode finalize() override {
    // TODO: finalise event-context state
    return StatusCode::SUCCESS;
  }

  mutable std::set<unsigned long> m_eventNumbersSeen{};
  mutable std::mutex              m_mutex{};
{% endif %}
};
{% if spec.namespace %}
} // namespace {{ spec.namespace }}
{% endif %}
DECLARE_COMPONENT({{ cls }})
"""

_CMAKE_TEMPLATE = """\
# Generated by Gaudi Functional C++ Class Generator
# Command: {{ spec.command_line }}

cmake_minimum_required(VERSION 3.15)
project({{ spec.class_name }}Plugin)

{% for pkg in find_packages %}{{ pkg }}
{% endfor %}
gaudi_add_module({{ spec.class_name }}Plugin
  SOURCES {{ spec.class_name }}.cpp
  LINK
{% for lib in link_libs %}    {{ lib }}
{% endfor %})
"""


# ---------------------------------------------------------------------------
# Code generation (pure Python logic, no string surgery)
# ---------------------------------------------------------------------------

def _build_includes(spec: AlgorithmSpec) -> str:
    lines = []

    # Framework header — MultiTransformer lives in Transformer.h for k4FWCore
    header_base = "Transformer" if spec.functional_type == "multitransformer" and spec.is_k4 \
                  else spec.base_short
    if spec.is_k4:
        lines.append(f'#include "k4FWCore/{header_base}.h"')
    else:
        lines.append(f'#include "Gaudi/Functional/{header_base}.h"')

    if spec.properties:
        lines.append('#include "Gaudi/Property.h"')

    edm_headers: set = set()
    podio_needed = False
    all_ds = spec.inputs + spec.outputs + ([spec.runtime_output] if spec.runtime_output else [])
    for ds in all_ds:
        if ds.edm4hep_header:
            edm_headers.add(ds.edm4hep_header)
        if ds.needs_podio_header:
            podio_needed = True

    for h in sorted(edm_headers):
        lines.append(f'#include "edm4hep/{h}"')
    if podio_needed:
        lines.append('#include "podio/UserDataCollection.h"')

    lines.append("#include <string>")
    if spec.event_context:
        lines += ["#include <limits>", "#include <mutex>", "#include <set>"]
    if len(spec.outputs) > 1 and not spec.use_ret_type_alias:
        lines.append("#include <tuple>")
    if spec.is_runtime or any(inp.is_vector for inp in spec.inputs):
        lines.append("#include <vector>")

    return "\n".join(lines)


def _build_constructor(spec: AlgorithmSpec) -> str:
    """Return the full constructor definition (k4FWCore style)."""
    cls  = spec.class_name
    base = spec.base_short
    rd   = spec.runtime_defaults

    def _kv(ds: DataSpec) -> str:
        if ds.is_vector:
            defs_str = ", ".join(f'"{d}"' for d in rd.get(ds.key, [ds.key]))
            return f'KeyValues("{ds.key}", {{{defs_str}}})'
        return f'KeyValue("{ds.key}", "{ds.key}")'

    # Input block
    if not spec.inputs:
        in_block = "{}"
    elif len(spec.inputs) == 1:
        in_block = _kv(spec.inputs[0])
    else:
        ind   = " " * 20
        items = (",\n" + ind).join(_kv(inp) for inp in spec.inputs)
        in_block = "{\n" + ind + items + ",\n" + " " * 16 + "}"

    ft = spec.functional_type

    if ft in ("consumer", "filter"):
        return (
            f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
            f"      : {base}(name, svcLoc, {in_block}) {{}}"
        )

    if spec.is_runtime:
        out_key   = spec.runtime_output.key
        out_block = f'{{KeyValues("OutputCollections", {{"{out_key}"}})}}'
        return (
            f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
            f"      : {base}(name, svcLoc, {in_block}, {out_block}) {{}}"
        )

    if not spec.outputs:
        return (
            f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
            f"      : {base}(name, svcLoc, {in_block}, {{}}) {{}}"
        )

    if len(spec.outputs) == 1:
        out_block = _kv(spec.outputs[0])
        return (
            f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
            f"      : {base}(name, svcLoc, {in_block}, {out_block}) {{}}"
        )

    # Multiple fixed outputs — brace-list of KeyValues
    ind2  = " " * 17
    items = (",\n" + ind2).join(_kv(out) for out in spec.outputs)
    return (
        f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
        f"      : {base}(name, svcLoc, {in_block},\n"
        f"                 {{\n"
        f"                 {items}}}) {{}}"
    )


def _build_constructor_gaudi(spec: AlgorithmSpec) -> str:
    cls  = spec.class_name
    base = spec.base_short
    all_kvs = [f'KeyValue{{"{ds.key}", "{ds.key}"}}' for ds in spec.inputs + spec.outputs]
    if not all_kvs:
        sep, args_str = "", ""
    elif len(all_kvs) == 1:
        sep, args_str = ", ", all_kvs[0]
    else:
        sep, args_str = ", ", "{" + ", ".join(all_kvs) + "}"
    return (
        f"  {cls}(const std::string& name, ISvcLocator* svcLoc)\n"
        f"      : {base}(name, svcLoc{sep}{args_str}) {{}}"
    )


def _build_op_signature(spec: AlgorithmSpec) -> str:
    params = []
    if spec.event_context:
        params.append("const EventContext& ctx")
    for inp in spec.inputs:
        disp = spec.display_type(inp)
        if inp.is_vector:
            params.append(f"const std::vector<const {disp}*>& {inp.key}")
        else:
            params.append(f"const {disp}& {inp.key}")
    return f"{spec.cpp_return_type} operator()({', '.join(params)}) const override"


def _build_op_body(spec: AlgorithmSpec) -> str:
    ft = spec.functional_type

    if spec.is_runtime:
        elem = spec.runtime_output.type_name
        return (
            f"    const auto locs = outputLocations();\n"
            f"    std::vector<{elem}> outputCollections;\n"
            f"    for (size_t i = 0; i < locs.size(); ++i) {{\n"
            f"      auto coll = {elem}();\n"
            f"      // TODO: fill coll\n"
            f"      outputCollections.emplace_back(std::move(coll));\n"
            f"    }}\n"
            f"    return outputCollections;"
        )

    if ft == "consumer":
        lines = []
        if spec.event_context:
            lines.append('    info() << "Event number is " << ctx.evt() << endmsg;')
        for inp in spec.inputs:
            lines += [
                f'    debug() << "Received {inp.key} with " << {inp.key}.size() << " elements" << endmsg;',
                f"    for (const auto& elem : {inp.key}) {{",
                f"      // TODO: process elem",
                f"    }}",
            ]
        return "\n".join(lines) if lines else "    // TODO: implement"

    if ft == "filter":
        return "    // TODO: implement filter logic\n    return false;"

    if len(spec.outputs) == 1:
        return (
            f"    // TODO: implement\n"
            f"    return {spec.outputs[0].type_name}{{}};"
        )

    # Multiple outputs
    lines = []
    for i, out in enumerate(spec.outputs, 1):
        lines.append(f"    auto output{i} = {out.type_name}();")
    lines += [
        "",
        "    // TODO: fill output collections",
        "",
    ]
    moves = ", ".join(f"std::move(output{i})" for i in range(1, len(spec.outputs) + 1))
    lines.append(f"    return std::make_tuple({moves}); // NOLINT")
    return "\n".join(lines)


def _build_cmake_context(spec: AlgorithmSpec) -> dict:
    all_ds = spec.inputs + spec.outputs + ([spec.runtime_output] if spec.runtime_output else [])
    has_edm4hep = any(ds.edm4hep_header for ds in all_ds)
    has_podio   = any(ds.needs_podio_header for ds in all_ds)

    find_packages, link_libs = [], []
    if spec.is_k4:
        find_packages.append("find_package(k4FWCore REQUIRED)")
        link_libs.append("k4FWCore::k4FWCore")
    else:
        find_packages.append("find_package(Gaudi REQUIRED)")
        link_libs += ["Gaudi::GaudiAlgLib", "Gaudi::GaudiKernel"]
    if has_edm4hep:
        find_packages.append("find_package(EDM4HEP REQUIRED)")
        link_libs.append("EDM4HEP::edm4hep")
    if has_podio:
        find_packages.append("find_package(podio REQUIRED)")
        link_libs.append("podio::podio")
    return {"find_packages": find_packages, "link_libs": link_libs}


# ---------------------------------------------------------------------------
# Top-level generate()
# ---------------------------------------------------------------------------

def generate(spec: AlgorithmSpec) -> tuple:
    """
    Render the C++ source (and optionally CMakeLists.txt) for *spec*.
    Returns (cpp_source, cmake_source_or_None).
    """
    env = Environment(
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
        undefined=StrictUndefined,
    )

    constructor = (
        _build_constructor(spec) if spec.is_k4
        else _build_constructor_gaudi(spec)
    )

    cpp_ctx = {
        "spec":         spec,
        "cls":          spec.class_name,
        "includes":     _build_includes(spec),
        "constructor":  constructor,
        "op_signature": _build_op_signature(spec),
        "op_body":      _build_op_body(spec),
        "class_kw":     "struct" if not spec.use_class else "class",
        "access_kw":    "public:\n" if spec.use_class else "",
    }

    cpp_source = env.from_string(_CPP_TEMPLATE).render(**cpp_ctx).lstrip("\n")

    cmake_source = None
    if spec.generate_cmake:
        cmake_ctx = {"spec": spec, **_build_cmake_context(spec)}
        cmake_source = env.from_string(_CMAKE_TEMPLATE).render(**cmake_ctx)

    return cpp_source, cmake_source


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="gaudi_gen.py",
        description="Generate Gaudi Functional C++ algorithm boilerplate.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
Functional type is inferred from the number of inputs and outputs:
  consumer         inputs > 0, outputs == 0
  producer         inputs == 0, outputs >= 1
  transformer      inputs >= 1, outputs == 1
  multitransformer inputs >= 1, outputs > 1
  filter           inputs >= 1, supply --type filter explicitly

Examples:
  # k4FWCore producer (type inferred)
  gaudi_gen.py MyProducer -o 'edm4hep::MCParticleCollection:MCParticles'

  # k4FWCore multi-output producer with properties
  gaudi_gen.py MyProducer \\
      -o 'edm4hep::MCParticleCollection:MCParticles' \\
         'edm4hep::TrackCollection:Tracks' \\
      -p 'int:ExampleInt:3:An example integer property'

  # Gaudi transformer with namespace (type inferred)
  gaudi_gen.py MySum -i 'Input1:Loc1' 'Input2:Loc2' -o 'Output:OutLoc' \\
      --framework gaudi -n MyNamespace

  # Variable-length inputs (k4FWCore only)
  gaudi_gen.py MyVarConsumer \\
      -i 'edm4hep::MCParticleCollection:Inputs' \\
      --runtime-inputs 'edm4hep::MCParticleCollection:Inputs:MCParticles0,MCParticles1'
""",
    )
    parser.add_argument("class_name", help="Name of the C++ class to generate")
    parser.add_argument(
        "functional_type", nargs="?",
        choices=["consumer", "producer", "transformer", "filter"],
        help="Functional type (inferred from I/O counts when omitted)",
    )
    parser.add_argument("-i", "--inputs",  nargs="*", default=[], metavar="TYPE:KEY")
    parser.add_argument("-o", "--outputs", nargs="*", default=[], metavar="TYPE:KEY")
    parser.add_argument(
        "--runtime-outputs", dest="runtime_outputs", default=None, metavar="TYPE",
        help="Enable dynamic output collections returning std::vector<TYPE>",
    )
    parser.add_argument("-p", "--properties", nargs="*", default=[], metavar="TYPE:NAME:DEFAULT[:DESC]")
    parser.add_argument("-n", "--namespace", default="")
    parser.add_argument("--framework", choices=["gaudi", "k4fwcore"], default="k4fwcore")
    parser.add_argument("--use-class",          dest="use_class",          action="store_true", default=False)
    parser.add_argument("-f", "--output-file",   dest="output_file",        default=None)
    parser.add_argument("--type-aliases",        dest="type_aliases",       action="store_true", default=False)
    parser.add_argument("--private-properties",  dest="private_properties", action="store_true", default=False)
    parser.add_argument("--all-keyvalues",       dest="all_keyvalues",      action="store_true", default=False)
    parser.add_argument("--keyvalues-inputs",    dest="keyvalues_inputs",   nargs="*", default=None)
    parser.add_argument("--runtime-inputs",      dest="runtime_inputs",     nargs="*", default=None)
    parser.add_argument("--event-context",       dest="event_context",      action="store_true", default=False)
    parser.add_argument("--cmake", action="store_true", default=False)
    return parser


def main() -> None:
    parser = _build_parser()
    args = parser.parse_args()

    if args.runtime_outputs and args.outputs:
        parser.error("--runtime-outputs and --outputs are mutually exclusive.")
    if args.runtime_outputs and args.framework != "k4fwcore":
        parser.error("--runtime-outputs is only supported with --framework k4fwcore.")

    try:
        spec = _build_spec(args)
    except ValueError as exc:
        parser.error(str(exc))
        return

    cpp_source, cmake_source = generate(spec)

    with open(spec.output_file, "w") as fh:
        fh.write(cpp_source)
    print(f"Written to {spec.output_file}", file=sys.stderr)

    if cmake_source is not None:
        with open("CMakeLists.txt", "w") as fh:
            fh.write(cmake_source)
        print("Written to CMakeLists.txt", file=sys.stderr)


if __name__ == "__main__":
    main()
