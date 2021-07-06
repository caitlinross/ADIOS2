/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleWritePlugin.cpp This plugin does nothing but write API calls out to a
 * log file.
 *
 *  Created on: Jul 31, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#include "ExampleWritePlugin.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#ifndef _WIN32
#include <sys/time.h>
#endif

namespace
{
std::string now()
{
    tm *timeInfo;
#ifdef _WIN32
    time_t rawTime;
    std::time(&rawTime);
    timeInfo = std::localtime(&rawTime);
#else
    timeval curTime;
    gettimeofday(&curTime, nullptr);
    timeInfo = std::localtime(&curTime.tv_sec);
#endif

    char timeBuf[80];
    std::memset(timeBuf, 0, sizeof(timeBuf));
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%S", timeInfo);

#ifdef _WIN32
    return std::string(timeBuf);
#else
    double subSec = curTime.tv_usec / 1e6;
    char subSecBuf[9];
    std::memset(subSecBuf, 0, sizeof(subSecBuf));
    std::sprintf(subSecBuf, "%1.6f", subSec);
    return std::string(timeBuf) + std::string(&subSecBuf[1]);
#endif
}
}

namespace adios2
{
namespace core
{

namespace engine
{

ExampleWritePlugin::ExampleWritePlugin(IO &io, const std::string &name,
                                         const Mode mode, helper::Comm comm)
: PluginEngineInterface(io, name, mode, comm.Duplicate())
{
    Init();
    m_Writer.reset(new BP4Writer(io, name, mode, comm.Duplicate()));
}

ExampleWritePlugin::~ExampleWritePlugin()
{
    m_Log.close();
}

void ExampleWritePlugin::Init()
{
    std::string logName = "ExamplePlugin.log";
    auto paramLogNameIt = m_IO.m_Parameters.find("LogName");
    if (paramLogNameIt != m_IO.m_Parameters.end())
    {
        logName = paramLogNameIt->second;
    }

    m_Log.open(logName);
    if (!m_Log)
    {
        throw std::ios_base::failure(
            "ExampleWritePlugin: Failed to open log file " + logName);
    }
    m_Log << now() << " Init" << std::endl;
}

#define declare(T)                                                              \
    void ExampleWritePlugin::DoPutSync(Variable<T> &variable,                 \
                                        const T *values)                       \
    {                                                                          \
        m_Log << now() << " Writing variable \"" << variable.m_Name << "\""    \
              << std::endl;                                                    \
        m_Writer->Put(variable, values, adios2::Mode::Sync); \
        m_Log << now() << " Wrote data of size " << variable.SelectionSize()  \
              << std::endl; \
    }                                                                          \
    void ExampleWritePlugin::DoPutDeferred(Variable<T> &variable,                 \
                                        const T *values)                       \
    {                                                                          \
        m_Log << now() << " Writing variable \"" << variable.m_Name << "\""    \
              << std::endl;                                                    \
        m_Writer->Put(variable, values); \
        m_Log << now() << " Wrote data of size " << variable.SelectionSize()  \
              << std::endl; \
    }
ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

void ExampleWritePlugin::PerformPuts()
{
    m_Writer->PerformPuts();
}

void ExampleWritePlugin::DoClose(const int transportIndex)
{
    m_Log << now() << " Close with transportIndex " << transportIndex
          << std::endl;
    m_Writer->Flush(transportIndex);
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
