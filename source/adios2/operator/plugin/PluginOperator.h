/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * PluginOperator.h Support for an operator implemented outside libadios2
 *
 *  Created on: July 17, 2017
 *      Author: Chuck Atkins <chuck.atkins@kitware.com>
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
#include "adios2/core/Variable.h"
#include "adios2/core/VariableCompound.h"
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
    // Function pointers used for the plugin factory methods

    using OperatorCreatePtr = std::add_pointer<PluginOperatorInterface *(
        IO &, const std::string &, const Mode, helper::Comm)>::type;
    using OperatorDestroyPtr =
        std::add_pointer<void(PluginOperatorInterface *)>::type;
    using OperatorCreateFun =
        std::function<std::remove_pointer<OperatorCreatePtr>::type>;
    using OperatorDestroyFun =
        std::function<std::remove_pointer<OperatorDestroyPtr>::type>;

    static void RegisterPlugin(const std::string pluginName,
                               OperatorCreateFun create,
                               OperatorDestroyFun destroy);
    static void RegisterPlugin(const std::string pluginName,
                               OperatorCreatePtr create, OperatorDestroyPtr destroy)
    {
        RegisterPlugin(pluginName, OperatorCreateFun(create),
                       OperatorDestroyFun(destroy));
    }

    // This is just a shortcut method to handle the case where the class type is
    // directly available to the caller so a simple new and delete call is
    // sufficient to create and destroy the operator object
    template <typename T>
    static void RegisterPlugin(const std::string name);

public:
    PluginOperator(IO &io, const std::string &name, const Mode mode,
                 helper::Comm comm);
    virtual ~PluginOperator();

    StepStatus BeginStep(StepMode mode,
                         const float timeoutSeconds = 0.f) override;
    void PerformPuts() override;
    void PerformGets() override;
    void EndStep() override;

protected:
    void Init() override;

#define declare(T)                                                             \
    void DoPutSync(Variable<T> &, const T *) override;                         \
    void DoPutDeferred(Variable<T> &, const T *) override;                     \
    void DoGetSync(Variable<T> &, T *) override;                               \
    void DoGetDeferred(Variable<T> &, T *) override;

    ADIOS2_FOREACH_STDTYPE_1ARG(declare)
#undef declare

    void DoClose(const int transportIndex = -1) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // end namespace compress
} // end namespace core
} // end namespace adios2

#include "PluginOperator.inl"

#endif /* ADIOS2_OPERATOR_PLUGIN_PLUGINOPERATOR_H_ */
