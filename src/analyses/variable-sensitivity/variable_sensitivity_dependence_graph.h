/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/
#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H

#include "variable_sensitivity_domain.h"

class variable_sensitivity_dependence_graph_domaint:
  public variable_sensitivity_domaint
{
};

class variable_sensitivity_dependence_grapht:
  public ait<variable_sensitivity_dependence_graph_domaint>,
  public grapht<dep_nodet>
{
public:
  explicit variable_sensitivity_dependence_grapht(const namespacet &_ns):
    ns(_ns),
    rd(ns)
  { }

protected:
  const namespacet &ns;
  reaching_definitions_analysist rd;

};
#endif // CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
