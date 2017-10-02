/*******************************************************************\

Module: Generate Java Generic Method - Instantiate a generic method
        with concrete type information.

Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#ifndef GENERATE_JAVA_GENERIC_METHOD_H
#define GENERATE_JAVA_GENERIC_METHOD_H

#include <util/message.h>
#include <util/symbol_table.h>
#include <util/std_types.h>
#include <java_bytecode/java_types.h>

class generate_java_generic_methodt
{
public:
  typedef std::initializer_list<std::pair<const symbol_typet, const symbol_typet>>
    type_variable_instantiationst;

  typedef std::unordered_map<const symbol_typet, const symbol_typet,
  irep_hash>
    type_variable_instantiation_mapt;

  generate_java_generic_methodt(message_handlert &message_handler);

  symbolt& operator()(
    const symbolt &generic_method,
    const type_variable_instantiationst &concrete_types,
    symbol_tablet &symbol_table) const;

  const std::string instantiation_decoration(
    type_variable_instantiationst concrete_types) const;

  const void instantiate_java_generic_parameter(
    java_generic_parametert &generic_parameter,
    const type_variable_instantiation_mapt &concrete_types) const;

  const void instantiate_java_generic_type(
    java_generic_typet &generic_type,
    const type_variable_instantiation_mapt &concrete_types) const;

  void decorate_identifier(
    irept &expr,
    const dstringt &identifier,
    const dstringt &pattern,
    const dstringt &decoration) const;

private:
  message_handlert &message_handler;
};

#endif // GENERATE_JAVA_GENERIC_METHOD_H
