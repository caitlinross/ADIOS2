/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperator.h Support for an operator implemented outside libadios2
 *
 *  Created on: July 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#ifndef ADIOS2_OPERATOR_PLUGIN_OPERATOR_INL_
#define ADIOS2_OPERATOR_PLUGIN_OPERATOR_INL_
#ifndef ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATOR_H_
#error "Inline file should only be included from it's header, never on it's own"
#endif

#include "PluginOperator.h"

namespace adios2
{
namespace core
{
namespace compress
{

template <typename T>
void PluginOperator::RegisterPlugin(const std::string name)
{
    OperatorCreateFun createFun =
        [](const std::string &type, const Params &parameters) -> PluginOperatorInterface * {
          return new T(type, parameters);
    };
    OperatorDestroyFun destroyFun = [](Operator *obj) -> void { delete obj; };

    RegisterPlugin(name, createFun, destroyFun);
}

} // end namespace compress
} // end namespace core
} // end namespace adios2

#endif // ADIOS2_OPERATOR_PLUGIN_OPERATOR_INL_
