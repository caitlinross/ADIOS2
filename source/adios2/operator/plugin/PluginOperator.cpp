/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperator.cpp
 *
 *  Created on: Dec 7, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "PluginOperator.h"
#include "PluginOperatorInterface.h"

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "adios2/helper/adiosDynamicBinder.h"
#include "adios2/helper/adiosPluginManager.h"

#include <adios2sys/SystemTools.hxx>

namespace adios2
{
namespace core
{
namespace compress
{

/******************************************************************************/

struct PluginOperator::Impl
{
    // TODO figure out how to do things better...this is confusing because there's already
    // a m_Parameters inherited from Operator
    Params m_Parameters;
    helper::PluginManager::OperatorCreateFun m_HandleCreate;
    helper::PluginManager::OperatorDestroyFun m_HandleDestroy;
    PluginOperatorInterface *m_Plugin = nullptr;
};

/******************************************************************************/

PluginOperator::PluginOperator(const Params &parameters)
  : Operator("plugin", PLUGIN_INTERFACE, parameters)
  , m_Impl(new Impl)
{
    Init();
}

PluginOperator::~PluginOperator() { m_Impl->m_HandleDestroy(m_Impl->m_Plugin); }

void PluginOperator::Init()
{
    if (m_Impl->m_Plugin)
    {
        return;
    }

    auto& pluginManager = helper::PluginManager::GetInstance();

    auto pluginNameIt = m_Parameters.find("pluginname");
    if (pluginNameIt != m_Parameters.end())
    {
        m_Impl->m_HandleCreate = pluginManager.GetOperatorCreateFun(pluginNameIt->second);
        m_Impl->m_HandleDestroy = pluginManager.GetOperatorDestroyFun(pluginNameIt->second);
        m_Impl->m_Parameters["PluginName"] = pluginNameIt->second;
        std::cout << "PluginOperator() PluginName " << pluginNameIt->second << std::endl;
        // TODO ...do I actually need pluginlibrary?
        // So user could on read side set the adios object parameters and have the plugins
        // loaded, but in a case like bpls, that won't happen, so we need the plugin library
        m_Impl->m_Parameters["PluginLibrary"] = pluginManager.GetPluginLibraryName(
            pluginNameIt->second, helper::PluginManager::PluginTypes::Operator);
        std::cout << "PluginOperator() PluginLibrary " << m_Impl->m_Parameters["PluginLibrary"] << std::endl;
        m_Impl->m_Plugin =
            m_Impl->m_HandleCreate(m_Parameters);
    }
    //else
    //{
    //    // in this case, we should be trying to create the operator to do an inverse operate
    //    // we can get the parameters
    //}

}

size_t PluginOperator::Operate(const char *dataIn, const Dims &blockStart,
               const Dims &blockCount, const DataType type,
               char *bufferOut)
{
    // So we can perform InverseOperate appropriately we need to save plugin info
    // For right now, we'll save this metadata on each Operate, but it's always the same
    // for a given operator plugin and it's two strings...Is there a way we can store this once
    // somewhere?
    // handle common header first
    size_t offset = 0;
    const uint8_t bufferVersion = 1;
    MakeCommonHeader(bufferOut, offset, bufferVersion);

    // now we can do plugin specific header
    PutParameters(bufferOut, offset, m_Impl->m_Parameters);
    // add offset to the bufferOut pointer, so that way the plugin doesn't need to know anything
    // about the plugin header.
    size_t pluginSize =
        m_Impl->m_Plugin->Operate(dataIn, blockStart, blockCount, type, bufferOut + offset);
    return offset + pluginSize;
}

size_t PluginOperator::InverseOperate(const char *bufferIn, const size_t sizeIn,
                      char *dataOut)
{
    size_t offset = 1; // skip operator type
    const uint8_t bufferVersion = GetParameter<uint8_t>(bufferIn, offset);
    offset += 2; // skip 2 reserved bytes

    // now handle plugin specific header
    m_Impl->m_Parameters = GetParameters(bufferIn, offset);
    // now set up the plugin if it hasn't already
    auto& pluginManager = helper::PluginManager::GetInstance();
    pluginManager.LoadPlugin(m_Impl->m_Parameters);

    auto pluginNameIt = m_Impl->m_Parameters.find("PluginName");
    if (pluginNameIt != m_Impl->m_Parameters.end())
    {
        m_Impl->m_HandleCreate = pluginManager.GetOperatorCreateFun(pluginNameIt->second);
        m_Impl->m_HandleDestroy = pluginManager.GetOperatorDestroyFun(pluginNameIt->second);
        m_Impl->m_Plugin =
            m_Impl->m_HandleCreate(m_Parameters);
    }
    // add offset to bufferIn, so plugin doesn't have to worry about plugin header
    size_t pluginSize = m_Impl->m_Plugin->InverseOperate(bufferIn + offset, sizeIn, dataOut);
    return offset + pluginSize;
}

bool PluginOperator::IsDataTypeValid(const DataType type) const
{
    return m_Impl->m_Plugin->IsDataTypeValid(type);
}

} // end namespace compress
} // end namespace core
} // end namespace adios2
