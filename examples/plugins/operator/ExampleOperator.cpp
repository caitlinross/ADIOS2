/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ExampleOperator.cpp
 *
 *  Created on: Dec 7, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include "ExampleOperator.h"

#include "adios2/helper/adiosSystem.h"

namespace adios2
{
namespace core
{
namespace compress
{

ExampleOperator::ExampleOperator(const Params &parameters)
: PluginOperatorInterface(parameters)
{
}

ExampleOperator::~ExampleOperator()
{
}

size_t ExampleOperator::Operate(const char *dataIn, const Dims &blockStart,
               const Dims &blockCount, const DataType type,
               char *bufferOut)
{
    //const uint8_t bufferVersion = 1;
    size_t bufferOutOffset = 0;
    //MakeCommonHeader(bufferOut, bufferOutOffset, bufferVersion);

    size_t sizeIn = helper::GetTotalSize(blockCount, helper::GetDataTypeSize(type));
    PutParameter(bufferOut, bufferOutOffset, sizeIn);
    std::memcpy(bufferOut + bufferOutOffset, dataIn, sizeIn);
    bufferOutOffset += sizeIn;
    return bufferOutOffset;
}

size_t ExampleOperator::InverseOperate(const char *bufferIn, const size_t sizeIn,
                      char *dataOut)
{
    size_t bufferInOffset = 0;
    //const uint8_t bufferVersion = GetParameter<uint8_t>(bufferIn, bufferInOffset);
    //bufferInOffset += 2; // skip 2 reserved bytes

    //if (bufferVersion != 1)
    //{
    //    throw("unknown example operator buffer version");
    //}

    const size_t totalBytes = GetParameter<size_t>(bufferIn, bufferInOffset);
    std::cout << "totalBytes = " << totalBytes << std::endl;
    std::memcpy(dataOut, bufferIn + bufferInOffset, totalBytes);
    return totalBytes;
}

bool ExampleOperator::IsDataTypeValid(const DataType type) const
{
    return true;
}

} // end namespace compress
} // end namespace core
} // end namespace adios2

extern "C" {

adios2::core::compress::ExampleOperator *
OperatorCreate(const adios2::Params &parameters)
{
    return new adios2::core::compress::ExampleOperator(parameters);
}

void OperatorDestroy(adios2::core::compress::ExampleOperator *obj)
{
    delete obj;
}
}
