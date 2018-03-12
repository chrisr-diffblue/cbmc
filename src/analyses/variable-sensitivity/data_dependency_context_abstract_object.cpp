#include "data_dependency_context_abstract_object.h"


abstract_object_pointert data_dependency_context_abstract_objectt::write(
  abstract_environmentt &environment,
  const namespacet &ns,
  const std::stack<exprt> stack,
  const exprt &specifier,
  const abstract_object_pointert value,
  bool merging_write) const
{
  const auto updated_parent =
    this->dependency_context_abstract_objectt::write(
      environment, ns, stack, specifier, value, merging_write);

  auto cast_parent =
    std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>
      (updated_parent);

  const auto &result=
    std::dynamic_pointer_cast<data_dependency_context_abstract_objectt>(
      cast_parent->mutable_clone());

  auto cast_value=
    std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>(value);

  // TODO: Do a more intelligent merge here???
  for(auto dep : cast_value->data_deps)
  {
    std::clog << " ### Logging dependency " << dep->location_number << " in"
      " write()" << std::endl;
    result->data_deps.insert(dep);
  }

  result->dump_data_deps("dominator_context::write");
  return result;
}

/**
 * Update the location context for an abstract object, potentially
 * propogating the update to any children of this abstract object.
 *
 * \param locations the set of locations to be updated
 * \param update_sub_elements if true, propogate the update operation to any
 * children of this abstract object
 *
 * \return a clone of this abstract object with it's location context
 * updated
 */
abstract_object_pointert
data_dependency_context_abstract_objectt::update_location_context(
  const abstract_objectt::locationst &locations,
  const bool update_sub_elements) const
{
  const auto updated_parent =
    this->dependency_context_abstract_objectt::update_location_context
      (locations, update_sub_elements);

  auto cast_parent =
    std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>
      (updated_parent);

  const auto &result=
    std::dynamic_pointer_cast<data_dependency_context_abstract_objectt>(
      cast_parent->mutable_clone());

  // TODO: Do a more intelligent merge here???
  for(auto d : locations)
  {
    result->data_deps.insert(d);
  }

  result->dump_data_deps("dominator_context::update_location_context");
  return result;
}

/**
 * Create a new abstract object that is the result of merging this abstract
 * object with a given abstract_object
 *
 * \param other the abstract object to merge with
 *
 * \return the result of the merge, or 'this' if the merge would not change
 * the current abstract object
 */
abstract_object_pointert data_dependency_context_abstract_objectt::merge(
  abstract_object_pointert other) const
{
  auto cast_other=
    std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>(other);

  if(cast_other)
  {
    const auto merged_parent =
      this->dependency_context_abstract_objectt::merge(other);

    auto cast_merged_parent = std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>(merged_parent);

    const auto &result=
      std::dynamic_pointer_cast<data_dependency_context_abstract_objectt>(
        cast_merged_parent->mutable_clone());

    for(auto d : cast_other->data_deps)
    {
      result->data_deps.insert(d);
    }

    result->dump_data_deps("dominator_context::merge");
    return result;
  }

  return abstract_objectt::merge(other);
}

/**
 * Helper function for abstract_objectt::abstract_object_merge to perform any
 * additional actions after the base abstract_object_merge has completed it's
 * actions but immediately prior to it returning. As such, this function gives
 * the ability to perform additional work for a merge.
 *
 * For the dependency context, this additional work is the tracking of
 * last_written_locations across the merge
 *
 * \param other the object to merge with this
 *
 * \return the result of the merge
 */
abstract_object_pointert
data_dependency_context_abstract_objectt::abstract_object_merge_internal(
  const abstract_object_pointert other) const
{
  auto other_context=
    std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>(other);

  if(other_context)
  {
    const auto merged_parent =
      this->dependency_context_abstract_objectt::abstract_object_merge_internal
        (other);

    auto cast_merged_parent = std::dynamic_pointer_cast<const data_dependency_context_abstract_objectt>(merged_parent);

    const auto &result=
      std::dynamic_pointer_cast<data_dependency_context_abstract_objectt>(
        cast_merged_parent->mutable_clone());

    for(auto d : other_context->data_deps)
    {
      result->data_deps.insert(d);
    }

    result->dump_data_deps("dominator_context::abstract_object_merge_internal");
    return result;
  }
  return shared_from_this();
}

void data_dependency_context_abstract_objectt::output(
  std::ostream &out, const class ai_baset &ai, const namespacet &ns) const
{
  this->dependency_context_abstract_objectt::output(out, ai,
                                                                    ns);

  out << "<Data dependencies: ";

  bool comma = false;
  for(auto d : data_deps)
  {
    out << (comma ? ", " : "") << d->location_number;
    comma = true;
  }
  out << '>';
}
