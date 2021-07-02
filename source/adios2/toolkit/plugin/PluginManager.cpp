/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginManager.cpp
 *
 *  Created on: Jun 11, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "adios2/toolkit/plugin/PluginManager.h"

#include <cstdlib>
#include <string>
#include <iostream>

namespace adios2
{
namespace plugin
{
namespace
{

// TODO could have multiple paths in variable...separate out into
// a vector of strings instead?
std::string getPluginPath()
{
  char* pluginPath = getenv("ADIOS2_PLUGIN_PATH");
  if (!pluginPath)
  {
    // Look somewhere else?
    std::cout << "ADIOS2_PLUGIN_PATH not set" << std::endl;
    return "";
  }
  return std::string(pluginPath);
}

} // end anon namespace

PluginManager::PluginManager() = default;

bool PluginManager::DiscoverPlugins()
{
  auto pluginPath = getPluginPath();

  return false;
}

} // end namespace plugin
} // end namespace adios2
