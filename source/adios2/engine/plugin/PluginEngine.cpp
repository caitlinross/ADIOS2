/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginEngine.cpp
 *
 *  Created on: July 5, 2021
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 *              Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "PluginEngine.h"
#include "PluginEngineInterface.h"

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "adios2/helper/adiosLog.h"
#include "adios2/helper/adiosPluginManager.h"

namespace adios2
{
namespace core
{
namespace engine
{

/******************************************************************************/

struct PluginEngine::Impl
{
    helper::PluginManager::EngineCreateFun m_HandleCreate;
    helper::PluginManager::EngineDestroyFun m_HandleDestroy;
    PluginEngineInterface *m_Plugin = nullptr;
};

/******************************************************************************/

PluginEngine::PluginEngine(IO &io, const std::string &name, const Mode mode,
                           helper::Comm comm)
: Engine("Plugin", io, name, mode, comm.Duplicate()), m_Impl(new Impl)
{
    auto pluginNameIt = m_IO.m_Parameters.find("PluginEngineName");
    if (pluginNameIt == m_IO.m_Parameters.end())
    {
        helper::Log("Plugins", "PluginEngine", "PluginEngine", "PluginEngineName was not"
            " correctly set by the IO object. Did you call IO::SetEngine(engineType, pluginName)"
            " with engineType set to 'plugin'?", helper::LogMode::EXCEPTION);
    }

    auto& pluginManager = helper::PluginManager::GetInstance();
    if (!pluginManager.PluginLoaded(pluginNameIt->second, helper::PluginManager::PluginTypes::Engine))
    {
        helper::Log("Plugins", "PluginEngine", "PluginEngine", "The engine plugin named " +
            pluginNameIt->second + " does not exist in the plugin registry. Check that you "
            " loaded the plugin and that you called IO::SetEngine(engineType, pluginName)"
            " with engineType set to 'plugin' and pluginName set to '" + pluginNameIt->second +
            "'", helper::LogMode::EXCEPTION);
    }
    m_Impl->m_HandleCreate = pluginManager.GetEngineCreateFun(pluginNameIt->second);
    m_Impl->m_HandleDestroy = pluginManager.GetEngineDestroyFun(pluginNameIt->second);
    m_Impl->m_Plugin = m_Impl->m_HandleCreate(io, pluginNameIt->second, mode,
                                              comm.Duplicate());
}

PluginEngine::~PluginEngine() { m_Impl->m_HandleDestroy(m_Impl->m_Plugin); }

StepStatus PluginEngine::BeginStep(StepMode mode, const float timeoutSeconds)
{
    return m_Impl->m_Plugin->BeginStep(mode, timeoutSeconds);
}

void PluginEngine::PerformPuts() { m_Impl->m_Plugin->PerformPuts(); }

void PluginEngine::PerformGets() { m_Impl->m_Plugin->PerformGets(); }

void PluginEngine::EndStep() { m_Impl->m_Plugin->EndStep(); }

// TODO do we need Init? and if so, should it call the one in the plugin?
void PluginEngine::Init()
{
}

#define declare(T)                                                             \
    void PluginEngine::DoPutSync(Variable<T> &variable, const T *values)       \
    {                                                                          \
        m_Impl->m_Plugin->DoPutSync(variable, values);                         \
    }                                                                          \
    void PluginEngine::DoPutDeferred(Variable<T> &variable, const T *values)   \
    {                                                                          \
        m_Impl->m_Plugin->DoPutDeferred(variable, values);                     \
    }                                                                          \
    void PluginEngine::DoGetSync(Variable<T> &variable, T *values)             \
    {                                                                          \
        m_Impl->m_Plugin->DoGetSync(variable, values);                         \
    }                                                                          \
    void PluginEngine::DoGetDeferred(Variable<T> &variable, T *values)         \
    {                                                                          \
        m_Impl->m_Plugin->DoGetDeferred(variable, values);                     \
    }

ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

void PluginEngine::DoClose(const int transportIndex)
{
    m_Impl->m_Plugin->Close(transportIndex);
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
