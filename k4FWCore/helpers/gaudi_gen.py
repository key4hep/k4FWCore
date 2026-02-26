#!/usr/bin/env python3
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
