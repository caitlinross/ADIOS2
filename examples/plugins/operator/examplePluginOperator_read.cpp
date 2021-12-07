/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * examplePluginEngine_write.cpp example showing how to use ExampleWritePlugin
 * engine
 *
 *  Created on: July 5, 2021
 *      Author: Caitlin Ross <caitlin.ross@kitware.com>
 */

#include <ios>       //std::ios_base::failure
#include <iostream>  //std::cout
#include <stdexcept> //std::invalid_argument std::exception
#include <vector>

#include "adios2.h"

int main(int argc, char *argv[])
{
    /** Application variable */
    std::vector<float> myFloats = {
        0.0001, 1.0001, 2.0001, 3.0001, 4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001,
        1.0001, 2.0001, 3.0001, 4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001, 8.0001,
        2.0001, 3.0001, 4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001, 8.0001, 7.0001,
        3.0001, 4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001, 8.0001, 7.0001, 6.0001,
        4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001, 8.0001, 7.0001, 6.0001, 5.0001,
        5.0001, 6.0001, 7.0001, 8.0001, 9.0001, 8.0001, 7.0001, 6.0001, 5.0001, 4.0001,
        6.0001, 7.0001, 8.0001, 9.0001, 8.0001, 7.0001, 6.0001, 5.0001, 4.0001, 3.0001,
        7.0001, 8.0001, 9.0001, 8.0001, 7.0001, 6.0001, 5.0001, 4.0001, 3.0001, 2.0001,
        8.0001, 9.0001, 8.0001, 7.0001, 6.0001, 5.0001, 4.0001, 3.0001, 2.0001, 1.0001,
        9.0001, 8.0001, 7.0001, 6.0001, 5.0001, 4.0001, 3.0001, 2.0001, 1.0001, 0.0001,
    };

    try
    {
        /** ADIOS class factory of IO class objects */
        adios2::ADIOS adios;

        /* set up parameters needed to use plugins */
        // In PluginOperator::Operate(), PluginName and PluginLibrary are saved each time
        // so in InverseOperate(), we can use those to load the operator plugin each time.
        //
        //adios2::Params params;
        //params["PluginName"] = "ExampleOperator";
        //params["PluginLibrary"] = "OperatorPlugin";
        //adios.LoadPlugin(params);

        /*** IO class object: settings and factory of Settings: Variables,
         * Parameters, Transports, and Execution: Engines */
        adios2::IO io = adios.DeclareIO("TestIO");
        io.SetEngine("BP4");

        /** global array: name, { shape (total dimensions) }, { start (local) },
         * { count (local) }, all are constant dimensions */
        adios2::Engine reader = io.Open("testOperator.bp", adios2::Mode::Read);
        auto var = io.InquireVariable<float>("data");
        if (!var)
        {
            std::cout << "variable does not exist" << std::endl;
        }

        // shouldn't be needed on read side actually
        //var.AddOperation("plugin", params);
        //var.AddOperation("bzip2", {{"accuracy", "0.01"}});

        std::vector<float> readFloats;
        reader.Get<float>(var, readFloats);
        reader.PerformGets();

        if (readFloats == myFloats)
        {
            std::cout << "data was read correctly!" << std::endl;
        }
        else
        {
            std::cout << "data was not read correctly!" << std::endl;
        }

        /** Engine becomes unreachable after this*/
        reader.Close();
    }
    catch (std::invalid_argument &e)
    {
        std::cout << "Invalid argument exception, STOPPING PROGRAM\n";
        std::cout << e.what() << "\n";
    }
    catch (std::ios_base::failure &e)
    {
        std::cout << "IO System base failure exception, STOPPING PROGRAM\n";
        std::cout << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cout << "Exception, STOPPING PROGRAM from rank\n";
        std::cout << e.what() << "\n";
    }

    return 0;
}

