cmake_minimum_required(VERSION 3.2)

project(CPROVER)

#include(src/config.cmake)

add_subdirectory(src)
#add_subdirectory(regression)
#add_subdirectory(unit)

#add_subdirectory(jbmc)

#set_target_properties(
#    driver
#    testing-utils
#    unit

#    java_bytecode
#    java-models-library
#    jbmc
#    jbmc-lib
#    janalyzer
#    janalyzer-lib
#    jdiff
#    jdiff-lib
#    java-testing-utils
#    java-unit
#    miniz

#    PROPERTIES
#    CXX_STANDARD 11
#    CXX_STANDARD_REQUIRED true
#    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application: Daniel Kroening"
#)
