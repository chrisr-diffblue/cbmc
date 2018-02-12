// Rough implementaition plan:
//
// * Extend the dependency_context_abstract_object to:
//   - Handle merge of deps (how?)
//   - perform transforms (how?)
//   - Output deps
//
// What about json output? Does Variable sensitivity support that already? If not,
// more work to do...
//
// Existing implementation does the following AI operations:
// * merge(domain, from, to)
// * transform(from, to, ai, ns)
// * output
// * output_json
// 

#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DOMINATOR_CONTEXT_ABSTRACT_OBJECT_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DOMINATOR_CONTEXT_ABSTRACT_OBJECT_H

#include "analyses/variable-sensitivity/dependency_context_abstract_object.h"
#include "variable_sensitivity_domain.h"
#include "dependency_context_abstract_object.h"
#include "analyses/cfg_dominators.h"

class dominator_context_abstract_objectt:
  public dependency_context_abstract_objectt {
public:
  // These constructors mirror those in the base abstract_objectt, but with
  // the addition of an extra argument which is the abstract_objectt to wrap.
  explicit dominator_context_abstract_objectt(
    const abstract_object_pointert child,
    const typet &type):
      dependency_context_abstract_objectt(child, type)
  { }

  dominator_context_abstract_objectt(
    const abstract_object_pointert child,
    const typet &type,
    bool top,
    bool bottom):
      dependency_context_abstract_objectt(child, type, top, bottom)
  { }

  explicit dominator_context_abstract_objectt(
    const abstract_object_pointert child,
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns):
      dependency_context_abstract_objectt(child, expr, environment, ns)
  { }

private:
  cfg_post_dominatorst dominators;
};

/**
 * Templated extension of the abstract implementation, used as a wrapper around
 * other abstract_objectt classes to enable the factory to instantiate the
 * context information
 */
template <class AOT>
class dominator_context_instance_abstract_objectt:
  public dominator_context_abstract_objectt
{
public:
  explicit dominator_context_instance_abstract_objectt(const typet &type):
    dominator_context_abstract_objectt(
      abstract_object_pointert(new AOT(type)), type) {}

  dominator_context_instance_abstract_objectt(
    const typet &type,
    bool top,
    bool bottom):
      dominator_context_abstract_objectt(
        abstract_object_pointert(new AOT(type, top, bottom)),
          type,
          top,
          bottom) {}

  explicit dominator_context_instance_abstract_objectt(
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns):
      dominator_context_abstract_objectt(
        abstract_object_pointert(new AOT(expr, environment, ns)),
        expr,
        environment,
        ns) {}
};

// NOLINTNEXTLINE(whitespace/line_length)
#endif /* CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DOMINATOR_CONTEXT_ABSTRACT_OBJECT_H */
