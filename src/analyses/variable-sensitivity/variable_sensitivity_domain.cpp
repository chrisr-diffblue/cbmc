/*******************************************************************\

Module: Abstract Interpretation

Author: Martin Brain

Date: April 2016

\*******************************************************************/

#include <ostream>

#include <util/message.h>
#include <util/simplify_expr.h>
#include <analyses/variable-sensitivity/pointer_abstract_object.h>

#include "variable_sensitivity_domain.h"
#include <util/cprover_prefix.h>

#ifdef DEBUG
#include <iostream>
#endif


/*******************************************************************\

Function: variable_sensitivity_domaint::transform

  Inputs: The instruction before (from) and after (to) the abstract domain,
          the abstract interpreter (ai) and the namespace (ns).

 Outputs: None

 Purpose: Compute the abstract transformer for a single instruction

\*******************************************************************/

void variable_sensitivity_domaint::transform(
    locationt from,
    locationt to,
    ai_baset &ai,
    const namespacet &ns)
{
  #ifdef DEBUG
  std::cout << "Transform from/to:\n";
  std::cout << from->location_number << " --> "
            << to->location_number << std::endl;
  #endif

  const goto_programt::instructiont &instruction=*from;
  switch(instruction.type)
  {
  case DECL:
    // Creates a new variable, which should be top
    // but we don't store top so ... no action required
    break;

  case DEAD:
    {
    // Assign to top is the same as removing
    abstract_object_pointert top_object=
      abstract_state.abstract_object_factory(
        to_code_dead(instruction.code).symbol().type(), ns, true);
    abstract_state.assign(
      to_code_dead(instruction.code).symbol(), top_object, ns);
    }
    break;

  case ASSIGN:
    {
      const code_assignt &inst = to_code_assign(instruction.code);

      // TODO : check return values
      abstract_object_pointert r = abstract_state.eval(inst.rhs(), ns);
      abstract_state.assign(inst.lhs(), r, ns);
    }
    break;

  case GOTO:
    {
      if(1) // (flow_sensitivity == FLOW_SENSITIVE)
      {
        // Get the next line
        locationt next=from;
        next++;
        // Is this a GOTO to the next line (i.e. pointless)
        if(next!=from->get_target())
        {
          if(to==from->get_target())
          {
            // The AI is exploring the branch where the jump is taken
            abstract_state.assume(instruction.guard, ns);
          }
          else
          {
            // Exploring the path where the jump is not taken - therefore assume
            // the condition is false
            abstract_state.assume(not_exprt(instruction.guard), ns);
          }
        }
        // ignore jumps to the next line, we can assume nothing
      }
    }
    break;

  case ASSUME:
    abstract_state.assume(instruction.guard, ns);
    break;

  case FUNCTION_CALL:
  {
    transform_function_call(from, to, ai, ns);
    break;
  }

  case END_FUNCTION:
  {
    // erase parameters

    const irep_idt id=from->function;
    const symbolt &symbol=ns.lookup(id);

    const code_typet &type=to_code_type(symbol.type);

    for(const auto &param : type.parameters())
    {
      // Bottom the arguments to the function
      abstract_state.assign(
        symbol_exprt(param.get_identifier(), param.type()),
        abstract_state.abstract_object_factory(param.type(), ns, true, false),
        ns);
    }
    break;
  }

    /***************************************************************/

  case ASSERT:
    // Conditions on the program, do not alter the data or information
    // flow and thus can be ignored.
    // Checking of assertions can only be reasonably done once the fix-point
    // has been computed, i.e. after all of the calls to transform.
    break;

  case SKIP:
  case LOCATION:
    // Can ignore
    break;

  case RETURN:
    throw "return instructions should be removed first";

  case START_THREAD:
  case END_THREAD:
  case ATOMIC_BEGIN:
  case ATOMIC_END:
    throw "threading not supported";

  case THROW:
  case CATCH:
    throw "exceptions not handled";

  case OTHER:
//    throw "other";
      break;

  default:
    throw "unrecognised instruction type";
  }

  assert(abstract_state.verify());
}

/*******************************************************************\

Function: variable_sensitivity_domaint::output

  Inputs: The output stream (out), the abstract interpreter (ai) and
          the namespace.

 Outputs: None

 Purpose: Basic text output of the abstract domain

\*******************************************************************/
void variable_sensitivity_domaint::output(
  std::ostream &out,
  const ai_baset &ai,
  const namespacet &ns) const
{
  abstract_state.output(out, ai, ns);
}

/*******************************************************************\

Function: variable_sensitivity_domaint::make_bottom

  Inputs: None

 Outputs: None

 Purpose: Sets the domain to bottom (no relations).

\*******************************************************************/
void variable_sensitivity_domaint::make_bottom()
{
  abstract_state.make_bottom();
  return;
}

/*******************************************************************\

Function: variable_sensitivity_domaint::make_top

  Inputs: None

 Outputs: None

 Purpose: Sets the domain to top (all relations).

\*******************************************************************/
void variable_sensitivity_domaint::make_top()
{
  abstract_state.make_top();
}


/*******************************************************************\

Function: variable_sensitivity_domaint::make_entry

  Inputs: None

 Outputs: None

 Purpose: Set up a sane entry state.

\*******************************************************************/
void variable_sensitivity_domaint::make_entry()
{
  abstract_state.make_top();
}

/*******************************************************************\

Function: variable_sensitivity_domaint::merge

  Inputs: The other domain (b) and it's preceding location (from) and
          current location (to).

 Outputs: True if something has changed.

 Purpose: Computes the join between "this" and "b".

\*******************************************************************/

bool variable_sensitivity_domaint::merge(
  const variable_sensitivity_domaint &b,
  locationt from,
  locationt to)
{
  #ifdef DEBUG
  std::cout << "Merging from/to:\n "
            << from->location_number << " --> "
            << to->location_number << std::endl;
  #endif

  // Use the abstract_environment merge
  bool any_changes=abstract_state.merge(b.abstract_state);

  assert(abstract_state.verify());
  return any_changes;
}

/*******************************************************************\

Function: variable_sensitivity_domaint::ai_simplify

  Inputs:
   condition - the expression to simplify
   ns - the namespace
   lhs - is the expression on the left hand side

 Outputs: True if no simplification was made

 Purpose: Use the information in the domain to simplify the expression
          with respect to the current location.  This may be able to
          reduce some values to constants.

\*******************************************************************/

bool variable_sensitivity_domaint::ai_simplify(
  exprt &condition, const namespacet &ns) const
{
  sharing_ptrt<abstract_objectt> res=abstract_state.eval(condition, ns);
  exprt c=res->to_constant();

  if(c.id()==ID_nil)
  {
    bool no_simplification=true;

    // Try to simplify recursively any child operations
    for(exprt &op : condition.operands())
    {
      no_simplification&=ai_simplify(op, ns);
    }

    return no_simplification;
  }
  else
  {
    bool condition_changed=(condition!=c);
    condition=c;
    return !condition_changed;
  }
}

/*******************************************************************\

Function: variable_sensitivity_domaint::is_bottom

  Inputs:

 Outputs: True if the domain is bottom (i.e. unreachable).

 Purpose: Find out if the domain is currently unreachable.

\*******************************************************************/

bool variable_sensitivity_domaint::is_bottom() const
{
  return abstract_state.is_bottom();
}

/*******************************************************************\

Function: variable_sensitivity_domaint::is_top

  Inputs:

 Outputs: True if the domain is top

 Purpose: Is the domain completely top at this state

\*******************************************************************/
bool variable_sensitivity_domaint::is_top() const
{
  return abstract_state.is_top();
}

/*******************************************************************\

Function: variable_sensitivity_domaint::ai_simplify_lhs

  Inputs:
   condition - the expression to simplify
   ns - the namespace

 Outputs: True if condition did not change. False otherwise. condition
          will be updated with the simplified condition if it has worked

 Purpose: Use the information in the domain to simplify the expression
          on the LHS of an assignment. This for example won't simplify symbols
          to their values, but does simplify indices in arrays, members of
          structs and dereferencing of pointers

\*******************************************************************/

bool variable_sensitivity_domaint::ai_simplify_lhs(
  exprt &condition, const namespacet &ns) const
{
  // Care must be taken here to give something that is still writable
  if(condition.id()==ID_index)
  {
    index_exprt ie = to_index_expr(condition);
    exprt index = ie.index();
    bool changed = ai_simplify(index, ns);
    if(changed)
    {
      ie.index() = index;
      condition = simplify_expr(ie, ns);
    }

    return !changed;
  }
  else if(condition.id()==ID_dereference)
  {
    dereference_exprt de = to_dereference_expr(condition);
    exprt pointer = de.pointer();
    bool changed = ai_simplify(pointer, ns);
    if(changed)
    {
      de.pointer() = pointer;
      condition = simplify_expr(de, ns);  // So *(&x) -> x
    }

    return !changed;
  }
  else if(condition.id()==ID_member)
  {
    member_exprt me = to_member_expr(condition);
    exprt compound = me.compound();
    // Carry on the RHS since we still require an addressable object for the
    // struct
    bool changed = ai_simplify_lhs(compound, ns);
    if(changed)
    {
      me.compound() = compound;
      condition = simplify_expr(me, ns);
    }

    return !changed;
  }
  else
    return true;
}

/*******************************************************************\

Function: variable_sensitivity_domaint::transform_function_call

  Inputs:
   from - the location to transform from which is a function call
   to - the destination of the transform (potentially inside the function call)
   ai - the abstract interpreter
   ns - the namespace of the current state

 Outputs:

 Purpose: Used by variable_sensitivity_domaint::transform to handle
          FUNCTION_CALL transforms. This is copying the values of the arguments
          into new symbols corresponding to the declared parameter names.

          If the function call is opaque we currently top the return value
          and the value of any things whose address is passed into the function.

\*******************************************************************/

void variable_sensitivity_domaint::transform_function_call(
  locationt from, locationt to, ai_baset &ai, const namespacet &ns)
{
  assert(from->type==FUNCTION_CALL);

  const code_function_callt &function_call=to_code_function_call(from->code);
  const exprt &function=function_call.function();

  const locationt next=std::next(from);

  if(function.id()==ID_symbol)
  {
    // called function identifier
    const symbol_exprt &symbol_expr=to_symbol_expr(function);
    const irep_idt function_id=symbol_expr.get_identifier();

    const code_function_callt::argumentst &called_arguments=
      function_call.arguments();

    if(to==next)
    {
      if(ignore_function_call_transform(function_id))
      {
        return;
      }

      if(0) // Sound on opaque function calls
      {
        abstract_state.havoc("opaque function call");
      }
      else
      {
        // For any parameter that is a pointer, top the value it is pointing
        // at.
        for(const exprt &called_arg : called_arguments)
        {
          if(called_arg.type().id()==ID_pointer)
          {
            sharing_ptrt<pointer_abstract_objectt> pointer_value=
              std::dynamic_pointer_cast<const pointer_abstract_objectt>(
                abstract_state.eval(called_arg, ns));

            assert(pointer_value);

            // Write top to the pointer
            pointer_value->write_dereference(
              abstract_state,
              ns,
              std::stack<exprt>(),
              abstract_state.abstract_object_factory(
                called_arg.type().subtype(), ns, true), false);
          }
        }

        // Top any global values
        for(const auto &symbol : ns.get_symbol_table().symbols)
        {
          if(!symbol.second.is_procedure_local())
          {
            abstract_state.assign(
              symbol_exprt(symbol.first, symbol.second.type),
              abstract_state.abstract_object_factory(symbol.second.type, ns, true),
              ns);
          }
        }
      }
    }
    else
    {
      // we have an actual call
      const symbolt &symbol=ns.lookup(function_id);
      const code_typet &code_type=to_code_type(symbol.type);
      const code_typet::parameterst &declaration_parameters=
        code_type.parameters();

      code_typet::parameterst::const_iterator parameter_it=
        declaration_parameters.begin();

      for(const exprt &called_arg : called_arguments)
      {
        if(parameter_it==declaration_parameters.end())
        {
          assert(code_type.has_ellipsis());
          break;
        }

        // Evaluate the expression that is being
        // passed into the function call (called_arg)
        abstract_object_pointert param_val=abstract_state.eval(called_arg, ns);

        // Assign the evaluated value to the symbol associated with the
        // parameter of the function
        const symbol_exprt parameter_expr(
          parameter_it->get_identifier(), called_arg.type());
        abstract_state.assign(parameter_expr, param_val, ns);

        ++parameter_it;
      }

      // Too few arguments so invalid code
      assert(parameter_it==declaration_parameters.end());
    }
  }
  else
  {
    assert(to==next);
    abstract_state.havoc("unknown opaque function call");
  }
}

/*******************************************************************\

Function: variable_sensitivity_domaint::ignore_function_call_transform

  Inputs:
   function_id - the name of the function being called

 Outputs: Returns true if the function should be ignored

 Purpose: Used to specify which CPROVER internal functions should be skipped
          over when doing function call transforms

\*******************************************************************/

bool variable_sensitivity_domaint::ignore_function_call_transform(
  const irep_idt &function_id) const
{
  static const std::set<irep_idt> ignored_internal_function={
    CPROVER_PREFIX "set_must",
    CPROVER_PREFIX "get_must",
    CPROVER_PREFIX "set_may",
    CPROVER_PREFIX "get_may",
    CPROVER_PREFIX "cleanup",
    CPROVER_PREFIX "clear_may",
    CPROVER_PREFIX "clear_must"
  };

  return ignored_internal_function.find(function_id)!=
    ignored_internal_function.cend();
}
