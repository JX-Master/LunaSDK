/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.hpp
* @author JXMaster
* @date 2023/11/26
*/
#pragma once
#include "Result.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeDLL DLL loading
    //! @}

    //! @addtogroup RuntimeDLL
    //! @{
    
    //! Loads the specified library to the process's address space. This call may load additional libraries
    //! required by the specified library.
    //! @param[in] path The path of the library file. It can be one `.dll` or `.exe` file on Windows,
    //! or one `.so` file on POSIX systems.
    //! @return Returns one handle that represents the loaded library.
    LUNA_RUNTIME_API R<opaque_t> load_library(const c8* path);

    //! Unloads the specified library.
    //! @param[in] handle The library handle returned by @ref load_library.
    //! @remark The library handle is reference counted: every call to  @ref load_library for the same library
    //! file increases the reference counter, and every @ref free_library for the same library handle decreases the 
    //! reference counter. The library will be removed from the process's address space when the reference counter drop to 0.
    //! 
    //! When one library is removed from the process's address space, it will decrease reference counters for all its dependent 
    //! libraries, and removes them as well if their reference counters drop to 0.
    LUNA_RUNTIME_API void free_library(opaque_t handle);

    //! Gets the function address (function pointer) of one function in the library from its symbol name.
    //! @param[in] handle The library handle returned by @ref load_library.
    //! @param[in] symbol The function's symbol name.
    //! @return Returns the function address of the specified function.
    LUNA_RUNTIME_API R<void*> get_library_function(opaque_t handle, const c8* symbol);

    //! @}
}