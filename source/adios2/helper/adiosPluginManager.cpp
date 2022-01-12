/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * adiosPluginManager.cpp
 *
 * Created on: Dec 14, 2021
 *     Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "adios2/common/ADIOSTypes.h"
#include "adios2/core/IO.h"
#include "adios2/engine/plugin/PluginEngineInterface.h"
#include "adios2/helper/adiosComm.h"
#include "adios2/helper/adiosLog.h"
#include "adios2/helper/adiosPluginManager.h"
#include "adios2/helper/adiosString.h"

#include <adios2sys/SystemTools.hxx>
#include <adios2sys/Directory.hxx>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace adios2
{
namespace helper
{

namespace
{
const std::string pluginEnvVarName = "ADIOS2_PLUGIN_PATH";

struct PluginInfo
{
    std::string m_LibraryName;
    std::unique_ptr<DynamicBinder> m_Binder;
    PluginManager::PluginTypes m_Type;
};

struct EnginePluginInfo : public PluginInfo
{
    PluginManager::EngineCreateFun m_HandleCreate;
    PluginManager::EngineDestroyFun m_HandleDestroy;
};

struct OperatorPluginInfo : public PluginInfo
{
    PluginManager::OperatorCreateFun m_HandleCreate;
    PluginManager::OperatorDestroyFun m_HandleDestroy;
};

} // end anon namespace


struct PluginManager::Impl
{
    std::unordered_map<std::string, EnginePluginInfo> m_EngineRegistry;
    std::unordered_map<std::string, OperatorPluginInfo> m_OperatorRegistry;
    adios2::Params m_Parameters;

    std::vector<std::string> m_LibSuffixes;

    Impl()
    {
#ifdef __APPLE__
        m_LibSuffixes.emplace_back(".dylib");
        m_LibSuffixes.emplace_back(".so");
#endif
#ifdef __hpux
        m_LibSuffixes.emplace_back(".sl");
#endif
#ifdef __unix__
        m_LibSuffixes.emplace_back(".so");
#endif
#ifdef _WIN32
        m_LibSuffixes.emplace_back(".dll");
#endif
    }
};

PluginManager* PluginManager::m_Instance = nullptr;
bool PluginManager::m_Destroyed = false;

PluginManager::PluginManager() : m_Impl(new Impl) {}

PluginManager::~PluginManager()
{
    m_Instance = nullptr;
    m_Destroyed = true;
}

PluginManager& PluginManager::GetInstance()
{
    if (!m_Instance)
    {
        if (m_Destroyed)
        {
            throw std::runtime_error("Dead reference to PluginManager singleton");
        }
        else
        {
            CreateInstance();
        }
    }
    return *m_Instance;
}

void PluginManager::CreateInstance()
{
    static PluginManager theInstance;
    m_Instance = &theInstance;
}

void PluginManager::DiscoverPlugins()
{
    static bool pluginsDiscovered = false;
    if (pluginsDiscovered)
    {
        Log("Plugins", "PluginManager", "DiscoverPlugins", "Plugins have already been loaded.",
            LogMode::INFO);
        return;
    }

    // grab plugin env var. Can contain multiple paths, so need to split and check
    // each path for possible plugins
    std::string allPluginPaths;
    adios2sys::SystemTools::GetEnv(pluginEnvVarName, allPluginPaths);
    if (allPluginPaths.empty())
    {
        // ADIOS2_PLUGIN_PATH env var not set, so assume if plugins are to be loaded,
        // user will load manually
        return;
    }
    Log("Plugins", "PluginManager", "DiscoverPlugins", pluginEnvVarName + " set to the following path(s): "
        + allPluginPaths, LogMode::INFO);

    auto pathsSplit = adios2sys::SystemTools::SplitString(allPluginPaths, ':', false);
    // TODO need to test loading plugins from multiple paths
    for (const auto& path : pathsSplit)
    {
        adios2sys::Directory dir;
        if (dir.Load(path))
        {
            for (unsigned long i = 0; i < dir.GetNumberOfFiles(); ++i)
            {
                std::string filename = path + "/" + dir.GetFile(i);
                filename = adios2sys::SystemTools::ConvertToOutputPath(filename);

                // checking for files that end in an appropriate shared lib suffix
                for (const auto& suffix : m_Impl->m_LibSuffixes)
                {
                    if (EndsWith(filename, suffix, false))
                    {
                        auto pluginName = adios2sys::SystemTools::GetFilenameWithoutExtension(filename);
                        if (adios2sys::SystemTools::StringStartsWith(pluginName, "lib"))
                        {
                            pluginName = pluginName.substr(3);
                        }
                        OpenPlugin(pluginName, filename);
                        break;
                    }
                }
            }
        }
    }
    pluginsDiscovered = true;
}

bool PluginManager::OpenPlugin(const std::string& pluginName, const std::string& fullPath)
{
    if (m_Impl->m_EngineRegistry.find(pluginName) != m_Impl->m_EngineRegistry.end() ||
        m_Impl->m_OperatorRegistry.find(pluginName) != m_Impl->m_OperatorRegistry.end())
    {
        // FIXME: what about if a plugin is already loaded in discovery, but user tries
        // to manually load the same plugin but gives it a different name?
        // plugin was already loaded, so nothing to do
        return true;
    }

    Log("Plugins", "PluginManager", "OpenPlugin", "Attempting to open plugin " +
        pluginName + " located at " + fullPath, LogMode::INFO);
    // TODO the extra constructor added to DynamicBinder doesn't seem to be necessary anymore...
    std::unique_ptr<DynamicBinder> binder(new DynamicBinder(fullPath));
    if (auto createHandle = binder->GetSymbol("EngineCreate"))
    {
        // we have an engine plugin
        EnginePluginInfo plugin;
        plugin.m_LibraryName = fullPath;
        plugin.m_Type = PluginManager::PluginTypes::Engine;
        plugin.m_HandleCreate = reinterpret_cast<EngineCreatePtr>(createHandle);
        if (!plugin.m_HandleCreate)
        {
            Log("Plugins", "PluginManager", "OpenPlugin", "Unable to locate EngineCreate"
                " symbol in library " + fullPath, LogMode::EXCEPTION);
        }

        plugin.m_HandleDestroy = reinterpret_cast<EngineDestroyPtr>(
            binder->GetSymbol("EngineDestroy"));
        if (!plugin.m_HandleDestroy)
        {
            Log("Plugins", "PluginManager", "OpenPlugin", "Unable to locate EngineDestroy"
                " symbol in library " + fullPath, LogMode::EXCEPTION);
        }
        plugin.m_Binder = std::move(binder);
        m_Impl->m_EngineRegistry[pluginName] = std::move(plugin);
        Log("Plugins", "PluginManager", "OpenPlugin", "Engine Plugin " +
            pluginName + " successfully opened", LogMode::INFO);
        return true;
    }
    else if (auto createHandle = binder->GetSymbol("OperatorCreate"))
    {
        // should be an operator plugin
        OperatorPluginInfo plugin;
        plugin.m_LibraryName = fullPath;
        plugin.m_Type = PluginManager::PluginTypes::Operator;
        plugin.m_HandleCreate = reinterpret_cast<OperatorCreatePtr>(createHandle);
        if (!plugin.m_HandleCreate)
        {
            Log("Plugins", "PluginManager", "OpenPlugin", "Unable to locate OperatorCreate"
                " symbol in library " + fullPath, LogMode::EXCEPTION);
        }

        plugin.m_HandleDestroy = reinterpret_cast<OperatorDestroyPtr>(
            binder->GetSymbol("OperatorDestroy"));
        if (!plugin.m_HandleDestroy)
        {
            Log("Plugins", "PluginManager", "OpenPlugin", "Unable to locate OperatorDestroy"
                " symbol in library " + fullPath, LogMode::EXCEPTION);
        }
        plugin.m_Binder = std::move(binder);
        m_Impl->m_OperatorRegistry[pluginName] = std::move(plugin);
        Log("Plugins", "PluginManager", "OpenPlugin", "Operator Plugin " +
            pluginName + " successfully opened", LogMode::INFO);
        return true;
    }
    return false;
}

bool PluginManager::LoadPlugin(const Params &params)
{
    // plugin env var not used here, since that plugin would already be found
    // at DiscoverPlugin() time
    auto paramPluginNameIt = params.find("PluginName");
    if (paramPluginNameIt == params.end())
    {
        Log("Plugins", "PluginManager", "LoadPlugin", "PluginName must be specified in the "
            " parameters when manually loading a plugin", LogMode::EXCEPTION);
    }
    std::string pluginName = paramPluginNameIt->second;

    auto paramPluginLibraryIt = params.find("PluginLibrary");
    if (paramPluginLibraryIt == params.end())
    {
        Log("Plugins", "PluginManager", "LoadPlugin", "PluginLibrary must be specified in the "
            " parameters when manually loading a plugin", LogMode::EXCEPTION);
    }
    const std::string &pluginLibrary = paramPluginLibraryIt->second;

    return OpenPlugin(pluginName, pluginLibrary);
}

bool PluginManager::PluginLoaded(const std::string& name, PluginManager::PluginTypes pluginType)
{
    if (pluginType == PluginManager::PluginTypes::Engine)
    {
        auto pluginIt = m_Impl->m_EngineRegistry.find(name);
        if (pluginIt != m_Impl->m_EngineRegistry.end())
        {
            return true;
        }
    }
    else if (pluginType == PluginManager::PluginTypes::Operator)
    {
        auto pluginIt = m_Impl->m_OperatorRegistry.find(name);
        if (pluginIt != m_Impl->m_OperatorRegistry.end())
        {
            return true;
        }
    }
    return false;
}

PluginManager::EngineCreateFun
PluginManager::GetEngineCreateFun(const std::string& name)
{
    auto pluginIt = m_Impl->m_EngineRegistry.find(name);
    if (pluginIt == m_Impl->m_EngineRegistry.end())
    {
        Log("Plugins", "PluginManager", "GetEngineCreateFun", "Couldn't find engine plugin named"
            + name, LogMode::EXCEPTION);
    }

    return pluginIt->second.m_HandleCreate;
}

PluginManager::EngineDestroyFun
PluginManager::GetEngineDestroyFun(const std::string& name)
{
    auto pluginIt = m_Impl->m_EngineRegistry.find(name);
    if (pluginIt == m_Impl->m_EngineRegistry.end())
    {
        Log("Plugins", "PluginManager", "GetEngineDestroyFun", "Couldn't find engine plugin named"
            + name, LogMode::EXCEPTION);
    }

    return pluginIt->second.m_HandleDestroy;
}

PluginManager::OperatorCreateFun
PluginManager::GetOperatorCreateFun(const std::string& name)
{
    auto pluginIt = m_Impl->m_OperatorRegistry.find(name);
    if (pluginIt == m_Impl->m_OperatorRegistry.end())
    {
        Log("Plugins", "PluginManager", "GetOperatorCreateFun", "Couldn't find operator plugin named"
            + name, LogMode::EXCEPTION);
    }

    return pluginIt->second.m_HandleCreate;
}

PluginManager::OperatorDestroyFun
PluginManager::GetOperatorDestroyFun(const std::string& name)
{
    auto pluginIt = m_Impl->m_OperatorRegistry.find(name);
    if (pluginIt == m_Impl->m_OperatorRegistry.end())
    {
        Log("Plugins", "PluginManager", "GetOperatorDestroyFun", "Couldn't find operator plugin named"
            + name, LogMode::EXCEPTION);
    }

    return pluginIt->second.m_HandleDestroy;
}

std::string PluginManager::GetPluginLibraryName(const std::string& name, PluginManager::PluginTypes pluginType)
{
    std::string libName;
    if (pluginType == PluginManager::PluginTypes::Engine)
    {
        auto pluginIt = m_Impl->m_EngineRegistry.find(name);
        if (pluginIt == m_Impl->m_EngineRegistry.end())
        {
            Log("Plugins", "PluginManager", "GetPluginLibraryName", "Couldn't find engine plugin named"
                + name, LogMode::EXCEPTION);
        }
        libName = pluginIt->second.m_LibraryName;
    }
    else if (pluginType == PluginManager::PluginTypes::Operator)
    {
        auto pluginIt = m_Impl->m_OperatorRegistry.find(name);
        if (pluginIt == m_Impl->m_OperatorRegistry.end())
        {
            Log("Plugins", "PluginManager", "GetPluginLibraryName", "Couldn't find operator plugin named"
                + name, LogMode::EXCEPTION);
        }
        libName = pluginIt->second.m_LibraryName;
    }
    else
    {
        Log("Plugins", "PluginManager", "GetPluginLibraryName", "Incorrect plugin type"
            + name, LogMode::EXCEPTION);
    }

    return libName;
}

} // end namespace helper
} // end namespace adios2
