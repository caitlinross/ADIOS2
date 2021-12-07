/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleOperator.h
 *
 *  Created on: Dec 7, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#ifndef EXAMPLEOPERATOR_H_
#define EXAMPLEOPERATOR_H_

#include "adios2/common/ADIOSTypes.h"
#include "adios2/operator/plugin/PluginOperatorInterface.h"

namespace adios2
{
namespace core
{
namespace compress
{

/** An operator interface to be used by the plugin infrastructure */
class ExampleOperator : public PluginOperatorInterface
{
public:
    ExampleOperator(const Params &parameters);
    virtual ~ExampleOperator();


    size_t Operate(const char *dataIn, const Dims &blockStart,
                   const Dims &blockCount, const DataType type,
                   char *bufferOut) override;

    size_t InverseOperate(const char *bufferIn, const size_t sizeIn,
                          char *dataOut) override;

    bool IsDataTypeValid(const DataType type) const override;
};

} // end namespace compress
} // end namespace core
} // end namespace adios2

extern "C" {

adios2::core::compress::ExampleOperator *
OperatorCreate(const adios2::Params &parameters);
void OperatorDestroy(adios2::core::compress::ExampleOperator *obj);
}

#endif /* EXAMPLEWRITEPLUGIN_H_ */
