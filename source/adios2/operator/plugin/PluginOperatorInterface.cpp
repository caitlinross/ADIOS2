/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperatorInterface.cxx
 *
 *  Created on: Dec 7, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "PluginOperatorInterface.h"

namespace adios2
{
namespace core
{
namespace compress
{

PluginOperatorInterface::PluginOperatorInterface(const Params &parameters)
: Operator("PluginInterface", PLUGIN_INTERFACE, parameters)
{
}

} // end namespace compress
} // end namespace core
} // end namespace adios2
