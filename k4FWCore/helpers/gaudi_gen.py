#!/usr/bin/env python3
"""
Gaudi Functional C++ Class Generator
"""

import argparse
import sys
import re
import shlex
from typing import List, Tuple

# Configuration Constants
FUNCTIONAL_TYPES = {
    'consumer':    {'base': 'Consumer',        'desc': 'In -> Void'},
    'producer':    {'base': 'Producer',        'desc': 'Void -> Out'},
    'transformer': {'base': 'Transformer',     'desc': 'In -> Out'},
    'filter':      {'base': 'FilterPredicate', 'desc': 'In -> Bool'}
}


class GaudiGen:
    def __init__(self, args):
        self.args = args
        self.command_line = ' '.join(shlex.quote(arg) for arg in sys.argv)

    def parse_data_spec(self, spec: str) -> Tuple[str, str]:
        """
        Splits TYPE:key at the last separator colon outside angle brackets.
        Skips C++ namespace '::' tokens so edm4hep::Foo:key splits correctly.
        """
        depth, colon_pos = 0, -1
        for i, char in enumerate(spec):
            if char == '<':
                depth += 1
            elif char == '>':
                depth -= 1
            elif char == ':' and depth == 0:
                prev_colon = (i > 0 and spec[i - 1] == ':')
                next_colon = (i + 1 < len(spec) and spec[i + 1] == ':')
                if not prev_colon and not next_colon:
                    colon_pos = i
        return (spec, '') if colon_pos == -1 else (spec[:colon_pos], spec[colon_pos + 1:])

    def parse_runtime_input_spec(self, spec: str):
        """
        Parses a --runtime-inputs spec of the form:
            TYPE:KEY:Default0[,Default1,...]
        Returns (type_str, key_str, [default, ...])
        Uses the FIRST separator colon (not inside <>) as the TYPE/KEY boundary,
        and the second separator colon as the KEY/defaults boundary.
        """
        # Find all separator colons (not inside <>, not part of ::)
        depth, sep_positions = 0, []
        for i, ch in enumerate(spec):
            if ch == '<': depth += 1
            elif ch == '>': depth -= 1
            elif ch == ':' and depth == 0:
                prev = (i > 0 and spec[i-1] == ':')
                nxt  = (i+1 < len(spec) and spec[i+1] == ':')
                if not prev and not nxt:
                    sep_positions.append(i)
        if len(sep_positions) == 0:
            return spec, self._default_key(spec), [self._default_key(spec)]
        if len(sep_positions) == 1:
            p = sep_positions[0]
            typ, key = spec[:p], spec[p+1:]
            return typ, key, [key]
        # Two or more: first split = TYPE, second split = KEY, rest = defaults
        p0, p1 = sep_positions[0], sep_positions[1]
        typ      = spec[:p0]
        key      = spec[p0+1:p1]
        defaults = [d.strip() for d in spec[p1+1:].split(',')]
        return typ, key, defaults

    def _default_key(self, typ: str) -> str:
        """e.g. edm4hep::MCParticleCollection -> MCParticles"""
        base = typ.split('::')[-1]
        base = re.sub(r'Collection$', '', base)
        return base + 's'

    # ------------------------------------------------------------------
    # Mode helpers
    # ------------------------------------------------------------------
    def _is_k4(self) -> bool:
        return self.args.framework == 'k4fwcore'

    def _base_class_alias(self) -> str:
        """Emits the BaseClass_t alias for native Gaudi Functional."""
        return "using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;"

    def _use_ret_type_alias(self) -> bool:
        """True for k4fwcore with multiple *fixed* outputs (not runtime)."""
        return self._is_k4() and not self.args.runtime_outputs and len(self.args.outputs) > 1

    def _is_runtime(self) -> bool:
        """True when --runtime-outputs is set (dynamic vector return)."""
        return bool(self.args.runtime_outputs)

    # ------------------------------------------------------------------
    # Type aliases
    # ------------------------------------------------------------------
    def _collect_type_aliases(self) -> List[Tuple[str, str]]:
        """
        Returns an ordered list of (alias, full_type) for each unique input type.

        Alias strategy (mirrors k4FWCore reference examples):
          podio::UserDataCollection<float> -> FloatColl
          edm4hep::MCParticleCollection    -> ParticleColl
          edm4hep::SimTrackerHitCollection -> SimTrackerHitColl
          edm4hep::TrackerHit3DCollection  -> TrackerHitColl
          edm4hep::TrackCollection         -> TrackColl
          edm4hep::ReconstructedParticleCollection -> RecoColl
          edm4hep::RecoMCParticleLinkCollection    -> LinkColl

        General rule:
          1. Template types (UserDataCollection<T>): use T.capitalize() + Coll
          2. Names ending in 'LinkCollection': strip to stem before 'Link' + LinkColl -> LinkColl
          3. Names ending in 'HitCollection' : strip 'Collection', keep full stem + Coll
          4. Names ending in 'Collection': strip 'Collection' + last CamelCase word + Coll
          Duplicates get a numeric suffix.
        """
        seen_types: dict = {}
        aliases: List[Tuple[str, str]] = []
        for spec in self.args.inputs:
            typ, _ = self.parse_data_spec(spec)
            if typ in seen_types:
                continue
            # Template types: podio::UserDataCollection<float> -> FloatColl
            inner = re.search(r'<([^>]+)>', typ)
            if inner:
                base = inner.group(1).strip().split('::')[-1].capitalize()
            else:
                last = typ.split('::')[-1]              # e.g. SimTrackerHitCollection
                stem = re.sub(r'Collection$', '', last) # e.g. SimTrackerHit
                # LinkCollection -> LinkColl
                if stem.endswith('Link'):
                    base = 'Link'
                # *HitCollection or *Hit3DCollection -> keep full stem
                elif re.search(r'Hit', stem):
                    # TrackerHit3D -> TrackerHit, SimTrackerHit -> SimTrackerHit
                    base = re.sub(r'\d+[A-Z]?$', '', stem) or stem
                # Reconstructed* -> Reco, otherwise last CamelCase word
                elif stem.startswith('Reconstructed'):
                    base = 'Reco'
                else:
                    words = re.findall(r'[A-Z][a-z0-9]*', stem)
                    base  = words[-1] if words else stem
            alias = base + 'Coll'
            existing_aliases = {a for a, _ in aliases}
            suffix, candidate = 2, alias
            while candidate in existing_aliases:
                candidate = alias + str(suffix); suffix += 1
            alias = candidate
            seen_types[typ] = alias
            aliases.append((alias, typ))
        return aliases

    def _alias_for(self, typ: str, alias_map: dict) -> str:
        return alias_map.get(typ, typ)

    # ------------------------------------------------------------------
    # Includes
    # ------------------------------------------------------------------
    def get_includes(self) -> str:
        inc = []

        if self._is_k4():
            base = FUNCTIONAL_TYPES[self.args.functional_type]['base']
            inc.append(f'#include "k4FWCore/{base}.h"')
        else:
            base = FUNCTIONAL_TYPES[self.args.functional_type]['base']
            inc.append(f'#include "Gaudi/Functional/{base}.h"')

        if self.args.properties:
            inc.append('#include "Gaudi/Property.h"')

        type_inc = set()
        # Collect all type specs: fixed outputs, inputs, and runtime output type
        all_specs = list(self.args.inputs) + list(self.args.outputs)
        if self._is_runtime():
            all_specs.append(self.args.runtime_outputs)  # bare type, no key

        for spec in all_specs:
            typ, _ = self.parse_data_spec(spec)
            if 'podio::UserDataCollection' in typ:
                type_inc.add('#include "podio/UserDataCollection.h"')
            if 'edm4hep::' in typ:
                for match in re.findall(r'edm4hep::(\w+Collection)', typ):
                    type_inc.add(f'#include "edm4hep/{match}.h"')

        inc.extend(sorted(type_inc))
        inc.append('#include <string>')
        if self.args.functional_type == 'consumer':
            inc.append('#include <sstream>')
            inc.append('#include <stdexcept>')
        if getattr(self.args, 'event_context', False):
            inc.extend(['#include <limits>', '#include <mutex>', '#include <set>'])
        if len(self.args.outputs) > 1:
            inc.append('#include <tuple>')
        # runtime mode needs <vector>
        if self._is_runtime() or getattr(self.args, 'runtime_inputs', None):
            inc.append('#include <vector>')
        return '\n'.join(inc)

    # ------------------------------------------------------------------
    # retType alias  (multi-output k4fwcore, fixed collections only)
    # ------------------------------------------------------------------
    def _ret_type_alias(self) -> str:
        out_types = [self.parse_data_spec(o)[0] for o in self.args.outputs]
        # Align continuation lines under the first type (after 'std::tuple<')
        prefix = '    std::tuple<'
        indent = ' ' * len(prefix)
        joined = (',\n' + indent).join(out_types)
        return f'using retType =\n{prefix}{joined}>;'

    # ------------------------------------------------------------------
    # C++ return type string used in signature and operator()
    # ------------------------------------------------------------------
    def _cpp_return_type(self) -> str:
        f_type    = self.args.functional_type
        out_types = [self.parse_data_spec(o)[0] for o in self.args.outputs]

        if f_type == 'consumer':
            return 'void'
        if f_type == 'filter':
            return 'bool'
        if self._is_runtime():
            elem = self.parse_data_spec(self.args.runtime_outputs)[0]
            return f'std::vector<{elem}>'
        if self._use_ret_type_alias():
            return 'retType'
        if len(out_types) == 1:
            return out_types[0]
        return f"std::tuple<{', '.join(out_types)}>"

    # ------------------------------------------------------------------
    # Template signature  e.g.  Out(const In1&, const In2&)
    # ------------------------------------------------------------------
    def generate_signature(self) -> str:
        runtime_in_keys = {self.parse_runtime_input_spec(s)[1]
                           for s in getattr(self.args, 'runtime_inputs', []) or []}
        use_aliases = getattr(self.args, 'type_aliases', False)
        alias_map   = {typ: alias for alias, typ in self._collect_type_aliases()} if use_aliases else {}
        in_t = []
        for spec in self.args.inputs:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            disp = self._alias_for(typ, alias_map)
            if key in runtime_in_keys:
                in_t.append(f'const std::vector<const {disp}*>&')
            else:
                in_t.append(f'const {disp}&')
        if getattr(self.args, 'event_context', False):
            in_t = ['const EventContext&'] + in_t
        ret  = self._cpp_return_type()
        sig  = f"{ret}({', '.join(in_t)})"
        if not self._is_k4():
            return f"{sig}, BaseClass_t"
        return sig

    # ------------------------------------------------------------------
    # Constructor
    # ------------------------------------------------------------------
    def _build_constructor_k4fwcore(self) -> str:
        cls        = self.args.class_name
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
        f_type     = self.args.functional_type

        # ---- inputs block ----
        # runtime_inputs entries use KeyValues(name, {defaults...}),
        # plain inputs use KeyValue(name, default).
        # Mixed case always uses a brace-list.
        # Build mapping key -> defaults list from --runtime-inputs specs
        ri_map = {}  # key -> [default, ...]
        for s in (getattr(self.args, 'runtime_inputs', []) or []):
            _, key, defaults = self.parse_runtime_input_spec(s)
            ri_map[key] = defaults
        ri_keys = set(ri_map.keys())

        # Build mapping key -> default from --keyvalues-inputs specs
        # These use KeyValues(name, {default}) but operator() type is still const TYPE&
        kvi_map = {}  # key -> default string
        for s in (getattr(self.args, 'keyvalues_inputs', []) or []):
            parts = s.split(':', 1)
            k   = parts[0]
            val = parts[1] if len(parts) > 1 else k
            kvi_map[k] = val
        kvi_keys = set(kvi_map.keys())

        if not self.args.inputs:
            in_block = '{}'
        else:
            kv_list = []
            has_runtime_input = False
            for spec in self.args.inputs:
                typ, key = self.parse_data_spec(spec)
                key = key if key else self._default_key(typ)
                if key in ri_keys:
                    defaults = ri_map.get(key, [key])
                    defs_str = ', '.join(f'"{d}"' for d in defaults)
                    kv_list.append(f'KeyValues("{key}", {{{defs_str}}})')
                    has_runtime_input = True
                elif key in kvi_keys:
                    default = kvi_map.get(key, key)
                    kv_list.append(f'KeyValues("{key}", {{"{default}"}})')
                elif getattr(self.args, 'all_keyvalues', False):
                    kv_list.append(f'KeyValues("{key}", {{"{key}"}})')
                else:
                    kv_list.append(f'KeyValue("{key}", "{key}")')
            # Single entry (KeyValue or KeyValues) stays bare; multiple use brace-list
            if len(kv_list) == 1:
                in_block = kv_list[0]
            else:
                indent = ' ' * 20
                in_block = '{\n' + indent + (',\n' + indent).join(kv_list) + ',\n' + ' ' * 16 + '}'

        # ---- outputs block ----
        if f_type in ('consumer', 'filter'):
            # Consumer/filter have only inputs; pass bare KV or brace-list
            ctor_args = in_block

        elif self._is_runtime():
            # Runtime: always KeyValues(...) in a brace-list, even if only one entry
            # The property name is e.g. "OutputCollections"; the default value is
            # the bare collection name without a trailing 's' added again.
            typ, key = self.parse_data_spec(self.args.runtime_outputs)
            key       = key if key else self._default_key(typ)
            out_block = f'{{KeyValues("OutputCollections", {{"{key}"}})}}'
            ctor_args = f'{in_block}, {out_block}'

        elif not self.args.outputs:
            ctor_args = f'{in_block}, {{}}'

        elif len(self.args.outputs) > 1:
            # Multiple fixed outputs: brace-list of KeyValues(...)
            # Reference style:
            #   : Producer(name, svcLoc, {},
            #              {
            #              KeyValues("A", {"a"}),
            #              KeyValues("B", {"b"})}) {}
            kv_items  = []
            for spec in self.args.outputs:
                typ, key = self.parse_data_spec(spec)
                key = key if key else self._default_key(typ)
                kv_items.append(f'KeyValues("{key}", {{"{key}"}})')
            # Reference layout (17 spaces aligns all KeyValues entries):
            #   : Producer(name, svcLoc, {},
            #              {
            #              KeyValues("A", {"a"}),
            #              KeyValues("B", {"b"})}) {}
            kv_ind = ' ' * 17
            joined = (',\n' + kv_ind).join(kv_items)
            cls_   = self.args.class_name
            return (
                f'  {cls_}(const std::string& name, ISvcLocator* svcLoc)\n'
                f'      : {base_short}(name, svcLoc, {in_block},\n'
                f'                 {{\n'
                f'                 {joined}}})'
                f' {{}}'
            )

        else:
            # Single fixed output: bare KeyValue(...)
            typ, key  = self.parse_data_spec(self.args.outputs[0])
            key        = key if key else self._default_key(typ)
            out_block  = f'KeyValue("{key}", "{key}")'
            ctor_args  = f'{in_block}, {out_block}'

        return (
            f'  {cls}(const std::string& name, ISvcLocator* svcLoc)\n'
            f'      : {base_short}(name, svcLoc, {ctor_args}) {{}}'
        )

    def _build_constructor_gaudi(self) -> str:
        """
        Native Gaudi Functional constructor.
        Single KV  -> : Producer(name, svcLoc, KeyValue{"Name", "Default"})
        Multiple   -> : Producer(name, svcLoc, {KeyValue{"A","a"}, KeyValue{"B","b"}})
        """
        cls        = self.args.class_name
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
        f_type     = self.args.functional_type

        def kv(spec: str) -> str:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            return f'KeyValue{{"{ key }", "{ key }"}}'

        in_kvs  = [kv(s) for s in self.args.inputs]
        out_kvs = [kv(s) for s in self.args.outputs]
        all_kvs = in_kvs + out_kvs

        if not all_kvs:
            ctor_args = ''
        elif len(all_kvs) == 1:
            ctor_args = all_kvs[0]
        else:
            ctor_args = '{' + ', '.join(all_kvs) + '}'

        sep = ', ' if ctor_args else ''
        return (
            f'  {cls}(const std::string& name, ISvcLocator* svcLoc)\n'
            f'      : {base_short}(name, svcLoc{sep}{ctor_args}) {{}}'
        )

    # ------------------------------------------------------------------
    # operator() signature
    # ------------------------------------------------------------------
    def generate_op_sig(self) -> str:
        f_type    = self.args.functional_type
        ret       = self._cpp_return_type()

        runtime_in_keys = {self.parse_runtime_input_spec(s)[1]
                            for s in getattr(self.args, 'runtime_inputs', []) or []}
        use_aliases = getattr(self.args, 'type_aliases', False)
        alias_map   = {typ: alias for alias, typ in self._collect_type_aliases()} if use_aliases else {}
        params = []
        if getattr(self.args, 'event_context', False):
            params.append('const EventContext& ctx')
        for spec in self.args.inputs:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            disp = self._alias_for(typ, alias_map)
            if key in runtime_in_keys:
                params.append(f'const std::vector<const {disp}*>& {key}')
            else:
                params.append(f'const {disp}& {key}')

        param_str = ', '.join(params)
        return f'{ret} operator()({param_str}) const override'

    def _default_return(self) -> str:
        f_type    = self.args.functional_type
        out_types = [self.parse_data_spec(o)[0] for o in self.args.outputs]

        if f_type == 'consumer':
            return ''
        if f_type == 'filter':
            return 'return false;'
        if self._is_runtime():
            elem = self.parse_data_spec(self.args.runtime_outputs)[0]
            return f'return std::vector<{elem}>{{}};'
        if len(out_types) == 1:
            return f'return {out_types[0]}{{}};'
        # Multi-output: named locals + std::move (mirrors reference)
        lines = []
        for i, t in enumerate(out_types, 1):
            lines.append(f'    auto output{i} = {t}();')
        lines.append('')
        lines.append('    // TODO: Fill output collections')
        lines.append('')
        moves = ', '.join(f'std::move(output{i})' for i in range(1, len(out_types)+1))
        lines.append(f'    return std::make_tuple({moves});  // NOLINT')
        return '\n'.join(lines)

    # ------------------------------------------------------------------
    # Runtime operator() body hint
    # ------------------------------------------------------------------
    def _initialize_hint(self) -> List[str]:
        """Emits an initialize() override skeleton when runtime inputs are present."""
        ri = getattr(self.args, 'runtime_inputs', None) or []
        if not ri:
            return []
        lines = [
            "  StatusCode initialize() override {",
            "    // Verify input locations set from Python",
        ]
        for i, spec in enumerate(self.args.inputs):
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            lines += [
                f'    // inputLocations({i}) or inputLocations("{key}") -> names of {key}',
            ]
        lines += [
            "    return StatusCode::SUCCESS;",
            "  }",
        ]
        return lines

    def _consumer_body_hint(self) -> str:
        """Skeleton body for Consumer showing Gaudi messaging and iteration idioms."""
        lines = []
        if getattr(self.args, 'event_context', False):
            lines.append('    info() << "Event number is " << ctx.evt() << endmsg;')
        if not self.args.inputs:
            lines.append('    // TODO: Implement consumer logic')
            return '\n'.join(lines)
        for spec in self.args.inputs:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            lines += [
                f'    debug() << "Received {key} with " << {key}.size() << " elements" << endmsg;',
                f'    for (const auto& elem : {key}) {{',
                f'      // TODO: process elem',
                f'    }}',
            ]
        return '\n'.join(lines)

    def _runtime_body_hint(self) -> str:
        """Emits a skeleton body showing outputLocations() usage."""
        elem = self.parse_data_spec(self.args.runtime_outputs)[0]
        return (
            f'    const auto locs = outputLocations();\n'
            f'    std::vector<{elem}> outputCollections;\n'
            f'    for (size_t i = 0; i < locs.size(); ++i) {{\n'
            f'      auto coll = {elem}();\n'
            f'      // TODO: fill coll\n'
            f'      outputCollections.emplace_back(std::move(coll));\n'
            f'    }}\n'
            f'    return outputCollections;'
        )

    # ------------------------------------------------------------------
    # Private members  (Gaudi only)
    # ------------------------------------------------------------------
    def _build_data_handle_members(self) -> str:
        lines = []
        for spec in self.args.inputs:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            lines.append(
                f'  DataObjectReadHandle<{typ}>  m_input_{key}'
                f'{{this, "{key}", "{key}"}};'
            )
        for spec in self.args.outputs:
            typ, key = self.parse_data_spec(spec)
            key = key if key else self._default_key(typ)
            lines.append(
                f'  DataObjectWriteHandle<{typ}> m_output_{key}'
                f'{{this, "{key}", "{key}"}};'
            )
        return '\n'.join(lines)

    # ------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------
    def _build_properties(self) -> str:
        lines = []
        for prop in self.args.properties:
            parts    = prop.split(':', 3)
            # Format: type:name:default:desc
            ptype    = parts[0]
            pname    = parts[1] if len(parts) > 1 else parts[0]
            pdefault = parts[2] if len(parts) > 2 else '0'
            pdesc    = parts[3] if len(parts) > 3 else f'Example {pname} property'
            # Avoid double m_ prefix if the user already included it
            member = pname if pname.startswith('m_') else f'm_{pname}'
            lines.append(
                f'  Gaudi::Property<{ptype}> {member}'
                f'{{this, "{pname}", {pdefault}, "{pdesc}"}};'
            )
        return '\n'.join(lines)

    # ------------------------------------------------------------------
    # Top-level code generator
    # ------------------------------------------------------------------
    def generate_code(self) -> str:
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
        base_full  = f"{'k4FWCore' if self._is_k4() else 'Gaudi::Functional'}::{base_short}"

        kw     = "struct" if not self.args.use_class else "class"
        access = "public:\n  " if self.args.use_class else ""

        ctor = (
            self._build_constructor_k4fwcore()
            if self._is_k4()
            else self._build_constructor_gaudi()
        )

        op_sig      = self.generate_op_sig()
        ns_open     = f'namespace {self.args.namespace} {{\n' if self.args.namespace else ''
        ns_close    = f'\n}} // namespace {self.args.namespace}\n' if self.args.namespace else '\n'

        lines = [
            "// Generated by Gaudi Functional C++ Class Generator",
            f"// Command: {self.command_line}",
            "",
            self.get_includes(),
        ]

        if not self._is_k4():
            lines += ["", self._base_class_alias()]
        if self._use_ret_type_alias():
            lines += ["", "// Which type of collections we are producing", self._ret_type_alias()]
        if getattr(self.args, 'type_aliases', False):
            aliases = self._collect_type_aliases()
            if aliases:
                lines.append("")
                lines.append("// Which type of collections we are reading")
                for alias, typ in aliases:
                    lines.append(f"using {alias} = {typ};")

        lines += [
            "",
            f"{ns_open}{kw} {self.args.class_name} final"
            f" : {base_full}<{self.generate_signature()}> {{",
            f"  {access}// Constructor: KeyValues map to collection names, settable from Python",
            ctor,
        ]

        # initialize() skeleton for runtime-input consumers
        init_lines = self._initialize_hint()
        if init_lines:
            lines += init_lines

        lines += [
            f"  // This is the function that will be called to produce the data",
            f"  {op_sig} {{",
        ]

        if self._is_runtime():
            lines.append('  ' + self._runtime_body_hint().replace('\n', '\n  '))
        elif self.args.functional_type == 'consumer':
            lines.append(self._consumer_body_hint())
        else:
            default_ret = self._default_return()
            if '\n' in default_ret:
                lines.append(default_ret)
            else:
                ret_line = f'\n    {default_ret}' if default_ret else ''
                lines.append(f'    // TODO: Implement logic{ret_line}')

        lines.append("  }")

        # Emit properties: under private: if --private-properties, else public
        if self.args.properties:
            if getattr(self.args, 'private_properties', False):
                lines += ["", "private:", self._build_properties()]
            else:
                lines += ["", self._build_properties()]

        # Event-context: add finalize() override and mutable threading members
        if getattr(self.args, 'event_context', False):
            lines += [
                "",
                "  StatusCode finalize() override {",
                "    // TODO: finalise event-context state",
                "    return StatusCode::SUCCESS;",
                "  }",
                "",
                "  mutable std::set<unsigned long> m_eventNumbersSeen{};",
                "  mutable std::mutex              m_mutex{};",
            ]

        lines += [
            f"}};" + ns_close,
            f"DECLARE_COMPONENT({self.args.class_name})",
        ]

        return '\n'.join(lines)

    # ------------------------------------------------------------------
    # CMake generator
    # ------------------------------------------------------------------
    def generate_cmake(self) -> str:
        cls = self.args.class_name
        src = f'{cls}.cpp'

        all_specs   = list(self.args.inputs) + list(self.args.outputs)
        if self._is_runtime():
            all_specs.append(self.args.runtime_outputs)
        has_edm4hep = any('edm4hep::' in self.parse_data_spec(s)[0] for s in all_specs)
        has_podio   = any('podio::'   in self.parse_data_spec(s)[0] for s in all_specs)

        find_pkgs: List[str] = []
        link_libs: List[str] = []

        if self._is_k4():
            find_pkgs.append('find_package(k4FWCore REQUIRED)')
            link_libs.append('k4FWCore::k4FWCore')
        else:
            find_pkgs.append('find_package(Gaudi REQUIRED)')
            link_libs += ['Gaudi::GaudiAlgLib', 'Gaudi::GaudiKernel']

        if has_edm4hep:
            find_pkgs.append('find_package(EDM4HEP REQUIRED)')
            link_libs.append('EDM4HEP::edm4hep')
        if has_podio:
            find_pkgs.append('find_package(podio REQUIRED)')
            link_libs.append('podio::podio')

        find_block = '\n'.join(find_pkgs)
        link_block = '\n    '.join(link_libs)

        return (
            f"# Generated by Gaudi Gen\n"
            f"# Command: {self.command_line}\n"
            f"\n"
            f"cmake_minimum_required(VERSION 3.15)\n"
            f"project({cls}Plugin)\n"
            f"\n"
            f"{find_block}\n"
            f"\n"
            f"gaudi_add_module({cls}Plugin\n"
            f"  SOURCES {src}\n"
            f"  LINK\n"
            f"    {link_block}\n"
            f")\n"
        )



# ======================================================================
# C++ formatter
# ======================================================================

def _split_top_level(s: str, delim: str = ',', openers: str = '<([{', closers: str = '>)]}') -> List[str]:
    """Split string at top-level delimiter (not inside brackets)."""
    parts, depth, buf = [], 0, ''
    i = 0
    while i < len(s):
        c = s[i]
        if c in openers:   depth += 1
        elif c in closers: depth -= 1
        if s[i:i+len(delim)] == delim and depth == 0:
            parts.append(buf); buf = ''; i += len(delim); continue
        buf += c; i += 1
    parts.append(buf)
    return parts


def _find_matching(s: str, start: int, open_c: str, close_c: str) -> int:
    """Return index of the closing char matching open_c at s[start]."""
    depth = 0
    for i in range(start, len(s)):
        if s[i] == open_c:   depth += 1
        elif s[i] == close_c:
            depth -= 1
            if depth == 0: return i
    return -1


def _wrap_angle_args(line: str, indent: str, col: int) -> str:
    """
    Reformat  prefix<A, B, C>suffix  as
      prefix<
        A,
        B,
        C>suffix
    where indent is the base indent of the continuation lines.
    """
    lt = line.find('<')
    if lt == -1: return line
    gt = _find_matching(line, lt, '<', '>')
    if gt == -1: return line
    inner  = line[lt+1:gt]
    parts  = [p.strip() for p in _split_top_level(inner)]
    if len(parts) <= 1: return line
    cont   = indent + '    '
    joined = (',\n' + cont).join(parts)
    return line[:lt+1] + '\n' + cont + joined + '\n' + indent + line[gt:]


def _wrap_paren_params(line: str, col: int) -> str:
    """
    Wrap the LAST top-level (...) parameter list on the line.
    Uses 6-space indent for continuation (matches reference style).
    """
    # Find first ( that is not inside < >
    depth_a, paren_s = 0, -1
    for i, c in enumerate(line):
        if c == '<': depth_a += 1
        elif c == '>': depth_a -= 1
        elif c == '(' and depth_a == 0: paren_s = i; break
    if paren_s == -1: return line
    paren_e = _find_matching(line, paren_s, '(', ')')
    if paren_e == -1: return line
    inner  = line[paren_s+1:paren_e]
    parts  = [p.strip() for p in _split_top_level(inner)]
    if len(parts) <= 1: return line
    suffix = line[paren_e+1:]
    base   = len(line) - len(line.lstrip())
    cont   = ' ' * (base + 6)
    joined = (',\n' + cont).join(parts)
    return line[:paren_s+1] + '\n' + cont + joined + suffix


def format_cpp(code: str, col: int = 100) -> str:
    """
    Wraps long lines in generated C++ to stay within `col` characters.

    Rules applied in order per line:
    1. struct Foo final : Base<Sig, BaseClass_t> {  -> split inheritance + wrap <>
    2. operator()(...) with long params             -> wrap param list
    3. constructor / method with long params        -> wrap param list
    4. Any line still over col with <>              -> wrap angle args
    """
    out = []
    for raw in code.splitlines():
        # NOLINT tag: emit as-is (strip the tag first)
        if 'NOLINT' in raw:
            out.append(raw.replace('  // NOLINT', '')); continue
        if len(raw) <= col:
            out.append(raw); continue

        line = raw
        base_indent = ' ' * (len(line) - len(line.lstrip()))

        # ── Rule 1: struct Foo final : Base<Sig> { ────────────────────
        m = re.match(r'^(struct \S+ final)\s*:\s*(.+)$', line)
        if m:
            struct_kw   = m.group(1)
            inheritance = m.group(2).rstrip()
            cont        = '    '  # 4-space indent before ':'
            inh_line    = cont + ': ' + inheritance
            # Wrap angle args inside the inheritance line if needed
            if len(inh_line) > col and '<' in inh_line:
                lt = inh_line.find('<')
                gt = _find_matching(inh_line, lt, '<', '>')
                if gt != -1:
                    inner = inh_line[lt+1:gt]
                    # The sig looks like: RetType(param1, param2, ...)[, BaseClass_t]
                    # Split at top level to get [sig_part, BaseClass_t?]
                    top_parts = [p.strip() for p in _split_top_level(inner)]
                    # Expand the first part (the function signature) at its '('
                    sig_part = top_parts[0]
                    paren_s = sig_part.find('(')
                    if paren_s != -1:
                        paren_e = _find_matching(sig_part, paren_s, '(', ')')
                        if paren_e != -1:
                            sig_inner = sig_part[paren_s+1:paren_e]
                            sig_params = [p.strip() for p in _split_top_level(sig_inner)]
                            if len(sig_params) > 1:
                                inner_indent = cont + '          '  # deep indent
                                sig_joined   = (',\n' + inner_indent).join(sig_params)
                                sig_part = sig_part[:paren_s+1] + '\n' + inner_indent + sig_joined + sig_part[paren_e:]
                    top_parts[0] = sig_part
                    outer_indent = cont + '      '
                    if len(top_parts) > 1:
                        joined = (',\n' + outer_indent).join(top_parts)
                    else:
                        joined = top_parts[0]
                    inh_line = inh_line[:lt+1] + '\n' + outer_indent + joined + inh_line[gt:]
            for sub in (struct_kw + '\n' + inh_line).splitlines():
                out.append(sub)
            continue

        # ── Rule 2: operator()(params) ────────────────────────────────
        # operator()() has two () pairs; we want the SECOND (the params)
        if 'operator()' in line:
            # Find the paren AFTER 'operator()'
            op_pos  = line.index('operator()')
            search_from = op_pos + len('operator()')
            paren_s = -1
            depth_a = 0
            for i in range(search_from, len(line)):
                c = line[i]
                if c == '<': depth_a += 1
                elif c == '>': depth_a -= 1
                elif c == '(' and depth_a == 0: paren_s = i; break
            if paren_s != -1:
                paren_e = _find_matching(line, paren_s, '(', ')')
                if paren_e != -1:
                    inner  = line[paren_s+1:paren_e]
                    parts  = [p.strip() for p in _split_top_level(inner)]
                    if len(parts) > 1:
                        bi    = len(line) - len(line.lstrip())
                        cont  = ' ' * (bi + 6)
                        joined = (',\n' + cont).join(parts)
                        line   = line[:paren_s+1] + '\n' + cont + joined + line[paren_e:]
                        for sub in line.splitlines():
                            out.append(sub)
                        continue

        # ── Rule 3: constructor / method param list ────────────────────
        is_fn = bool(re.match(r'^\s+\w[\w:<>*& ]+\(', line)) and '(' in line
        if is_fn:
            wrapped = _wrap_paren_params(line, col)
            if wrapped != line:
                for sub in wrapped.splitlines():
                    out.append(sub)
                continue

        # ── Rule 4: generic angle-bracket wrapping ─────────────────────
        if '<' in line:
            wrapped = _wrap_angle_args(line, base_indent, col)
            if wrapped != line:
                for sub in wrapped.splitlines():
                    out.append(sub)
                continue

        out.append(line)
    return '\n'.join(out)


# ----------------------------------------------------------------------
# CLI
# ----------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(
        description='Generate Gaudi Functional C++ algorithm boilerplate.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='Functional types:\n' + '\n'.join(
            f'  {k}: {v["desc"]}' for k, v in FUNCTIONAL_TYPES.items()
        )
    )

    parser.add_argument('class_name', help='Name of the C++ class to generate')
    parser.add_argument(
        'functional_type',
        choices=list(FUNCTIONAL_TYPES.keys()),
        help='Functional algorithm type'
    )
    parser.add_argument(
        '-i', '--inputs',
        nargs='*', default=[],
        metavar='TYPE:KEY',
        help='Input collection specs, e.g. edm4hep::MCParticleCollection:mcParticles'
    )
    parser.add_argument(
        '-o', '--outputs',
        nargs='*', default=[],
        metavar='TYPE:KEY',
        help='Fixed output collection specs, e.g. edm4hep::MCParticleCollection:MCParticles'
    )
    parser.add_argument(
        '--runtime-outputs',
        dest='runtime_outputs',
        default=None,
        metavar='TYPE',
        help=(
            'Enable runtime (dynamic) output collections returning std::vector<TYPE>. '
            'e.g. --runtime-outputs edm4hep::MCParticleCollection. '
            'Mutually exclusive with --outputs.'
        )
    )
    parser.add_argument(
        '-p', '--properties',
        nargs='*', default=[],
        metavar='NAME:TYPE:DEFAULT[:DESC]',
        help='Properties, e.g. threshold:float:0.5 or ExampleInt:int:3:My description'
    )
    parser.add_argument(
        '--namespace', default='',
        help='Optional C++ namespace to wrap the class in'
    )
    parser.add_argument(
        '--framework',
        choices=['gaudi', 'k4fwcore'],
        default='k4fwcore',
        help='Target framework (default: k4fwcore)'
    )
    parser.add_argument(
        '--use-class',
        action='store_true', default=False,
        help='Use "class" keyword instead of "struct"'
    )
    parser.add_argument(
        '--output-file',
        dest='output_file', default=None,
        help='Override the output .cpp filename (default: <ClassName>.cpp)'
    )
    parser.add_argument(
        '--type-aliases',
        dest='type_aliases', action='store_true', default=False,
        help=(
            'Emit "using AliasColl = FullType;" aliases before the struct and use '
            'them in the template signature and operator() parameters. '
            'Alias names are derived automatically: '
            'edm4hep::MCParticleCollection -> ParticleColl, '
            'podio::UserDataCollection<float> -> FloatColl.'
        )
    )
    parser.add_argument(
        '--private-properties',
        dest='private_properties', action='store_true', default=False,
        help='Place Gaudi::Property members under a private: access label'
    )
    parser.add_argument(
        '--all-keyvalues',
        dest='all_keyvalues', action='store_true', default=False,
        help=(
            'Use KeyValues(name, {"default"}) for ALL inputs instead of KeyValue(name, "default"). '
            'Applies to any input not already covered by --runtime-inputs or --keyvalues-inputs.'
        )
    )
    parser.add_argument(
        '--keyvalues-inputs',
        dest='keyvalues_inputs', nargs='*', default=None,
        metavar='KEY[:Default]',
        help=(
            'Declare one or more inputs using KeyValues(..., {"Default"}) instead of '
            'KeyValue(..., "Default"). The parameter type in operator() remains const TYPE&. '
            'KEY must match an --inputs entry. Optionally override the default: KEY:MyDefault. '
            'Example: --keyvalues-inputs InputCollection:MCParticles'
        )
    )
    parser.add_argument(
        '--runtime-inputs',
        dest='runtime_inputs', nargs='*', default=None,
        metavar='TYPE:KEY:Default0[,Default1,...]',
        help=(
            'Mark one or more inputs as runtime (dynamic) collections received as '
            'std::vector<const TYPE*>&. The KEY must match a --inputs entry. '
            'Defaults are comma-separated, e.g. '
            'edm4hep::MCParticleCollection:InputSeveralCollections:MCParticles0,MCParticles1'
        )
    )
    parser.add_argument(
        '--event-context',
        dest='event_context', action='store_true', default=False,
        help='Pass EventContext as first argument to operator() (consumer only)'
    )
    parser.add_argument(
        '--cmake',
        action='store_true', default=False,
        help='Also generate a CMakeLists.txt alongside the .cpp file'
    )

    args = parser.parse_args()

    # Validation
    if args.runtime_outputs and args.outputs:
        parser.error('--runtime-outputs and --outputs are mutually exclusive.')
    if args.runtime_outputs and args.framework != 'k4fwcore':
        parser.error('--runtime-outputs is only supported with --framework k4fwcore.')

    gen  = GaudiGen(args)
    code = gen.generate_code()

    code = format_cpp(code)
    cpp_file = args.output_file if args.output_file else f'{args.class_name}.cpp'
    with open(cpp_file, 'w') as f:
        f.write(code)
    print(f'Written to {cpp_file}', file=sys.stderr)

    if args.cmake:
        cmake_file = 'CMakeLists.txt'
        with open(cmake_file, 'w') as f:
            f.write(gen.generate_cmake())
        print(f'Written to {cmake_file}', file=sys.stderr)


if __name__ == '__main__':
    main()#!/usr/bin/env python3
"""
Gaudi Functional C++ Class Generator

A user-friendly script to generate Gaudi Functional C++ classes with proper
structure and boilerplate code. Supports both Gaudi::Functional and k4FWCore variants.
"""

import argparse
import sys
import re
from typing import List, Tuple, Optional

# Functional type definitions
FUNCTIONAL_TYPES = {
    'consumer': {
        'base': 'Consumer',
        'description': 'One or more inputs, no output',
        'example': 'EventTimeMonitor, ProcStatusAbortMoni'
    },
    'producer': {
        'base': 'Producer',
        'description': 'No input, one or more outputs',
        'example': 'ExampleFunctionalProducerMultiple, file IO, constant data generation'
    },
    'transformer': {
        'base': 'Transformer',
        'description': 'One or more inputs, one or more outputs',
        'example': 'Data transformation algorithms'
    },
    'filter': {
        'base': 'FilterPredicate',
        'description': 'One or more inputs, boolean output',
        'example': 'Event selection, filtering based on criteria'
    }
}


def parse_arguments():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description='Generate Gaudi/k4FWCore Functional C++ classes',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=_get_functional_types_help()
    )
    
    parser.add_argument('class_name', help='Name of the C++ class to generate')
    parser.add_argument('functional_type', 
                       choices=list(FUNCTIONAL_TYPES.keys()),
                       help='Type of functional to generate')
    parser.add_argument('-i', '--inputs', nargs='*', default=[],
                       help='Input data specifications (format: "Type:Location" or just "Type")')
    parser.add_argument('-o', '--outputs', nargs='*', default=[],
                       help='Output data specifications (format: "Type:Location" or just "Type")')
    parser.add_argument('-n', '--namespace', default='',
                       help='Namespace for the class')
    parser.add_argument('-f', '--output-file', 
                       help='Output file name (default: <ClassName>.cpp)')
    parser.add_argument('--framework', choices=['gaudi', 'k4fwcore'], default='k4fwcore',
                       help='Target framework (default: k4fwcore)')
    parser.add_argument('--class', dest='use_class', action='store_true',
                       help='Generate as class instead of struct (struct is default for k4fwcore)')
    parser.add_argument('-p', '--properties', nargs='*', default=[],
                       help='Gaudi properties (format: "Type:Name:Default:Description")')
    
    return parser.parse_args()


def _get_functional_types_help():
    """Generate help text for functional types."""
    help_text = "\nAvailable Functional Types:\n"
    for key, info in FUNCTIONAL_TYPES.items():
        help_text += f"\n  {key}:\n"
        help_text += f"    {info['description']}\n"
        help_text += f"    Example: {info['example']}\n"
    
    help_text += "\n\nExample Usage:\n"
    help_text += "  # k4FWCore producer with multiple outputs (generates struct by default)\n"
    help_text += "  python gaudi_gen.py MyProducer producer \\\n"
    help_text += "    -o 'edm4hep::MCParticleCollection:MCParticles' \\\n"
    help_text += "       'edm4hep::TrackCollection:Tracks'\n\n"
    help_text += "  # Gaudi transformer (generates class by default)\n"
    help_text += "  python gaudi_gen.py MyTransformer transformer \\\n"
    help_text += "    -i 'InputType:InputLoc' \\\n"
    help_text += "    -o 'OutputType:OutputLoc' \\\n"
    help_text += "    --framework gaudi\n"
    
    return help_text


def parse_data_spec(spec: str) -> Tuple[str, str]:
    """Parse data specification string into type and location.
    Handles types with template parameters like podio::UserDataCollection<float>
    """
    # Find the last colon that's not inside angle brackets
    depth = 0
    colon_pos = -1
    for i, char in enumerate(spec):
        if char == '<':
            depth += 1
        elif char == '>':
            depth -= 1
        elif char == ':' and depth == 0:
            colon_pos = i
    
    if colon_pos == -1:
        return spec, ''
    
    return spec[:colon_pos], spec[colon_pos+1:]


def parse_property_spec(spec: str) -> Tuple[str, str, str, str]:
    """Parse property specification into type, name, default, description."""
    parts = spec.split(':', 3)
    typ = parts[0] if len(parts) > 0 else 'int'
    name = parts[1] if len(parts) > 1 else 'Property'
    default = parts[2] if len(parts) > 2 else '0'
    desc = parts[3] if len(parts) > 3 else 'Property description'
    return typ, name, default, desc


def generate_template_signature(functional_type: str, inputs: List[str], outputs: List[str], 
                                framework: str) -> str:
    """Generate the template signature for the functional."""
    in_types = [parse_data_spec(i)[0] for i in inputs]
    out_types = [parse_data_spec(o)[0] for o in outputs]
    
    if functional_type == 'consumer':
        in_sig = ', '.join([f"const {t}&" for t in in_types])
        return f"void({in_sig})"
    elif functional_type == 'producer':
        if len(out_types) == 0:
            return "void()"
        elif len(out_types) == 1:
            return f"{out_types[0]}()"
        else:
            return f"std::tuple<{', '.join(out_types)}>()"
    elif functional_type == 'transformer':
        in_sig = ', '.join([f"const {t}&" for t in in_types]) if in_types else ""
        if len(out_types) == 1:
            return f"{out_types[0]}({in_sig})"
        else:
            out_sig = ', '.join(out_types)
            return f"std::tuple<{out_sig}>({in_sig})"
    elif functional_type == 'filter':
        in_sig = ', '.join([f"const {t}&" for t in in_types])
        return f"bool({in_sig})"
    
    return ""


def generate_keyvalues_k4fwcore(data_specs: List[str], is_input: bool) -> str:
    """Generate KeyValues initialization for k4FWCore."""
    if not data_specs:
        return "{}"
    
    lines = []
    for spec in data_specs:
        typ, loc = parse_data_spec(spec)
        if not loc:
            # Generate a default location name from type
            # Remove Collection suffix and namespace
            clean_name = typ.split('::')[-1].replace('Collection', '')
            loc = clean_name
        lines.append(f'KeyValues("{loc}", {{"{loc}"}})')
    
    if len(lines) == 1:
        return lines[0]
    else:
        return "{\n                 " + ",\n                 ".join(lines) + "}"


def generate_keyvalue_gaudi(data_specs: List[str]) -> str:
    """Generate KeyValue initialization for Gaudi."""
    if not data_specs:
        return ""
    
    key_values = []
    for spec in data_specs:
        typ, loc = parse_data_spec(spec)
        if not loc:
            loc = f"{typ.split('::')[-1]}Loc"
        default_val = loc
        key_values.append(f'KeyValue("{loc}", "{default_val}")')
    
    if len(key_values) == 1:
        return key_values[0]
    else:
        return "{\n              " + ",\n              ".join(key_values) + " }"


def generate_constructor(class_name: str, functional_type: str, inputs: List[str], 
                        outputs: List[str], framework: str, base_class_short: str) -> str:
    """Generate the constructor."""
    if framework == 'k4fwcore':
        input_kv = generate_keyvalues_k4fwcore(inputs, True)
        output_kv = generate_keyvalues_k4fwcore(outputs, False)
        
        # For k4FWCore, use the short base class name (just "Producer", not "k4FWCore::Producer")
        # because we're already inheriting from the fully qualified name
        return f"""  {class_name}(const std::string& name, ISvcLocator* svcLoc)
      : {base_class_short}(name, svcLoc, {input_kv},
                 {output_kv}) {{}}"""
    else:  # gaudi
        input_kv = generate_keyvalue_gaudi(inputs)
        output_kv = generate_keyvalue_gaudi(outputs)
        
        init_parts = [f"\n           {base_class_short}(\n               name,\n               pSvc"]
        if input_kv:
            init_parts.append(f", {input_kv}")
        if output_kv:
            init_parts.append(f",\n               {output_kv}")
        init_parts.append(")")
        
        constructor_init = ''.join(init_parts)
        
        return f"""  {class_name}(const std::string& name, ISvcLocator* pSvc)
      :{constructor_init} {{}}"""


def generate_operator_signature(functional_type: str, inputs: List[str], outputs: List[str]) -> str:
    """Generate the operator() signature."""
    in_types = [parse_data_spec(i)[0] for i in inputs]
    out_types = [parse_data_spec(o)[0] for o in outputs]
    
    if functional_type == 'consumer':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        return f"void operator()({params}) const override"
    elif functional_type == 'producer':
        if len(out_types) == 0:
            return "void operator()() const override"
        elif len(out_types) == 1:
            return f"{out_types[0]} operator()() const override"
        else:
            return f"std::tuple<{', '.join(out_types)}> operator()() const override"
    elif functional_type == 'transformer':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        if len(out_types) == 1:
            return f"{out_types[0]} operator()({params}) const override"
        else:
            return f"std::tuple<{', '.join(out_types)}> operator()({params}) const override"
    elif functional_type == 'filter':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        return f"bool operator()({params}) const override"
    
    return ""


def generate_operator_body(functional_type: str, outputs: List[str]) -> str:
    """Generate a template body for the operator()."""
    out_types = [parse_data_spec(o)[0] for o in outputs]
    
    if functional_type == 'consumer':
        return "    // Process input data here\n"
    elif functional_type == 'producer':
        if len(out_types) == 0:
            return "    // Perform operations here\n"
        elif len(out_types) == 1:
            return f"    // Generate and return output data\n    auto output = {out_types[0]}();\n    \n    return output;\n"
        else:
            lines = []
            for i, typ in enumerate(out_types):
                lines.append(f"    auto output{i+1} = {typ}();")
            lines.append("\n    // TODO: Fill output collections\n")
            lines.append(f"    return std::make_tuple({', '.join([f'std::move(output{i+1})' for i in range(len(out_types))])});")
            return '\n'.join(lines) + '\n'
    elif functional_type == 'filter':
        return "    // Apply filter logic and return true/false\n    return true;\n"
    elif functional_type == 'transformer':
        if len(out_types) == 1:
            return f"    // Transform input(s) to output\n    auto output = {out_types[0]}();\n    \n    return output;\n"
        else:
            lines = []
            for i, typ in enumerate(out_types):
                lines.append(f"    auto output{i+1} = {typ}();")
            lines.append("\n    // TODO: Fill output collections\n")
            lines.append(f"    return std::make_tuple({', '.join([f'std::move(output{i+1})' for i in range(len(out_types))])});")
            return '\n'.join(lines) + '\n'
    
    return ""


def extract_edm_includes(typ: str) -> List[str]:
    """Extract EDM4hep include files from a type string."""
    includes = []
    
    # Handle podio types
    if 'podio::UserDataCollection' in typ:
        includes.append('#include "podio/UserDataCollection.h"')
        return includes
    
    # Handle edm4hep types
    if 'edm4hep::' in typ:
        # Extract all collection types (handle nested templates)
        # Match patterns like edm4hep::MCParticleCollection
        pattern = r'edm4hep::(\w+Collection)'
        matches = re.findall(pattern, typ)
        for match in matches:
            base_type = match.replace('Collection', '')
            includes.append(f'#include "edm4hep/{base_type}Collection.h"')
    
    return includes


def generate_includes(functional_type: str, inputs: List[str], outputs: List[str], 
                     framework: str, properties: List[str]) -> str:
    """Generate include statements."""
    includes = []
    
    if framework == 'k4fwcore':
        includes.append('#include "k4FWCore/Consumer.h"' if functional_type == 'consumer' else
                       '#include "k4FWCore/Producer.h"' if functional_type == 'producer' else
                       '#include "k4FWCore/Transformer.h"' if functional_type == 'transformer' else
                       '#include "k4FWCore/FilterPredicate.h"')
    else:
        includes.append('#include "GaudiAlg/Functional.h"')
        includes.append('#include "GaudiKernel/KeyValue.h"')
    
    if properties:
        includes.append('#include "Gaudi/Property.h"')
    
    includes.append('')
    
    # Collect unique type includes
    all_type_strings = []
    for inp in inputs:
        typ, _ = parse_data_spec(inp)
        all_type_strings.append(typ)
    for out in outputs:
        typ, _ = parse_data_spec(out)
        all_type_strings.append(typ)
    
    # Generate includes for EDM4hep types
    type_includes = []
    for typ in all_type_strings:
        type_includes.extend(extract_edm_includes(typ))
    
    if type_includes:
        includes.extend(sorted(set(type_includes)))
        includes.append('')
    
    includes.append('#include <string>')
    
    # Add tuple if multiple outputs
    out_types = [parse_data_spec(o)[0] for o in outputs]
    if len(out_types) > 1:
        includes.append('#include <tuple>')
    
    return '\n'.join(includes)


def generate_properties(properties: List[str]) -> str:
    """Generate Gaudi property declarations."""
    if not properties:
        return ""
    
    lines = ["\n\nprivate:"]
    for prop_spec in properties:
        typ, name, default, desc = parse_property_spec(prop_spec)
        lines.append(f'  Gaudi::Property<{typ}> m_{name}{{this, "{name}", {default}, "{desc}"}};')
    
    return '\n'.join(lines)


def generate_return_type_alias(outputs: List[str]) -> Optional[str]:
    """Generate return type alias if needed (for complex return types)."""
    out_types = [parse_data_spec(o)[0] for o in outputs]
    if len(out_types) > 1:
        types_str = ',\n               '.join(out_types)
        return f"using retType =\n    std::tuple<{types_str}>;\n\n"
    return None


def generate_class(class_name: str, functional_type: str, inputs: List[str], 
                   outputs: List[str], namespace: str, framework: str,
                   use_class: bool, properties: List[str], command_line: str) -> str:
    """Generate the complete C++ class code."""
    base_class_short = FUNCTIONAL_TYPES[functional_type]['base']
    
    if framework == 'k4fwcore':
        base_class_full = f"k4FWCore::{base_class_short}"
    else:
        base_class_full = f"Gaudi::Functional::{base_class_short}"
    
    template_sig = generate_template_signature(functional_type, inputs, outputs, framework)
    # Pass just the short name for constructor initialization
    constructor = generate_constructor(class_name, functional_type, inputs, outputs, framework, base_class_short)
    operator_sig = generate_operator_signature(functional_type, inputs, outputs)
    operator_body = generate_operator_body(functional_type, outputs)
    includes = generate_includes(functional_type, inputs, outputs, framework, properties)
    prop_declarations = generate_properties(properties)
    return_type_alias = generate_return_type_alias(outputs)
    
    # k4FWCore uses struct by default, Gaudi uses class by default
    if framework == 'k4fwcore':
        class_keyword = "class" if use_class else "struct"
        public_keyword = "public:\n" if use_class else ""
    else:
        class_keyword = "struct" if not use_class else "class"
        public_keyword = "" if not use_class else "public:\n"
    
    code = f"""// Generated by Gaudi Functional C++ Class Generator
// Command: {command_line}
{includes}

"""
    
    if namespace:
        code += f"namespace {namespace} {{\n\n"
    
    # Use retType alias if it exists
    if return_type_alias:
        code += return_type_alias
        template_for_class = "retType()"
    else:
        template_for_class = template_sig
    
    code += f"""{class_keyword} {class_name} final : {base_class_full}<{template_for_class}> {{

{public_keyword}{constructor}

  // This is the function that will be called to produce the data
  {operator_sig} {{
{operator_body}  }}{prop_declarations}
}};

"""
    
    if namespace:
        code += f"}} // namespace {namespace}\n\n"
    
    code += f"DECLARE_COMPONENT({class_name})\n"
    
    return code


def main():
    """Main entry point."""
    args = parse_arguments()
    
    # Validate inputs/outputs based on functional type
    if args.functional_type == 'consumer' and not args.inputs:
        print("Error: Consumer requires at least one input", file=sys.stderr)
        return 1
    elif args.functional_type == 'producer' and not args.outputs:
        print("Error: Producer requires at least one output", file=sys.stderr)
        return 1
    elif args.functional_type in ['transformer', 'filter'] and not args.inputs:
        print(f"Error: {args.functional_type} requires at least one input", file=sys.stderr)
        return 1
    elif args.functional_type == 'transformer' and not args.outputs:
        print(f"Error: {args.functional_type} requires at least one output", file=sys.stderr)
        return 1
    
    # Reconstruct the command line for documentation
    import shlex
    command_line = ' '.join(shlex.quote(arg) for arg in sys.argv)
    
    # Generate the class
    code = generate_class(
        args.class_name,
        args.functional_type,
        args.inputs,
        args.outputs,
        args.namespace,
        args.framework,
        args.use_class,
        args.properties,
        command_line
    )
    
    # Determine output file
    output_file = args.output_file
    if not output_file:
        output_file = f"{args.class_name}.cpp"
    
    # Write to file or stdout
    if output_file == '-':
        print(code)
    else:
        with open(output_file, 'w') as f:
            f.write(code)
        print(f"Generated {output_file}")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
