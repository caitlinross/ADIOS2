/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperatorInterface.h Operators using the plugin interface should derive from
 * this class.
 *
 *  Created on: July 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#ifndef ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATORINTERFACE_H_
#define ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATORINTERFACE_H_

/// \cond EXCLUDE_FROM_DOXYGEN
/// \endcond

#include "adios2/common/ADIOSConfig.h"
#include "adios2/common/ADIOSTypes.h"
#include "adios2/core/Operator.h"
#include "adios2/core/IO.h"
#include "adios2/helper/adiosComm.h"

namespace adios2
{
namespace core
{
namespace compress
{

/** An operator interface to be used by the plugin infrastructure */
class PluginOperatorInterface : public Operator
{
    // Give the plugin operator access to everything
    friend class PluginOperator;

public:
    PluginOperatorInterface(IO &io, const std::string &name, const Mode mode,
                          helper::Comm comm);
    virtual ~PluginOperatorInterface() = default;
};

} // end namespace compress
} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATORINTERFACE_H_ */
