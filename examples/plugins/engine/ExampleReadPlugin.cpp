/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleReadPlugin.cpp This plugin does nothing but write API calls out to a
 * log file.
 *
 *  Created on: Jul 5, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "ExampleReadPlugin.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#ifndef _WIN32
#include <sys/time.h>
#endif

namespace adios2
{
namespace core
{

namespace engine
{

ExampleReadPlugin::ExampleReadPlugin(IO &io, const std::string &name,
                                         const Mode mode, helper::Comm comm)
: PluginEngineInterface(io, name, mode, std::move(comm))
{
    Init();
}

ExampleReadPlugin::~ExampleReadPlugin() { m_Data.close(); }

void ExampleReadPlugin::Init()
{
    std::string fileName = "ExamplePlugin.bin";
    auto paramFileNameIt = m_IO.m_Parameters.find("FileName");
    if (paramFileNameIt != m_IO.m_Parameters.end())
    {
        fileName = paramFileNameIt->second;
    }

    m_Data.open(fileName);
    if (!m_Data)
    {
        throw std::ios_base::failure(
            "ExampleReadPlugin: Failed to open file " + fileName);
    }
}

#define declare(T)                                                              \
    void ExampleReadPlugin::DoGetSync(Variable<T> &variable,                 \
                                        T *values)                       \
    {                                                                          \
    }                                                                          \
    void ExampleReadPlugin::DoGetDeferred(Variable<T> &variable,                 \
                                            T *values)                       \
    {                                                                          \
    }
ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

void ExampleReadPlugin::DoClose(const int transportIndex)
{
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
