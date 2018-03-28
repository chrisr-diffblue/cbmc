/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/
#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H

#include "variable_sensitivity_domain.h"

class variable_sensitivity_dependence_grapht:
  public variable_sensitivity_domaint
{
public:
  void output(
    std::ostream &out,
    const ai_baset &ai,
    const namespacet &ns) const;
};

#endif // CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
