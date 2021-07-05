add_executable(examplePluginEngine_class
  examplePluginEngine_class.cpp
  ExampleEnginePlugin.h ExampleEnginePlugin.cpp
)
target_link_libraries(examplePluginEngine_class adios2::cxx11 adios2_core)
