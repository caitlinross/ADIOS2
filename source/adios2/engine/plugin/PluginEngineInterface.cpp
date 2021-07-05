/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginEngineInterface.cxx
 *
 *  Created on: Aug 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#include "PluginEngineInterface.h"

namespace adios2
{
namespace core
{
namespace engine
{

PluginEngineInterface::PluginEngineInterface(IO &io, const std::string &name,
                                             const Mode mode, helper::Comm comm)
: Engine("PluginInterface", io, name, mode, std::move(comm))
{
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
