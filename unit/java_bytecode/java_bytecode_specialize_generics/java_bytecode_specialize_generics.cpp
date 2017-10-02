/*******************************************************************\

 Module: Unit tests for parsing generic classes

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#include <catch.hpp>

#include <istream>
#include <memory>

#include <util/config.h>
#include <util/language.h>
#include <util/message.h>
#include <java_bytecode/java_bytecode_language.h>
#include <java_bytecode/generate_java_generic_method.h>
#include <iostream>
#include <util/namespace.h>
#include <std_types.h>
#include <expr_iterator.h>


SCENARIO(
  "java_bytecode_specialize_generics",
  "[core][java_bytecode][java_bytecode_specialize_generics]")
{
  std::unique_ptr<languaget>java_lang(new_java_bytecode_language());

  // Configure the path loading
  cmdlinet command_line;
  command_line.set(
    "java-cp-include-files",
    "./java_bytecode/java_bytecode_specialize_generics");
  config.java.classpath.push_back(
    "./java_bytecode/java_bytecode_specialize_generics");

  std::istringstream java_code_stream("ignored");
  null_message_handlert message_handler;


  // Configure the language, load the class files
  java_lang->get_language_options(command_line);
  java_lang->set_message_handler(message_handler);
  java_lang->parse(java_code_stream, "generics.class");
  symbol_tablet new_symbol_table;
  java_lang->typecheck(new_symbol_table, "");
  java_lang->final(new_symbol_table);

  generate_java_generic_methodt
    instantiate_generic_method{message_handler};


  GIVEN("Some class files with Generics")
  {
    WHEN("Methods with generic signatures")
    {
      REQUIRE(
        new_symbol_table
          .has_symbol("java::generics$bound_element.f:()Ljava/lang/Number;"));

      THEN("Specialise the method into a method with a concrete return type")
      {
        const symbolt &method_symbol=
          new_symbol_table.lookup(
            "java::generics$bound_element.f:()Ljava/lang/Number;");
        const typet &symbol_type=method_symbol.type;

        REQUIRE(symbol_type.id()==ID_code);
        const code_typet &code=to_code_type(symbol_type);
        const typet &method_return_type=code.return_type();

        REQUIRE(is_java_generic_parameter(method_return_type));
        const java_generic_parametert
          &generic_ret_type=to_java_generic_parameter(method_return_type);
        const symbol_typet &type_var=generic_ret_type.type_variable();
        REQUIRE(type_var.get_identifier()==
                "java::generics$bound_element::NUM");

        const symbolt &concrete_type_sym=new_symbol_table.lookup(
          "java::java.lang.Integer");
        symbol_typet concrete_type=symbol_typet("java::java.lang.Integer");
        concrete_type.set(ID_C_base_name, concrete_type_sym.base_name);

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
          {symbol_typet("java::generics$bound_element::NUM"),concrete_type}
        };
        symbolt &specialized_method=instantiate_generic_method(
          method_symbol,
          concrete_types,
          new_symbol_table);


      }


      REQUIRE(
        new_symbol_table
          .has_symbol("java::generics$bound_element.g:(Ljava/lang/Number;)V"));




      THEN("Specialise the method into a method with a concrete parameter "
             "types")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$bound_element.g:(Ljava/lang/Number;)V");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick a concrete type
        const symbolt &concrete_type_sym=new_symbol_table.lookup(
          "java::java.lang.Integer");
        symbol_typet concrete_type=symbol_typet("java::java.lang.Integer");
        concrete_type.set(ID_C_base_name, concrete_type_sym.base_name);

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
            {symbol_typet("java::generics$bound_element::NUM"),concrete_type}
            };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);


        REQUIRE(true);
      }


      REQUIRE(
        new_symbol_table
          .has_symbol(
            "java::generics$double_element.insert:(Ljava/lang/Object;Ljava/lang/Object;)V"));


      THEN("Specialise a method with multiple generic parameters"
             "types")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$double_element.insert:(Ljava/lang/Object;Ljava/lang/Object;)V");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick a concrete type
        symbol_typet concrete_integer_type=symbol_typet("java::java.lang.Integer");

        symbol_typet concrete_double_type
          =symbol_typet("java::java.lang.Double");

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
          {symbol_typet("java::generics$double_element::A"),
            concrete_integer_type},
          {symbol_typet("java::generics$double_element::B"),
            concrete_double_type}
        };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);

        REQUIRE(true);
      }


      REQUIRE(
        new_symbol_table
          .has_symbol(
            "java::generics$double_element.setMap:(Ljava/util/Map;)V"));


      THEN("Specialise a method with generic type with multiple generic "
             "parameters")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$double_element.setMap:(Ljava/util/Map;)V");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick concrete types
        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
            {symbol_typet("java/util/Map<TA;TB;>::A"),
              symbol_typet("java::java.lang.Integer")},
            {symbol_typet("java/util/Map<TA;TB;>::B"),
              symbol_typet("java::java.lang.Double")}
        };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);

        REQUIRE(true);
      }


      REQUIRE(
        new_symbol_table
          .has_symbol(
            "java::generics$compound_element.setElem:(Ljava/util/List;)V"));


      THEN("Specialise a method with a compound generic parameter type")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$compound_element.setElem:(Ljava/util/List;)V");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick a concrete type
        symbol_typet concrete_integer_type=symbol_typet("java::java.lang.Integer");
        symbol_typet concrete_double_type
          =symbol_typet("java::java.lang.Double");

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
          {symbol_typet("java/util/List<TB;>::B"),
            concrete_integer_type}
        };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);

        REQUIRE(true);
      }


      REQUIRE(
        new_symbol_table
          .has_symbol(
            "java::generics$compound_element.setFixedElem:(Ljava/util/List;)"
              "V"));


      THEN("Specialise a method with a bound compound generic parameter type")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$compound_element.setFixedElem:"
                     "(Ljava/util/List;)V");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick a concrete type
        symbol_typet concrete_integer_type=symbol_typet("java::java.lang.Integer");
        symbol_typet concrete_double_type
          =symbol_typet("java::java.lang.Double");

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
          {symbol_typet("java/util/List<TB;>::B"),
            concrete_integer_type}
        };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);

        REQUIRE(true);
      }

      // ()Ljava/util/List
      REQUIRE(
        new_symbol_table
          .has_symbol(
            "java::generics$compound_element.getElem:()Ljava/util/List;"));


      THEN("Specialise a method with a compound generic return type")
      {
        const symbolt &generic_method_symbol=
          new_symbol_table.
            lookup("java::generics$compound_element.getElem:()"
                     "Ljava/util/List;");

        REQUIRE(generic_method_symbol.type.id()==ID_code);

        // Pick a concrete type
        symbol_typet concrete_integer_type=symbol_typet("java::java.lang.Integer");
        symbol_typet concrete_double_type
          =symbol_typet("java::java.lang.Double");

        generate_java_generic_methodt::type_variable_instantiationst
          concrete_types{
          {symbol_typet("java/util/List<TB;>::B"),
            concrete_integer_type}
        };
        symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          concrete_types,
          new_symbol_table);

        REQUIRE(true);
      }

    }
  }
}
