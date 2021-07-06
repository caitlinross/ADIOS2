/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleReadPlugin.h This plugin does nothing but write API calls out to a
 * log file.
 *
 *  Created on: Jul 5, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#ifndef EXAMPLEREADPLUGIN_H_
#define EXAMPLEREADPLUGIN_H_

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

/** An engine interface to be used by the plugin infrastructure */
class ExampleReadPlugin : public PluginEngineInterface
{
public:
    ExampleReadPlugin(IO &io, const std::string &name, const Mode openMode,
                        helper::Comm comm);
    virtual ~ExampleReadPlugin();

protected:
    void Init() override;

#define declare(T)                                                             \
    void DoGetSync(Variable<T> &variable, T *values) override;           \
    void DoGetDeferred(Variable<T> &variable, T *values) override;
    ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

    void DoClose(const int transportIndex = -1) override;

private:
    std::ifstream m_Data;
};

} // end namespace engine
} // end namespace core
} // end namespace adios2
#endif /* EXAMPLEREADPLUGIN_H_ */
