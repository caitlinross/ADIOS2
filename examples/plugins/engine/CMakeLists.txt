add_executable(examplePluginEngine_write
  examplePluginEngine_write.cpp
  ExampleWritePlugin.h ExampleWritePlugin.cpp
)
target_link_libraries(examplePluginEngine_write adios2::cxx11 adios2_core)
