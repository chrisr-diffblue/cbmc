/*******************************************************************\

 Module: analyses variable-sensitivity-dependence-graph

 FIXME: Document...

\*******************************************************************/
#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H

#include "variable_sensitivity_domain.h"

#include <iostream>

class variable_sensitivity_dependence_grapht:
  public variable_sensitivity_domaint
{
public:

  void transform(
    locationt from,
    locationt to,
    ai_baset &ai,
    const namespacet &ns) override;

  virtual bool merge(
    const variable_sensitivity_domaint &b,
    locationt from,
    locationt to) override;

  virtual void merge_three_way_function_return(
    const ai_domain_baset &function_call,
    const ai_domain_baset &function_start,
    const ai_domain_baset &function_end,
    const namespacet &ns) override;


  void output(
    std::ostream &out,
    const ai_baset &ai,
    const namespacet &ns) const override;

  jsont output_json(
    const ai_baset &ai,
    const namespacet &ns) const override;

private:
  // General case: typedef std::set<goto_programt::const_targett> data_depst;
  // Toyota case:
  typedef std::map<goto_programt::const_targett, std::set<exprt>> data_depst;
  data_depst domain_data_deps;

  void eval_data_deps(
    const exprt &condition, const namespacet &ns, data_depst &deps) const;
};

#endif // CPROVER_ANALYSES_VARIABLE_SENSITIVITY_VARIABLE_SENSITIVITY_DEPENDENCE_GRAPH_H
