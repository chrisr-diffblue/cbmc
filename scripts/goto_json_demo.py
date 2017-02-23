#!/usr/bin/env python

import goto_json
import sys

if len(sys.argv) != 2:
  print >>sys.stderr, "Usage: goto_json_demo.py irep_spec_directory"
  sys.exit(1)

gj = goto_json.goto_json(sys.argv[1])

int_type = gj.make_irep("signedbv_type", width = 32)

def bin32(i):
  return "{0:0>32b}".format(i)

symtab = goto_json.symbol_table()
global_symbol = goto_json.symbol(
  name = "g",
  base_name = "g",
  mode = "C",
  pretty_name = "g",
  is_static_lifetime = True,
  type = int_type,
  value = gj.make_irep("constant_expr", value = bin32(0), type = int_type))
symtab.add(global_symbol)
global_symexpr = global_symbol.symbol_expr(gj)

parameter_symbol = goto_json.symbol(
  name = "arg1",
  base_name = "arg1",
  mode = "C",
  pretty_name = "arg1",
  is_parameter = True,
  type = int_type)
symtab.add(parameter_symbol)
parameter_symexpr = parameter_symbol.symbol_expr(gj)

ftype = gj.make_irep("code_type")
ftype["parameters"] = \
  [gj.make_irep("code_parameter", identifier = "arg1", base_name = "arg1", type = int_type)]
ftype["return_type"] = int_type

fbody = gj.make_irep("code_block")

sum_expr = gj.make_irep("+", lhs = global_symexpr, rhs = parameter_symexpr, type = int_type)

finstructions = [
  gj.make_irep(
    "code_assign",
    lhs = global_symexpr,
    rhs = gj.make_irep("constant_expr", value = bin32(1), type = global_symbol["type"])),
  gj.make_irep(
    "code_assign",
    lhs = global_symexpr,
    rhs = sum_expr),
  gj.make_irep("code_return", return_value = parameter_symexpr)
]

fbody["ops"] = finstructions

function_symbol = goto_json.symbol(
  name = "test_function",
  base_name = "test_function",
  mode = "C",
  pretty_name = "test_function",
  type = ftype,
  value = fbody)
symtab.add(function_symbol)

sys.stdout.write(symtab.to_json())
