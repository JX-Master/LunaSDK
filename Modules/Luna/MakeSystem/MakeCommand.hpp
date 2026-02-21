/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeCommand.hpp
* @author JXMaster
* @date 2026/2/9
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
    namespace MakeSystem
    {
        //! Defines a build action that can be executed by the make system.
        struct IMakeCommand : virtual Interface
        {
            luiid("{caec8bd0-9ad4-4660-86e3-8640b240c2a7}");

            //! Executes the build action for a node.
            //! @param[in] log_handler The log handler that is used to output logs.
            //! @return Returns zero on success, non-zero on failure.
            virtual RV execute(LogHandler& log_handler) = 0;
        };
    }
}