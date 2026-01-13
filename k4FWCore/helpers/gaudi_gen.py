#!/usr/bin/env python3
"""
Gaudi Functional C++ Class Generator

A user-friendly script to generate Gaudi Functional C++ classes with proper
structure and boilerplate code.
"""

import argparse
import sys
from typing import List, Tuple

# Functional type definitions
FUNCTIONAL_TYPES = {
    'consumer': {
        'base': 'Consumer',
        'description': 'One input, no output',
        'example': 'EventTimeMonitor, ProcStatusAbortMoni'
    },
    'producer': {
        'base': 'Producer',
        'description': 'No input, one or more outputs',
        'example': 'File IO, constant data generation'
    },
    'filter': {
        'base': 'FilterPredicate',
        'description': 'True/False output only',
        'example': 'HDRFilter, L0Filter, ODINFilter'
    },
    'transformer': {
        'base': 'Transformer',
        'description': 'One or more inputs, one output',
        'example': 'MySum, data transformation'
    },
    'multi_transformer': {
        'base': 'MultiTransformer',
        'description': 'One or more inputs, multiple outputs',
        'example': 'Complex data processing with multiple results'
    },
    'merging_transformer': {
        'base': 'MergingTransformer',
        'description': 'Identical inputs, one output',
        'example': 'TrackListMerger, InCaloAcceptanceAlg'
    },
    'splitting_transformer': {
        'base': 'SplittingTransformer',
        'description': 'One input, identical outputs',
        'example': 'HltRawBankDecoderBase'
    },
    'scalar_transformer': {
        'base': 'ScalarTransformer',
        'description': 'Vector to vector with 1-to-1 element mapping',
        'example': 'CaloElectronAlg, CaloSinglePhotonAlg'
    }
}


def parse_arguments():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description='Generate Gaudi Functional C++ classes',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=_get_functional_types_help()
    )
    
    parser.add_argument('class_name', help='Name of the C++ class to generate')
    parser.add_argument('functional_type', 
                       choices=list(FUNCTIONAL_TYPES.keys()),
                       help='Type of functional to generate')
    parser.add_argument('-i', '--inputs', nargs='*', default=[],
                       help='Input data specifications (format: "Type:Location:DefaultValue")')
    parser.add_argument('-o', '--outputs', nargs='*', default=[],
                       help='Output data specifications (format: "Type:Location:DefaultValue")')
    parser.add_argument('-n', '--namespace', default='',
                       help='Namespace for the class')
    parser.add_argument('-f', '--output-file', 
                       help='Output file name (default: <ClassName>.cpp)')
    parser.add_argument('--header-only', action='store_true',
                       help='Generate header file instead of implementation')
    
    return parser.parse_args()


def _get_functional_types_help():
    """Generate help text for functional types."""
    help_text = "\nAvailable Functional Types:\n"
    for key, info in FUNCTIONAL_TYPES.items():
        help_text += f"\n  {key}:\n"
        help_text += f"    {info['description']}\n"
        help_text += f"    Example: {info['example']}\n"
    return help_text


def parse_data_spec(spec: str) -> Tuple[str, str, str]:
    """Parse data specification string into type, location, and default value."""
    parts = spec.split(':')
    if len(parts) == 1:
        return parts[0], '', ''
    elif len(parts) == 2:
        return parts[0], parts[1], ''
    else:
        return parts[0], parts[1], parts[2]


def generate_template_signature(functional_type: str, inputs: List[str], outputs: List[str]) -> str:
    """Generate the template signature for the functional."""
    in_types = [parse_data_spec(i)[0] for i in inputs]
    out_types = [parse_data_spec(o)[0] for o in outputs]
    
    if functional_type == 'consumer':
        return f"void(const {in_types[0]}&)" if in_types else "void()"
    elif functional_type == 'producer':
        if len(out_types) == 1:
            return f"{out_types[0]}()"
        else:
            return f"std::tuple<{', '.join(out_types)}>()"
    elif functional_type == 'filter':
        in_sig = ', '.join([f"const {t}&" for t in in_types])
        return f"bool({in_sig})"
    elif functional_type == 'transformer':
        in_sig = ', '.join([f"const {t}&" for t in in_types])
        return f"{out_types[0]}({in_sig})"
    elif functional_type == 'multi_transformer':
        in_sig = ', '.join([f"const {t}&" for t in in_types])
        out_sig = ', '.join(out_types)
        return f"std::tuple<{out_sig}>({in_sig})"
    elif functional_type == 'merging_transformer':
        return f"{out_types[0]}(const std::vector<{in_types[0]}*>&)"
    elif functional_type == 'splitting_transformer':
        return f"std::vector<{out_types[0]}>(const {in_types[0]}&)"
    elif functional_type == 'scalar_transformer':
        return f"{out_types[0]}(const {in_types[0]}&)"
    
    return ""


def generate_constructor_inputs(inputs: List[str]) -> str:
    """Generate input KeyValue list for constructor."""
    if not inputs:
        return ""
    
    key_values = []
    for inp in inputs:
        typ, loc, default = parse_data_spec(inp)
        loc_name = loc if loc else f"{typ}Loc"
        default_val = default if default else f"Input/{typ}"
        key_values.append(f'KeyValue("{loc_name}", "{default_val}")')
    
    if len(key_values) == 1:
        return key_values[0]
    else:
        return "{\n              " + ",\n              ".join(key_values) + " }"


def generate_constructor_outputs(outputs: List[str]) -> str:
    """Generate output KeyValue list for constructor."""
    if not outputs:
        return ""
    
    key_values = []
    for out in outputs:
        typ, loc, default = parse_data_spec(out)
        loc_name = loc if loc else f"{typ}Loc"
        default_val = default if default else f"Output/{typ}"
        key_values.append(f'KeyValue("{loc_name}", "{default_val}")')
    
    if len(key_values) == 1:
        return key_values[0]
    else:
        return "{\n              " + ",\n              ".join(key_values) + " }"


def generate_operator_signature(functional_type: str, inputs: List[str], outputs: List[str]) -> str:
    """Generate the operator() signature."""
    in_types = [parse_data_spec(i)[0] for i in inputs]
    out_types = [parse_data_spec(o)[0] for o in outputs]
    
    if functional_type == 'consumer':
        in_sig = f"const {in_types[0]}& input" if in_types else ""
        return f"void operator()({in_sig}) const override"
    elif functional_type == 'producer':
        if len(out_types) == 1:
            return f"{out_types[0]} operator()() const override"
        else:
            return f"std::tuple<{', '.join(out_types)}> operator()() const override"
    elif functional_type == 'filter':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        return f"bool operator()({params}) const override"
    elif functional_type == 'transformer':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        return f"{out_types[0]} operator()({params}) const override"
    elif functional_type == 'multi_transformer':
        params = ', '.join([f"const {t}& in{i+1}" for i, t in enumerate(in_types)])
        return f"std::tuple<{', '.join(out_types)}> operator()({params}) const override"
    elif functional_type == 'merging_transformer':
        return f"{out_types[0]} operator()(const std::vector<{in_types[0]}*>& inputs) const override"
    elif functional_type == 'splitting_transformer':
        return f"std::vector<{out_types[0]}> operator()(const {in_types[0]}& input) const override"
    elif functional_type == 'scalar_transformer':
        return f"{out_types[0]} operator()(const {in_types[0]}& input) const override"
    
    return ""


def generate_operator_body(functional_type: str, inputs: List[str], outputs: List[str]) -> str:
    """Generate a template body for the operator()."""
    if functional_type == 'consumer':
        return "        // Process input data here\n"
    elif functional_type == 'producer':
        out_types = [parse_data_spec(o)[0] for o in outputs]
        if len(out_types) == 1:
            return f"        // Generate and return output data\n        return {out_types[0]}{{}};\n"
        else:
            return f"        // Generate and return output data\n        return {{{', '.join([f'{t}{{}}' for t in out_types])}}};\n"
    elif functional_type == 'filter':
        return "        // Apply filter logic and return true/false\n        return true;\n"
    elif functional_type in ['transformer', 'scalar_transformer']:
        out_type = parse_data_spec(outputs[0])[0]
        return f"        // Transform input(s) to output\n        return {out_type}{{}};\n"
    elif functional_type == 'multi_transformer':
        out_types = [parse_data_spec(o)[0] for o in outputs]
        return f"        // Transform inputs to multiple outputs\n        return {{{', '.join([f'{t}{{}}' for t in out_types])}}};\n"
    elif functional_type == 'merging_transformer':
        out_type = parse_data_spec(outputs[0])[0]
        return f"        // Merge inputs into single output\n        return {out_type}{{}};\n"
    elif functional_type == 'splitting_transformer':
        out_type = parse_data_spec(outputs[0])[0]
        return f"        // Split input into multiple outputs\n        return std::vector<{out_type}>{{}};\n"
    
    return ""


def generate_class(class_name: str, functional_type: str, inputs: List[str], 
                   outputs: List[str], namespace: str = '') -> str:
    """Generate the complete C++ class code."""
    base_class = FUNCTIONAL_TYPES[functional_type]['base']
    template_sig = generate_template_signature(functional_type, inputs, outputs)
    input_keyvalues = generate_constructor_inputs(inputs)
    output_keyvalues = generate_constructor_outputs(outputs)
    operator_sig = generate_operator_signature(functional_type, inputs, outputs)
    operator_body = generate_operator_body(functional_type, inputs, outputs)
    
    # Build constructor initializer list
    init_parts = [f"\n           {base_class}(\n               name,\n               pSvc"]
    if input_keyvalues:
        init_parts.append(f", {input_keyvalues}")
    if output_keyvalues:
        init_parts.append(f",\n               {output_keyvalues}")
    init_parts.append(")")
    
    constructor_init = ''.join(init_parts)
    
    code = f"""// Generated by Gaudi Functional C++ Class Generator
#include "GaudiAlg/Functional.h"
#include "GaudiKernel/KeyValue.h"

"""
    
    if namespace:
        code += f"namespace {namespace} {{\n\n"
    
    code += f"""class {class_name}
  : public Gaudi::Functional::{base_class}<{template_sig}> {{

public:
    {class_name}(const std::string& name, ISvcLocator* pSvc)
      :{constructor_init} {{}}

    {operator_sig} {{
{operator_body}    }}
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
    elif args.functional_type in ['transformer', 'filter'] and (not args.inputs or not args.outputs):
        print(f"Error: {args.functional_type} requires both inputs and outputs", file=sys.stderr)
        return 1
    
    # Generate the class
    code = generate_class(
        args.class_name,
        args.functional_type,
        args.inputs,
        args.outputs,
        args.namespace
    )
    
    # Determine output file
    output_file = args.output_file
    if not output_file:
        ext = '.h' if args.header_only else '.cpp'
        output_file = f"{args.class_name}{ext}"
    
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
