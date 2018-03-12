/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/
#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H

#include "variable_sensitivity_domain.h"
#include <analyses/dependence_graph.h>

class variable_sensitivity_dependence_grapht;

class variable_sensitivity_dependence_graph_domaint:
  public variable_sensitivity_domaint
{
public:
  typedef grapht<dep_nodet>::node_indext node_indext;

  variable_sensitivity_dependence_graph_domaint():
    has_values(false),
    node_id(std::numeric_limits<node_indext>::max())
  {
  }

  void populate_dep_graph(
    variable_sensitivity_dependence_grapht &, goto_programt::const_targett) const;

//  virtual void output(
//    std::ostream &out,
//    const ai_baset &ai,
//    const namespacet &ns) const override;


private:
  tvt has_values;
  node_indext node_id;

  typedef std::set<goto_programt::const_targett> depst;
  depst control_deps, data_deps;
};

class variable_sensitivity_dependence_grapht:
  public ait<variable_sensitivity_dependence_graph_domaint>,
  public grapht<dep_nodet>
{
public:
  typedef std::map<irep_idt, cfg_post_dominatorst> post_dominators_mapt;

  explicit variable_sensitivity_dependence_grapht(const namespacet &_ns):
    ns(_ns),
    rd(ns)
  { }

  void initialize(const goto_functionst &goto_functions)
  {
    ait<variable_sensitivity_dependence_graph_domaint>::initialize(goto_functions);
    rd(goto_functions, ns);
  }

  void initialize(const goto_programt &goto_program)
  {
    ait<variable_sensitivity_dependence_graph_domaint>::initialize(goto_program);

    if(!goto_program.empty())
    {
      const irep_idt id=goto_programt::get_function_id(goto_program);
      cfg_post_dominatorst &pd=post_dominators[id];
      pd(goto_program);
    }
  }

  void finalize()
  {
    for(const auto &location_state : state_map)
    {
      location_state.second.populate_dep_graph(*this, location_state.first);
    }
  }

protected:
  const namespacet &ns;

  post_dominators_mapt post_dominators;
  reaching_definitions_analysist rd;
};

#endif // CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
