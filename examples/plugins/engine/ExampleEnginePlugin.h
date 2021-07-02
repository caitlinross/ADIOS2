/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleEnginePlugin.h This plugin does nothing but write API calls out to a
 * log file.
 *
 *  Created on: Jul 31, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
 */

#ifndef EXAMPLEENGINEPLUGIN_H_
#define EXAMPLEENGINEPLUGIN_H_

#include <fstream>
#include <string>

#include <adios2/common/ADIOSMacros.h>
#include <adios2/common/ADIOSTypes.h>
#include <adios2/core/IO.h>
#include <adios2/engine/plugin/PluginEngineInterface.h>
#include "adios2/helper/adiosComm.h"

namespace adios2
{
namespace core
{
namespace engine
{

/** An engine interface to be used aby the plugin infrastructure */
class ExampleEnginePlugin : public PluginEngineInterface
{
public:
    ExampleEnginePlugin(IO &io, const std::string &name, const Mode openMode,
                        helper::Comm comm);
    virtual ~ExampleEnginePlugin();

protected:
    void Init() override;

#define declare(T)                                                             \
    void DoPutSync(Variable<T> &variable, const T *values) override;           \
    void DoPutDeferred(Variable<T> &variable, const T *values) override;
    ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

    void DoClose(const int transportIndex = -1) override;

private:
    std::ofstream m_Log;
};

} // end namespace engine
} // end namespace core
} // end namespace adios2
#endif /* EXAMPLEENGINEPLUGIN_H_ */
