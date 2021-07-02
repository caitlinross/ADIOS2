/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginEngineInterface.h Engines using the plugin interface should derive from
 * this class.
 *
 *  Created on: July 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#ifndef ADIOS2_ENGINE_PLUGIN_PLUGINENGINEINTERFACE_H_
#define ADIOS2_ENGINE_PLUGIN_PLUGINENGINEINTERFACE_H_

/// \cond EXCLUDE_FROM_DOXYGEN
/// \endcond

#include "adios2/common/ADIOSConfig.h"
#include "adios2/common/ADIOSTypes.h"
#include "adios2/core/Engine.h"
#include "adios2/core/IO.h"
#include "adios2/helper/adiosComm.h"

namespace adios2
{
namespace core
{
namespace engine
{

/** An engine interface to be used by the plugin infrastructure */
class PluginEngineInterface : public Engine
{
    // Give the plugin engine access to everything
    friend class PluginEngine;

public:
    PluginEngineInterface(IO &io, const std::string &name, const Mode mode,
                          helper::Comm comm);
    virtual ~PluginEngineInterface() = default;
};

} // end namespace engine
} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_ENGINE_PLUGIN_PLUGINENGINEINTERFACE_H_ */
