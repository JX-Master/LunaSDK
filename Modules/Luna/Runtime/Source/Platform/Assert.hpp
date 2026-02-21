/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Assert.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Reports an assertion failure information to the underlying OS or CRT.
        //! This function works in all builds, and can be called even if the runtime is not initialized.
        //! The behavior of this function depends on the OS/CRT implementation, but in general it will 
        //! present an error message box and then terminate the program.
        //! 
        //! If the underlying platform does not support assertions, it can simply terminate the program,
        //! or pause the program at where the assertion fails.
        //! @param[in] msg The UTF-8 error message to show.
        //! @param[in] file The UTF-8 name of the file that causes the panic.
        //! @param[in] line The code line the assertion is placed.
        void assert_fail(const c8* msg, const c8* file, u32 line);

        //! Triggers a debug break, pauses the program and attaches the debugger to the program.
        //! This only works in debug build. In release build, this function returns directly. If the underlying
        //! platform does not support debug break, this function is skipped and returned directly.
        void debug_break();
    }
}