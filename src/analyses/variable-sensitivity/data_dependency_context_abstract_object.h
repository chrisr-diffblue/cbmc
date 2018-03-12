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
// Equivalents in Variable Sensitivity are, I think:
// * merge -> merge(AO, AO, bool)
// * transform -> possibly update_location_context/read/write
// * output -> needs some thought
// 

#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DATA_DEPENDENCY_CONTEXT_ABSTRACT_OBJECT_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DATA_DEPENDENCY_CONTEXT_ABSTRACT_OBJECT_H

#include "analyses/variable-sensitivity/dependency_context_abstract_object.h"
#include "variable_sensitivity_domain.h"
#include "dependency_context_abstract_object.h"
#include "analyses/cfg_dominators.h"

class data_dependency_context_abstract_objectt:
  public dependency_context_abstract_objectt {
public:
  // These constructors mirror those in the base abstract_objectt, but with
  // the addition of an extra argument which is the abstract_objectt to wrap.
  explicit data_dependency_context_abstract_objectt(
    const abstract_object_pointert child,
    const typet &type):
      dependency_context_abstract_objectt(child, type)
  {
  }

  data_dependency_context_abstract_objectt(
    const abstract_object_pointert child,
    const typet &type,
    bool top,
    bool bottom):
      dependency_context_abstract_objectt(child, type, top, bottom)
  {
  }

  explicit data_dependency_context_abstract_objectt(
    const abstract_object_pointert child,
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns):
      dependency_context_abstract_objectt(child, expr, environment, ns)
  {
  }

  virtual abstract_object_pointert write(
    abstract_environmentt &environment,
    const namespacet &ns,
    const std::stack<exprt> stack,
    const exprt &specifier,
    const abstract_object_pointert value,
    bool merging_write) const override;

  virtual abstract_object_pointert update_location_context(
    const abstract_objectt::locationst &locations,
    const bool update_sub_elements) const override;


  virtual void output(
    std::ostream &out, const class ai_baset &ai, const namespacet &ns) const
  override;

protected:
  CLONE

  virtual abstract_object_pointert merge(
    abstract_object_pointert other) const override;

  virtual abstract_object_pointert abstract_object_merge_internal(
    const abstract_object_pointert other) const override;


private:
  typedef std::set<goto_programt::const_targett> dependencest;
  dependencest data_deps;

  void dump_data_deps(const std::string msg) const
  {

    std::clog << "<<< " << msg << ": Full data deps: ";
    bool comma = false;
    for(auto d : data_deps)
    {
      std::clog << (comma ? ", " : "") << d->location_number;
      comma = true;
    }
    std::clog << ">>>" << std::endl;
  }
};

/**
 * Templated extension of the abstract implementation, used as a wrapper around
 * other abstract_objectt classes to enable the factory to instantiate the
 * context information
 */
template <class AOT>
class dominator_context_instance_abstract_objectt:
  public data_dependency_context_abstract_objectt
{
public:
  explicit dominator_context_instance_abstract_objectt(const typet &type):
    data_dependency_context_abstract_objectt(
      abstract_object_pointert(new AOT(type)), type) {}

  dominator_context_instance_abstract_objectt(
    const typet &type,
    bool top,
    bool bottom):
      data_dependency_context_abstract_objectt(
        abstract_object_pointert(new AOT(type, top, bottom)),
          type,
          top,
          bottom) {}

  explicit dominator_context_instance_abstract_objectt(
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns):
      data_dependency_context_abstract_objectt(
        abstract_object_pointert(new AOT(expr, environment, ns)),
        expr,
        environment,
        ns) {}
};

// NOLINTNEXTLINE(whitespace/line_length)
#endif /* CPROVER_ANALYSES_VARIABLE_SENSITIVITY_DATA_DEPENDENCY_CONTEXT_ABSTRACT_OBJECT_H */
