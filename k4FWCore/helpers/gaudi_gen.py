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
        p0, p1 = sep_positions[0], sep_positions[1]
        typ      = spec[:p0]
        key      = spec[p0+1:p1]
        defaults = [d.strip() for d in spec[p1+1:].split(',')]
        return typ, key, defaults

    def _default_key(self, typ: str) -> str:
        base = typ.split('::')[-1]
        base = re.sub(r'Collection$', '', base)
        return base + 's'

    def _is_k4(self) -> bool:
        return self.args.framework == 'k4fwcore'

    def _base_class_alias(self) -> str:
        return "using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;"

    def _use_ret_type_alias(self) -> bool:
        return self._is_k4() and not self.args.runtime_outputs and len(self.args.outputs) > 1

    def _is_runtime(self) -> bool:
        return bool(self.args.runtime_outputs)

    def _collect_type_aliases(self) -> List[Tuple[str, str]]:
        seen_types: dict = {}
        aliases: List[Tuple[str, str]] = []
        for spec in self.args.inputs:
            typ, _ = self.parse_data_spec(spec)
            if typ in seen_types:
                continue
            inner = re.search(r'<([^>]+)>', typ)
            if inner:
                base = inner.group(1).strip().split('::')[-1].capitalize()
            else:
                last = typ.split('::')[-1]
                stem = re.sub(r'Collection$', '', last)
                if stem.endswith('Link'):
                    base = 'Link'
                elif re.search(r'Hit', stem):
                    base = re.sub(r'\d+[A-Z]?$', '', stem) or stem
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
        all_specs = list(self.args.inputs) + list(self.args.outputs)
        if self._is_runtime():
            all_specs.append(self.args.runtime_outputs)
        for spec in all_specs:
            typ, _ = self.parse_data_spec(spec)
            if 'podio::UserDataCollection' in typ:
                type_inc.add('#include "podio/UserDataCollection.h"')
            if 'edm4hep::' in typ:
                for match in re.findall(r'edm4hep::(\w+Collection)', typ):
                    header = re.sub(r'Collection$', '', match)
                    type_inc.add(f'#include "edm4hep/{header}.h"')
        inc.extend(sorted(type_inc))
        inc.append('#include <string>')
        # sstream/stdexcept only if consumer body uses them (not added by default)
        if getattr(self.args, 'event_context', False):
            inc.extend(['#include <limits>', '#include <mutex>', '#include <set>'])
        if len(self.args.outputs) > 1:
            inc.append('#include <tuple>')
        if self._is_runtime() or getattr(self.args, 'runtime_inputs', None):
            inc.append('#include <vector>')
        return '\n'.join(inc)

    def _ret_type_alias(self) -> str:
        out_types = [self.parse_data_spec(o)[0] for o in self.args.outputs]
        prefix = '    std::tuple<'
        indent = ' ' * len(prefix)
        joined = (',\n' + indent).join(out_types)
        return f'using retType =\n{prefix}{joined}>;'

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

    def _build_constructor_k4fwcore(self) -> str:
        cls        = self.args.class_name
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
        if self._is_k4() and self.args.functional_type == 'transformer' and len(self.args.outputs) > 1:
            base_short = 'MultiTransformer'
        f_type     = self.args.functional_type
        ri_map = {}
        for s in (getattr(self.args, 'runtime_inputs', []) or []):
            _, key, defaults = self.parse_runtime_input_spec(s)
            ri_map[key] = defaults
        ri_keys = set(ri_map.keys())
        kvi_map = {}
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
            if len(kv_list) == 1:
                in_block = kv_list[0]
            else:
                indent = ' ' * 20
                in_block = '{\n' + indent + (',\n' + indent).join(kv_list) + ',\n' + ' ' * 16 + '}'
        if f_type in ('consumer', 'filter'):
            ctor_args = in_block
        elif self._is_runtime():
            typ, key = self.parse_data_spec(self.args.runtime_outputs)
            key       = key if key else self._default_key(typ)
            out_block = f'{{KeyValues("OutputCollections", {{"{key}"}})}}'
            ctor_args = f'{in_block}, {out_block}'
        elif not self.args.outputs:
            ctor_args = f'{in_block}, {{}}'
        elif len(self.args.outputs) > 1:
            kv_items  = []
            for spec in self.args.outputs:
                typ, key = self.parse_data_spec(spec)
                key = key if key else self._default_key(typ)
                kv_items.append(f'KeyValues("{key}", {{"{key}"}})')
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
            typ, key  = self.parse_data_spec(self.args.outputs[0])
            key        = key if key else self._default_key(typ)
            out_block  = f'KeyValue("{key}", "{key}")'
            ctor_args  = f'{in_block}, {out_block}'
        return (
            f'  {cls}(const std::string& name, ISvcLocator* svcLoc)\n'
            f'      : {base_short}(name, svcLoc, {ctor_args}) {{}}'
        )

    def _build_constructor_gaudi(self) -> str:
        cls        = self.args.class_name
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
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
        lines = []
        for i, t in enumerate(out_types, 1):
            lines.append(f'    auto output{i} = {t}();')
        lines.append('')
        lines.append('    // TODO: Fill output collections')
        lines.append('')
        moves = ', '.join(f'std::move(output{i})' for i in range(1, len(out_types)+1))
        lines.append(f'    return std::make_tuple({moves});  // NOLINT')
        return '\n'.join(lines)

    def _initialize_hint(self) -> List[str]:
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
            lines += [f'    // inputLocations({i}) or inputLocations("{key}") -> names of {key}']
        lines += ["    return StatusCode::SUCCESS;", "  }"]
        return lines

    def _consumer_body_hint(self) -> str:
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

    def _build_properties(self) -> str:
        lines = []
        for prop in self.args.properties:
            parts    = prop.split(':', 3)
            ptype    = parts[0]
            pname    = parts[1] if len(parts) > 1 else parts[0]
            pdefault = parts[2] if len(parts) > 2 else '0'
            pdesc    = parts[3] if len(parts) > 3 else f'Example {pname} property'
            member = pname if pname.startswith('m_') else f'm_{pname}'
            lines.append(
                f'  Gaudi::Property<{ptype}> {member}'
                f'{{this, "{pname}", {pdefault}, "{pdesc}"}};'
            )
        return '\n'.join(lines)

    def generate_code(self) -> str:
        base_short = FUNCTIONAL_TYPES[self.args.functional_type]['base']
        # k4FWCore uses MultiTransformer when there are multiple outputs
        if self._is_k4() and self.args.functional_type == 'transformer' and len(self.args.outputs) > 1:
            base_short = 'MultiTransformer'
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
        if self.args.properties:
            if getattr(self.args, 'private_properties', False):
                lines += ["", "private:", self._build_properties()]
            else:
                lines += ["", self._build_properties()]
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
            f"# Command: {self.command_line}\n\n"
            f"cmake_minimum_required(VERSION 3.15)\n"
            f"project({cls}Plugin)\n\n"
            f"{find_block}\n\n"
            f"gaudi_add_module({cls}Plugin\n"
            f"  SOURCES {src}\n"
            f"  LINK\n"
            f"    {link_block}\n"
            f")\n"
        )


def main():
    parser = argparse.ArgumentParser(
        description='Generate Gaudi Functional C++ algorithm boilerplate.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='Functional types:\n' + '\n'.join(
            f'  {k}: {v["desc"]}' for k, v in FUNCTIONAL_TYPES.items()
        )
    )
    parser.add_argument('class_name')
    parser.add_argument('functional_type', choices=list(FUNCTIONAL_TYPES.keys()))
    parser.add_argument('-i', '--inputs', nargs='*', default=[])
    parser.add_argument('-o', '--outputs', nargs='*', default=[])
    parser.add_argument('--runtime-outputs', dest='runtime_outputs', default=None)
    parser.add_argument('-p', '--properties', nargs='*', default=[])
    parser.add_argument('-n', '--namespace', default='')
    parser.add_argument('--framework', choices=['gaudi', 'k4fwcore'], default='k4fwcore')
    parser.add_argument('--use-class', action='store_true', default=False)
    parser.add_argument('--output-file', dest='output_file', default=None)
    parser.add_argument('--type-aliases', dest='type_aliases', action='store_true', default=False)
    parser.add_argument('--private-properties', dest='private_properties', action='store_true', default=False)
    parser.add_argument('--all-keyvalues', dest='all_keyvalues', action='store_true', default=False)
    parser.add_argument('--keyvalues-inputs', dest='keyvalues_inputs', nargs='*', default=None)
    parser.add_argument('--runtime-inputs', dest='runtime_inputs', nargs='*', default=None)
    parser.add_argument('--event-context', dest='event_context', action='store_true', default=False)
    parser.add_argument('--cmake', action='store_true', default=False)
    args = parser.parse_args()
    if args.runtime_outputs and args.outputs:
        parser.error('--runtime-outputs and --outputs are mutually exclusive.')
    if args.runtime_outputs and args.framework != 'k4fwcore':
        parser.error('--runtime-outputs is only supported with --framework k4fwcore.')
    gen  = GaudiGen(args)
    code = gen.generate_code()
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
    main()
