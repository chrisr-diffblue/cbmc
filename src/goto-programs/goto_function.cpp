/*******************************************************************\

Module: A GOTO Function

Author: Daniel Kroening

Date: May 2018

\*******************************************************************/

/// \file
/// Goto Function

#include "goto_function.h"

/// Return in \p dest the identifiers of the local variables declared in the \p
/// goto_function and the identifiers of the paramters of the \p goto_function.
void get_local_identifiers(
  const goto_functiont &goto_function,
  std::set<irep_idt> &dest)
{
  goto_function.body.get_decl_identifiers(dest);

  const code_typet::parameterst &parameters = goto_function.type.parameters();

  // add parameters
  for(const auto &param : parameters)
  {
    const irep_idt &identifier = param.get_identifier();
    if(identifier != "")
      dest.insert(identifier);
  }
}

/// Check that the goto function is well-formed
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void goto_functiont::validate(const namespacet &ns, const validation_modet vm) const
{
  body.validate(ns, vm);

  find_symbols_sett typetags;
  find_type_symbols(type, typetags);
  const symbolt *symbol;
  for(const auto &identifier : typetags)
  {
    DATA_CHECK(
      vm,
      !ns.lookup(identifier, symbol),
      id2string(identifier) + " not found");
  }

  // Check that a void function does not contain any RETURN instructions
  if(to_code_type(type).return_type().id() == ID_empty)
  {
    forall_goto_program_instructions(instruction, body)
    {
      DATA_CHECK(
        vm,
        !instruction->is_return(),
        "void function should not return a value");
    }
  }

  validate_full_type(type, ns, vm);
}
