/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperator.h Support for an operator implemented outside libadios2
 *
 *  Created on: Dec 7, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#ifndef ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATOR_H_
#define ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATOR_H_

#include "PluginOperatorInterface.h"

#include <functional>  // for function
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <type_traits> // for add_pointer
#include <vector>      // for vector

#include "adios2/common/ADIOSMacros.h"
#include "adios2/common/ADIOSTypes.h"
#include "adios2/core/Operator.h"
#include "adios2/core/IO.h"
#include "adios2/helper/adiosComm.h"

namespace adios2
{
namespace core
{
namespace compress
{

/** A front-end wrapper for an operator implemented outside of libadios2 */
class PluginOperator : public Operator
{
public:
    PluginOperator(const Params &parameters);
    virtual ~PluginOperator();

    size_t Operate(const char *dataIn, const Dims &blockStart,
                   const Dims &blockCount, const DataType type,
                   char *bufferOut) override;

    size_t InverseOperate(const char *bufferIn, const size_t sizeIn,
                          char *dataOut) override;

    bool IsDataTypeValid(const DataType type) const override;

protected:
    void Init();

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // end namespace compress
} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATOR_H_ */
