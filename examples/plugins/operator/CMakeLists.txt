#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

add_library(OperatorPlugin
  ExampleOperator.cpp
)
target_link_libraries(OperatorPlugin adios2::cxx11 adios2_core)

add_executable(exampleOperatorPlugin
  examplePluginOperator.cpp
)
target_link_libraries(exampleOperatorPlugin adios2::cxx11)

add_executable(exampleOperatorPlugin_read
  examplePluginOperator_read.cpp
)
target_link_libraries(exampleOperatorPlugin_read adios2::cxx11)
