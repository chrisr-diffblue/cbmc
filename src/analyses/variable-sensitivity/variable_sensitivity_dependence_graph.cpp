/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/

#include "variable_sensitivity_dependence_graph.h"


void variable_sensitivity_dependence_graph_domaint::populate_dep_graph(
  variable_sensitivity_dependence_grapht &, goto_programt::const_targett) const
{
}

void variable_sensitivity_dependence_graph_domaint::output(
    std::ostream &out,
    const ai_baset &ai,
    const namespacet &ns) const
{
  out << "[variable-sensitvity-dependence-graph stuff here]";
}

