/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/

#include "variable_sensitivity_dependence_graph.h"

#include "data_dependency_context.h"
#include <util/json.h>
#include <util/json_expr.h>


void variable_sensitivity_dependence_grapht::eval_data_deps(
  const exprt &condition, const namespacet &ns, data_depst &deps) const
{
  const auto res=
    std::dynamic_pointer_cast<const data_dependency_contextt>(
      abstract_state.eval(condition, ns));
  
  if(res->get_data_dependencies().size() > 0)
  {
    // If the expression was able to be eval'ed to something with data
    // dependencies, then that's all we need to gather.
    for (const auto dep : res->get_data_dependencies())
      deps[dep].insert(condition);
  }
  else
  {
    // If the expression could not be eval'ed to somethign with data
    // dependencies, then it may have been some sort of compound expression,
    // so attempt to eval the data dependencies for all the operands, taking
    // the union of them all.
    for(const exprt &op : condition.operands())
    {
      eval_data_deps(op, ns, deps);
    }
  }
}

void variable_sensitivity_dependence_grapht::transform(
  locationt from,
  locationt to,
  ai_baset &ai,
   const namespacet &ns)
{
  variable_sensitivity_domaint::transform(from, to, ai, ns);

  // Find all the data dependencies in the the 'to' expression
  domain_data_deps.clear();
  if(to->is_assign())
  {
    const code_assignt &inst = to_code_assign(to->code);
    
    eval_data_deps(inst.rhs(), ns, domain_data_deps);
  }
}

bool variable_sensitivity_dependence_grapht::merge(
    const variable_sensitivity_domaint &b,
    locationt from,
    locationt to)
{
  bool changed = false;

  changed = variable_sensitivity_domaint::merge(b, from, to);

  const auto cast_b =
    dynamic_cast<const variable_sensitivity_dependence_grapht&>(b);

  for (auto bdep : cast_b.domain_data_deps)
  {
    for(exprt bexpr : bdep.second)
    {
      auto result = domain_data_deps[bdep.first].insert(bexpr);
      changed |= result.second;
    }
  }

  return changed;
}

  void variable_sensitivity_dependence_grapht::merge_three_way_function_return(
    const ai_domain_baset &function_call,
    const ai_domain_baset &function_start,
    const ai_domain_baset &function_end,
    const namespacet &ns)
  {
    variable_sensitivity_domaint::merge_three_way_function_return(
      function_call,
      function_start,
      function_end,
      ns);
    // FIXME: Need to do 3-way merge of domain data deps too.
    // FIXME: Need to merge anything in function_end which has changed since function_start, into
    // FIXME: a new state based on that from function_call
  }


void variable_sensitivity_dependence_grapht::output(
   std::ostream &out,
   const ai_baset &ai,
   const namespacet &ns) const
{
  if(!domain_data_deps.empty())
  {
      out << "Data dependencies: ";
      bool first = true;
      for (auto &dep : domain_data_deps)
      {
        if(!first)
          out << ", ";

        out << dep.first->location_number;
        out << " [";
        bool first_expr = true;
        for (auto &expr : dep.second)
        {
          if (!first_expr)
            out << ", ";

          out << from_expr(ns, "", expr);
          first_expr = false;
        }
        out << "]";
        first = false;
      }
      out << std::endl;
    }
}

/**
 * \brief Outputs the current value of the domain.
 * 
 * \param ai the abstract interpreter
 * \param ns the namespace
 * 
 * \return the domain, formatted as a JSON object.
 */
jsont variable_sensitivity_dependence_grapht::output_json(
  const ai_baset &ai,
  const namespacet &ns) const
{
  json_arrayt graph;

  for(const auto &dep : domain_data_deps)
  {
    json_objectt &link=graph.push_back().make_object();
    link["locationNumber"]=
      json_numbert(std::to_string(dep.first->location_number));
    link["sourceLocation"]=json(dep.first->source_location);
      json_stringt(dep.first->source_location.as_string());
    link["type"]=json_stringt("data");

    const std::set<exprt> &expr_set=dep.second;
    json_arrayt &expressions=link["expressions"].make_array();

    for(const exprt &e : expr_set)
    {
      json_objectt &object=expressions.push_back().make_object();
      object["expression"]=json_stringt(from_expr(ns, "", e));
      object["certainty"]=json_stringt("maybe");
    }
  }

  return graph;
}
