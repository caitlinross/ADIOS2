/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperatorInterface.cxx
 *
 *  Created on: Aug 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#include "PluginOperatorInterface.h"

namespace adios2
{
namespace core
{
namespace compress
{

PluginOperatorInterface::PluginOperatorInterface(IO &io, const std::string &name,
                                             const Mode mode, helper::Comm comm)
: Operator("PluginInterface", io, name, mode, std::move(comm))
{
}

} // end namespace compress
} // end namespace core
} // end namespace adios2
