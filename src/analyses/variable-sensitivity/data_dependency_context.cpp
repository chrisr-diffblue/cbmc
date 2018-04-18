#include <algorithm>
#include <iterator>

#include "data_dependency_context.h"

/**
 * Determine whether 'this' abstract_object has been modified in comparison
 * to a previous 'before' state.
 *
 * \param before the abstract_object_pointert to use as a reference to
 * compare against
 *
 * \return true if 'this' is considered to have been modified in comparison
 * to 'before', false otherwise.
 */
bool data_dependency_contextt::has_been_modified(
  const abstract_object_pointert before) const
{
  if(this->write_location_contextt::has_been_modified(before))
    return true;

  auto cast_before=
    std::dynamic_pointer_cast<const data_dependency_contextt>
      (before);

  if(!cast_before)
  {
    // The other context is not something we understand, so must assume
    // that the abstract_object has been modified
    return true;
  }

  // Check whether the data depdendencies hace changed as well
  abstract_objectt::locationst intersection;
  std::set_intersection(
    data_deps.cbegin(),
    data_deps.cend(),
    cast_before->data_deps.cbegin(),
    cast_before->data_deps.cend(),
    std::inserter(intersection, intersection.end()),
    location_ordert());
  bool all_matched=intersection.size()==data_deps.size() &&
                   intersection.size()==cast_before->data_deps.size();

  if(!all_matched)
    return true;

  intersection.clear();
  std::set_intersection(
    data_dominators.cbegin(),
    data_dominators.cend(),
    cast_before->data_dominators.cbegin(),
    cast_before->data_dominators.cend(),
    std::inserter(intersection, intersection.end()),
    location_ordert());

  all_matched=intersection.size()==data_dominators.size() &&
              intersection.size()==cast_before->data_dominators.size();

  return !all_matched;
}


abstract_object_pointert data_dependency_contextt::write(
  abstract_environmentt &environment,
  const namespacet &ns,
  const std::stack<exprt> stack,
  const exprt &specifier,
  const abstract_object_pointert value,
  bool merging_write) const
{
  const auto updated_parent =
    this->write_location_contextt::write(
      environment, ns, stack, specifier, value, merging_write);

  const auto cast_parent =
    std::dynamic_pointer_cast<const data_dependency_contextt>
      (updated_parent);

  const auto &result=
    std::dynamic_pointer_cast<data_dependency_contextt>(
      cast_parent->mutable_clone());

  const auto cast_value=
    std::dynamic_pointer_cast<const data_dependency_contextt>(value);

  // FIXME: Do a more intelligent merge here - in particular, only clone
  // FIXME: if we are actually inserting any new depdendencies...
  result->insert_data_deps(cast_value->data_deps);

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
data_dependency_contextt::update_location_context(
  const abstract_objectt::locationst &locations,
  const bool update_sub_elements) const
{
  const auto updated_parent =
    this->write_location_contextt::update_location_context
      (locations, update_sub_elements);

  auto cast_parent =
    std::dynamic_pointer_cast<const data_dependency_contextt>
      (updated_parent);

  const auto &result=
    std::dynamic_pointer_cast<data_dependency_contextt>(
      cast_parent->mutable_clone());

  // FIXME: Do a more intelligent merge here - in particular, only clone
  // FIXME: if we are actually inserting any new depdendencies...
  result->insert_data_deps(locations);

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
abstract_object_pointert data_dependency_contextt::merge(
  abstract_object_pointert other) const
{
  auto cast_other=
    std::dynamic_pointer_cast<const data_dependency_contextt>(other);

  if(cast_other)
  {
    const auto merged_parent =
      this->write_location_contextt::merge(other);

    auto cast_merged_parent = std::dynamic_pointer_cast<const data_dependency_contextt>(merged_parent);

    const auto &result=
      std::dynamic_pointer_cast<data_dependency_contextt>(
        cast_merged_parent->mutable_clone());

    result->insert_data_deps(cast_other->data_deps);
    // On a merge, data_dominators are the intersection of this object and the
    // other object. In other words, the dominators at this merge point are
    // those dominators that exist in all possible execution paths to this
    // merge point.
    // FIXME: This can probably be done a lot more efficently than this clear()
    // FIXME: and rebuild approach...
    result->data_dominators.clear();
    std::set_intersection(
      data_dominators.begin(), data_dominators.end(),
      cast_other->data_dominators.begin(), cast_other->data_dominators.end(),
      std::inserter(result->data_dominators, result->data_dominators.end()),
      location_ordert());

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
data_dependency_contextt::abstract_object_merge_internal(
  const abstract_object_pointert other) const
{
  auto other_context=
    std::dynamic_pointer_cast<const data_dependency_contextt>(other);

  if(other_context)
  {
    const auto merged_parent =
      this->write_location_contextt::abstract_object_merge_internal
        (other);

    auto cast_merged_parent = std::dynamic_pointer_cast<const data_dependency_contextt>(merged_parent);

    const auto &result=
      std::dynamic_pointer_cast<data_dependency_contextt>(
        cast_merged_parent->mutable_clone());

    // FIXME: Do a more intelligent merge here - in particular, only clone
    // FIXME: if we are actually inserting any new depdendencies...
    result->insert_data_deps(other_context->data_deps);

    return result;
  }
  return shared_from_this();
}

std::set<goto_programt::const_targett> data_dependency_contextt::get_data_dependencies() const
{
  std::set<goto_programt::const_targett> result;
  for (auto d : data_deps) result.insert(d);
  return result;
}

std::set<goto_programt::const_targett> data_dependency_contextt::get_data_dominators() const
{
  std::set<goto_programt::const_targett> result;
  return result;
}

void data_dependency_contextt::output(
  std::ostream &out, const class ai_baset &ai, const namespacet &ns) const
{
  this->write_location_contextt::output(out, ai, ns);

  out << "[Data dependencies: ";

  bool comma = false;
  for(auto d : data_deps)
  {
    out << (comma ? ", " : "") << d->location_number;
    comma = true;
  }
  out << ']';

  out << "[Data dominators: ";

  comma = false;
  for(auto d : data_dominators)
  {
    out << (comma ? ", " : "") << d->location_number;
    comma = true;
  }
  out << ']';
}
