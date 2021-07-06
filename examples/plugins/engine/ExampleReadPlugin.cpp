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
: PluginEngineInterface(io, name, mode, comm.Duplicate())
{
    Init();
    m_Reader.reset(new BP4Reader(io, name, mode, comm.Duplicate()));
}

ExampleReadPlugin::~ExampleReadPlugin() {  }

void ExampleReadPlugin::Init()
{
}

#define declare(T)                                                              \
    void ExampleReadPlugin::DoGetSync(Variable<T> &variable,                 \
                                        T *values)                       \
    {                                                                          \
        m_Reader->Get(variable, values, adios2::Mode::Sync); \
    }                                                                          \
    void ExampleReadPlugin::DoGetDeferred(Variable<T> &variable,                 \
                                            T *values)                       \
    {                                                                          \
        m_Reader->Get(variable, values); \
    }
ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

void ExampleReadPlugin::PerformGets()
{
    m_Reader->PerformGets();
}

void ExampleReadPlugin::DoClose(const int transportIndex)
{
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
