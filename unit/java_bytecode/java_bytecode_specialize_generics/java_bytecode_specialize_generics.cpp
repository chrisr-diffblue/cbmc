/*******************************************************************\

 Module: Unit tests for specializing generic methods

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
#include <util/std_types.h>
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

      THEN("Specialize a method with a generic return type")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$bound_element.f:()Ljava/lang/Number;")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$bound_element.f:()Ljava/lang/Number;");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        const typet &method_return_type=code.return_type();
        REQUIRE(is_java_generic_parameter(method_return_type));

        const java_generic_parametert
          &generic_ret_type=to_java_generic_parameter(method_return_type);
        const symbol_typet &type_var=generic_ret_type.type_variable();
        REQUIRE(type_var.get_identifier()==
                "java::generics$bound_element::NUM");

        const symbolt &specialized_method_symbol=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$bound_element::NUM"),
              symbol_typet("java::java.lang.Integer")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$bound_element.f:()Ljava/lang/Number;<java::java.lang.Integer>")
        );
        const typet &new_symbol_type=specialized_method_symbol.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        const typet &new_method_return_type=new_code.return_type();
        REQUIRE(!is_java_generic_parameter(new_method_return_type));

        REQUIRE(
          new_method_return_type.subtype()==symbol_typet("java::java.lang.Integer")
        );
      }



      THEN("Specialize a method with a generic parameter types")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$bound_element.g:(Ljava/lang/Number;)V")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$bound_element.g:(Ljava/lang/Number;)V");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        REQUIRE(code.parameters().size()==2);

        const code_typet::parametert &param=code.parameters()[1];
        REQUIRE(is_java_generic_parameter(param.type()));

        const java_generic_parametert &generic_param=
          to_java_generic_parameter(param.type());
        const symbol_typet &type_var=generic_param.type_variable();
        REQUIRE(type_var.get_identifier()==
                "java::generics$bound_element::NUM");

        const symbolt &specialized_method_symbol=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$bound_element::NUM"),
              symbol_typet("java::java.lang.Integer")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$bound_element.g:(Ljava/lang/Number;)V<java::java.lang.Integer>")
        );
        const typet &new_symbol_type=specialized_method_symbol.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        REQUIRE(new_code.parameters().size()==2);

        const code_typet::parametert &new_param=new_code.parameters()[1];
        REQUIRE(!is_java_generic_parameter(new_param.type()));
        REQUIRE(new_param.type().subtype()==symbol_typet("java::java.lang.Integer"));
      }




      THEN("Specialise a method with multiple generic parameters")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$double_element.insert:(Ljava/lang/Object;Ljava/lang/Object;)V")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$double_element.insert:(Ljava/lang/Object;Ljava/lang/Object;)V");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        REQUIRE(code.parameters().size()==3);

        const code_typet::parametert &param_one=code.parameters()[1];
        REQUIRE(is_java_generic_parameter(param_one.type()));

        const code_typet::parametert &param_two=code.parameters()[2];
        REQUIRE(is_java_generic_parameter(param_two.type()));

        const java_generic_parametert &generic_param_one=
          to_java_generic_parameter(param_one.type());
        const symbol_typet &type_var_one=generic_param_one.type_variable();
        REQUIRE(type_var_one.get_identifier()==
                "java::generics$double_element::A");

        const java_generic_parametert &generic_param_two=
          to_java_generic_parameter(param_two.type());
        const symbol_typet &type_var_two=generic_param_two.type_variable();
        REQUIRE(type_var_two.get_identifier()==
                "java::generics$double_element::B");

        const symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$double_element::A"),
              symbol_typet("java::java.lang.Integer")},
            { symbol_typet("java::generics$double_element::B"),
              symbol_typet("java::java.lang.Double")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$double_element.insert:(Ljava/lang/Object;"
              "Ljava/lang/Object;)V<java::java.lang.Integer,java::java.lang.Double>")
        );
        const typet &new_symbol_type=specialized_method.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        REQUIRE(new_code.parameters().size()==3);

        const code_typet::parametert &new_param_one=new_code.parameters()[1];
        REQUIRE(!is_java_generic_parameter(new_param_one.type()));
        REQUIRE(new_param_one.type().subtype()==symbol_typet("java::java.lang.Integer"));

        const code_typet::parametert &new_param_two=new_code.parameters()[2];
        REQUIRE(!is_java_generic_parameter(new_param_two.type()));
        REQUIRE(new_param_two.type().subtype()==symbol_typet("java::java.lang.Double"));
      }




      THEN("Specialise a method with a parameterized generic argument")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$double_element.setMap:(Ljava/util/Map;)V")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$double_element.setMap:(Ljava/util/Map;)V");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        REQUIRE(code.parameters().size()==2);

        const code_typet::parametert &param=code.parameters()[1];
        REQUIRE(is_java_generic_type(param.type()));

        const java_generic_typet &generic_type=
          to_java_generic_type(param.type());
        const java_generic_typet::generic_type_variablest &generic_type_vars=
          generic_type.generic_type_variables();
        REQUIRE(generic_type_vars.size()==2);
        REQUIRE(!is_java_generic_inst_parameter(generic_type_vars[0]));
        REQUIRE(generic_type_vars[0].type_variable().get_identifier()==
          "java::generics$double_element::A");
        REQUIRE(!is_java_generic_inst_parameter(generic_type_vars[1]));
        REQUIRE(generic_type_vars[1].type_variable().get_identifier()==
                "java::generics$double_element::B");

        const symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$double_element::A"),
              symbol_typet("java::java.lang.Integer")},
            { symbol_typet("java::generics$double_element::B"),
              symbol_typet("java::java.lang.Double")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$double_element.setMap:(Ljava/util/Map;)V<java::java.lang.Integer,java::java.lang.Double>")
        );
        const typet &new_symbol_type=specialized_method.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        REQUIRE(new_code.parameters().size()==2);

        const code_typet::parametert &new_param=new_code.parameters()[1];
        REQUIRE(is_java_generic_type(new_param.type()));

        const java_generic_typet &new_generic_type=
          to_java_generic_type(new_param.type());
        const java_generic_typet::generic_type_variablest
          &new_generic_type_vars=new_generic_type.generic_type_variables();
        REQUIRE(new_generic_type_vars.size()==2);
        REQUIRE(is_java_generic_inst_parameter(new_generic_type_vars[0]));
        REQUIRE(new_generic_type_vars[0].subtype()==
                  symbol_typet("java::java.lang.Integer"));
        REQUIRE(is_java_generic_inst_parameter(new_generic_type_vars[1]));
        REQUIRE(new_generic_type_vars[1].subtype()==
                  symbol_typet("java::java.lang.Double"));
      }



      THEN("Specialise a method with a bound compound generic parameter type")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
              "java::generics$compound_element.setFixedElem:(Ljava/util/List;)V")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$compound_element.setFixedElem:(Ljava/util/List;)V");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        REQUIRE(code.parameters().size()==2);

        const code_typet::parametert &param=code.parameters()[1];
        REQUIRE(is_java_generic_type(param.type()));

        const java_generic_typet &generic_type=
          to_java_generic_type(param.type());
        const java_generic_typet::generic_type_variablest &generic_type_vars=
          generic_type.generic_type_variables();
        REQUIRE(generic_type_vars.size()==1);
        REQUIRE(is_java_generic_inst_parameter(generic_type_vars[0]));
        REQUIRE(generic_type_vars[0].subtype()==
                symbol_typet("java::java.lang.Integer"));

        const symbolt &specialized_method=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$compound_element::B"),
              symbol_typet("java::java.lang.Integer")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$compound_element.setFixedElem:(Ljava/util/List;)V<java::java.lang.Integer>")
        );

        // FIXME: Eventually want to replace instantiations of
        // FIXME: generic types (e.g. List<Integer>) with references
        // FIXME: to concrete instantiations of those once PR1406
        // FIXME: is merged. These post-condition tests will need
        // FIXME: updating at that point.

        const typet &new_symbol_type=specialized_method.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        REQUIRE(new_code.parameters().size()==2);

        const code_typet::parametert &new_param=new_code.parameters()[1];
        REQUIRE(is_java_generic_type(new_param.type()));

        const java_generic_typet &new_generic_type=
          to_java_generic_type(new_param.type());
        const java_generic_typet::generic_type_variablest &new_generic_type_vars=
          new_generic_type.generic_type_variables();
        REQUIRE(new_generic_type_vars.size()==1);
        REQUIRE(is_java_generic_inst_parameter(new_generic_type_vars[0]));
        REQUIRE(new_generic_type_vars[0].subtype()==
                symbol_typet("java::java.lang.Integer"));

      }


      THEN("Specialise a method with a parameterized generic return type")
      {
        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$compound_element.getElem:()Ljava/util/List;")
        );

        const symbolt &generic_method_symbol=
          *new_symbol_table.lookup(
            "java::generics$compound_element.getElem:()Ljava/util/List;");

        const typet &symbol_type=generic_method_symbol.type;
        REQUIRE(symbol_type.id()==ID_code);

        const code_typet &code=to_code_type(symbol_type);
        const typet &method_return_type=code.return_type();
        REQUIRE(is_java_generic_type(method_return_type));

        const java_generic_typet &generic_type=
          to_java_generic_type(method_return_type);
        const java_generic_typet::generic_type_variablest &generic_type_vars=
          generic_type.generic_type_variables();
        REQUIRE(generic_type_vars.size()==1);
        REQUIRE(!is_java_generic_inst_parameter(generic_type_vars[0]));
        REQUIRE(generic_type_vars[0].type_variable().get_identifier()==
                "java::generics$compound_element::B");

        const symbolt &specialized_method_symbol=instantiate_generic_method(
          generic_method_symbol,
          {
            { symbol_typet("java::generics$compound_element::B"),
              symbol_typet("java::java.lang.Integer")}
          },
          new_symbol_table);

        REQUIRE(
          new_symbol_table.has_symbol(
            "java::generics$compound_element.getElem:()Ljava/util/List;<java::java.lang.Integer>")
        );
        const typet &new_symbol_type=specialized_method_symbol.type;
        REQUIRE(new_symbol_type.id()==ID_code);

        const code_typet &new_code=to_code_type(new_symbol_type);
        const typet &new_method_return_type=new_code.return_type();
        REQUIRE(is_java_generic_type(new_method_return_type));

        const java_generic_typet &new_generic_type=
          to_java_generic_type(new_method_return_type);
        const java_generic_typet::generic_type_variablest
          &new_generic_type_vars=new_generic_type.generic_type_variables();
        REQUIRE(new_generic_type_vars.size()==1);
        REQUIRE(is_java_generic_inst_parameter(new_generic_type_vars[0]));
        REQUIRE(new_generic_type_vars[0].subtype()==
                symbol_typet("java::java.lang.Integer"));

      }

    }
  }
}
