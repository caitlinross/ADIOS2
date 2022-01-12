/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * adiosPluginManager.h
 *
 * Created on: Dec 14, 2021
 *     Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#ifndef ADIOS2_HELPER_PLUGINMANAGER_H
#define ADIOS2_HELPER_PLUGINMANAGER_H

#include "adios2/common/ADIOSTypes.h"
#include "adios2/engine/plugin/PluginEngineInterface.h"
#include "adios2/operator/plugin/PluginOperatorInterface.h"
#include "adios2/helper/adiosDynamicBinder.h"

#include <memory>
#include <utility>

namespace adios2
{
namespace helper
{

class PluginManager
{
public:
    enum class PluginTypes
    {
        Engine,
        Operator,
        Unknown
    };

    using EngineCreatePtr = std::add_pointer<core::engine::PluginEngineInterface *(
        core::IO &, const std::string &, const Mode, helper::Comm)>::type;
    using EngineDestroyPtr =
        std::add_pointer<void(core::engine::PluginEngineInterface *)>::type;
    using EngineCreateFun =
        std::function<typename std::remove_pointer<EngineCreatePtr>::type>;
    using EngineDestroyFun =
        std::function<typename std::remove_pointer<EngineDestroyPtr>::type>;

    using OperatorCreatePtr = std::add_pointer<core::compress::PluginOperatorInterface *(
        const Params &)>::type;
    using OperatorDestroyPtr =
        std::add_pointer<void(core::compress::PluginOperatorInterface *)>::type;
    using OperatorCreateFun =
        std::function<std::remove_pointer<OperatorCreatePtr>::type>;
    using OperatorDestroyFun =
        std::function<std::remove_pointer<OperatorDestroyPtr>::type>;

    static PluginManager& GetInstance();

    /**
     * Attempts to open shared libraries located in the dir(s) indicated in
     * ADIOS2_PLUGIN_PATH. Multiple paths can be listed in ADIOS2_PLUGIN_PATH,
     * delimited by a ':'. Each shared library is opened and checked for the appropriate
     * symbols to determine if it is an engine or operator plugin. Engine Plugins must
     * contain EngineCreate and EngineDestroy symbols, while Operator plugins must contain
     * OperatorCreate and OperatorDestroy symbols. If a library contains none of these,
     * it is closed.
     * For successfully opened libraries, it is given a name based on the library name, but
     * with the extension and leading "lib" removed. e.g., libMyPlugin.so would be called
     * "MyPlugin".
     */
    void DiscoverPlugins();

    /**
     * Attempts to load a single plugin specified by the PluginName and PluginLibrary
     * params. This method does not use ADIOS2_PLUGIN_PATH, so a path should be
     * provided for PluginLibrary, otherwise, the usual dlopen rules apply (refer to
     * dlopen manpages for specific details).
     * Returns true if the plugin was loaded (whether it was through this call
     * or already loaded during DiscoverPlugins()).
     */
    bool LoadPlugin(const Params &params);

    bool PluginLoaded(const std::string& name, PluginTypes pluginType);

    EngineCreateFun GetEngineCreateFun(const std::string& name);
    EngineDestroyFun GetEngineDestroyFun(const std::string& name);

    OperatorCreateFun GetOperatorCreateFun(const std::string& name);
    OperatorDestroyFun GetOperatorDestroyFun(const std::string& name);

    std::string GetPluginLibraryName(const std::string& name, PluginTypes pluginType);

private:
    PluginManager();
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    virtual ~PluginManager();

    static void CreateInstance();

    bool OpenPlugin(const std::string& pluginName, const std::string& fullPath);

    static PluginManager* m_Instance;
    static bool m_Destroyed;

    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // end namespace helper
} // end namespace adios2

#endif /* ADIOS2_HELPER_PLUGINMANAGER_H */
