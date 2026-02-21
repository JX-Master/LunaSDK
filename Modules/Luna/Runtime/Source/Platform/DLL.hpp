/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "Result.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Loads the specified library to the process's address space. This call may load additional libraries
        //! required by the specified library.
        //! @param[in] path The path of the library file. It can be one `.dll` or `.exe` file on Windows,
        //! or one `.so` file on POSIX systems.
        //! @return Returns one handle that represents the loaded library.
        Result load_library(const c8* path, opaque_t& out_handle);

        //! Unloads the specified library.
        //! @param[in] handle The library handle returned by @ref load_library.
        //! @remark The library handle is reference counted: every call to  @ref load_library for the same library
        //! file increases the reference counter, and every @ref free_library for the same library handle decreases the 
        //! reference counter. The library will be removed from the process's address space when the reference counter drop to 0.
        //! 
        //! When one library is removed from the process's address space, it will decrease reference counters for all its dependent 
        //! libraries, and removes them as well if their reference counters drop to 0.
        void free_library(opaque_t handle);

        //! Gets the function address (function pointer) of one function in the library from its symbol name.
        //! @param[in] handle The library handle returned by @ref load_library.
        //! @param[in] symbol The function's symbol name.
        //! @param[out] out_addr Returns function address of the specified function.
        Result get_library_function(opaque_t handle, const c8* symbol, void*& out_addr);
    }
}