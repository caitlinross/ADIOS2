/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperator.cpp
 *
 *  Created on: July 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#include "PluginOperator.h"
#include "PluginOperatorInterface.h"

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "adios2/helper/adiosDynamicBinder.h"

namespace adios2
{
namespace core
{
namespace compress
{

/******************************************************************************/

struct PluginOperator::Impl
{
    using Registry =
        std::map<std::string, std::pair<OperatorCreateFun, OperatorDestroyFun>>;
    static Registry m_Registry;

    std::string m_PluginName;
    std::unique_ptr<helper::DynamicBinder> m_Binder;
    OperatorCreateFun m_HandleCreate;
    OperatorDestroyFun m_HandleDestroy;
    PluginOperatorInterface *m_Plugin = nullptr;
};
PluginOperator::Impl::Registry PluginOperator::Impl::m_Registry;

/******************************************************************************/

void PluginOperator::RegisterPlugin(const std::string pluginName,
                                  OperatorCreateFun create,
                                  OperatorDestroyFun destroy)
{
    PluginOperator::Impl::m_Registry.emplace(pluginName,
                                           std::make_pair(create, destroy));
}

/******************************************************************************/

PluginOperator::PluginOperator(const std::string type, const Params &parameters)
: Operator(type, parameters), m_Impl(new Impl)
{
    Init();
    m_Impl->m_Plugin =
        m_Impl->m_HandleCreate(type, parameters);
}

PluginOperator::~PluginOperator() { m_Impl->m_HandleDestroy(m_Impl->m_Plugin); }

void PluginOperator::Init()
{
    auto paramPluginNameIt = m_IO.m_Parameters.find("PluginName");
    if (paramPluginNameIt == m_IO.m_Parameters.end())
    {
        throw std::invalid_argument("PluginOperator: PluginName must be "
                                    "specified in operator parameters");
    }
    m_Impl->m_PluginName = paramPluginNameIt->second;

    // First we check to see if we can find the plugin currently registerd
    auto registryEntryIt =
        PluginOperator::Impl::m_Registry.find(m_Impl->m_PluginName);

    if (registryEntryIt != PluginOperator::Impl::m_Registry.end())
    {
        m_Impl->m_HandleCreate = registryEntryIt->second.first;
        m_Impl->m_HandleDestroy = registryEntryIt->second.second;
    }
    else
    {
        // It's not currently registered so try to load it from a shared
        // library
        //
        auto paramPluginLibraryIt = m_IO.m_Parameters.find("PluginLibrary");
        if (paramPluginLibraryIt == m_IO.m_Parameters.end())
        {
            throw std::invalid_argument(
                "PluginOperator: PluginLibrary must be specified in "
                "operator parameters if no PluginName "
                "is specified");
        }
        std::string &pluginLibrary = paramPluginLibraryIt->second;

        m_Impl->m_Binder.reset(new helper::DynamicBinder(pluginLibrary));

        m_Impl->m_HandleCreate = reinterpret_cast<OperatorCreatePtr>(
            m_Impl->m_Binder->GetSymbol("OperatorCreate"));
        if (!m_Impl->m_HandleCreate)
        {
            throw std::runtime_error("PluginOperator: Unable to locate "
                                     "OperatorCreate symbol in specified plugin "
                                     "library");
        }

        m_Impl->m_HandleDestroy = reinterpret_cast<OperatorDestroyPtr>(
            m_Impl->m_Binder->GetSymbol("OperatorDestroy"));
        if (!m_Impl->m_HandleDestroy)
        {
            throw std::runtime_error("PluginOperator: Unable to locate "
                                     "OperatorDestroy symbol in specified plugin "
                                     "library");
        }
    }
}

} // end namespace compress
} // end namespace core
} // end namespace adios2
