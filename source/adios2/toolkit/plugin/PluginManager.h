/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginManager.h
 *
 *  Created on: Jun 11, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#ifndef SOURCE_ADIOS2_TOOLKIT_PLUGINMANAGER_H
#define SOURCE_ADIOS2_TOOLKIT_PLUGINMANAGER_H


namespace adios2
{
namespace plugin
{

class PluginManager
{
public:
  PluginManager();

  bool DiscoverPlugins();

private:

};

} // end namespace plugin
} // end namespace adios2

#endif /* SOURCE_ADIOS2_TOOLKIT_PLUGINMANAGER_H */
