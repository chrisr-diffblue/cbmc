#include "dominator_context_abstract_object.h"



/*
 * transform(location from, location to, AI env, namespace ns):
 *   if from location is function call:
 *      if recursive (e.g. from/to are same function):
 *           contrl_dependencies(from, to, depgraph)
 *      else:
 *          // inter-function call
 *          set control deps of return location (e.g. from.next) to union
 *            (control_deps here, control_deps of return location
 *          clear control_deps here
 *   else not a function call:
 *      control_dependencies(from ,to, depgraph)
 *   finally, data_dependencies(from, to, depgraph)
 *
 *
 *
 *
 * merge(AI env, from ,to):
 *   set curernt contol deps to union(current control deps, src control deps)
 *   set data_deps to union(current data deps, src data deps)
 *   return true if any mods, false otherwise
 *
 *
 *
 *  What does populate_dep_graph do, where is it called from?
 *
 *  dep_graph_domaint has:
 *    tvt has_values
 *    nodeid
 *    control deps (set of locationt)
 *    data deps (set of locationt)
 *
 *
 *  dependence_grapht inherits from AIT<dep_graph_domaint> and from
 *  grapht<dep_nodet> and sets up local reaching_definitions_analysist
 *
 *  initialize does:
 *    initialize the ait (straight pass through)
 *    then (depending on prototype) either:
 *       applies reaching definitions to the given goto_function
 *    or:
 *       applies the post_dominators obeject for a given location
 *
 * NOTE: dependence_grapht keeps global state - its the abstract_env, so this
 * e.g. has an entry to get the 'state' for a given locationt
 */

void dominator_context_abstract_objectt::output(
  std::ostream &out, const class ai_baset &ai, const namespacet &ns) const
{
  std::cout << "[dominator context output called]" << std::endl;
  out << "[Dominator stuff here]";
}
