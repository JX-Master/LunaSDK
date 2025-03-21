/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMain.hpp
* @author JXMaster
* @date 2025/3/19
*/
#pragma once
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace Window
    {
        //! The result code of application callbacks.
        enum class AppResult : i32
        {
            //! Indicates that the function is finished.
            ok = 0,
            //! Indicates that the function is failed.
            failed = 1,
            //! Indicates that the function is finished, and the application is exiting after this function.
            exiting = 2,
        };

        
    }

    //! Called when the application is initialized.
    //! @param[out] app_state The opaque pointer that can be set by the user program to the application state object.
    //! This pointer will be passed to all application callbacks.
    //! @param[in] argc The number of startup arguments, usually passed from the command line.
    //! @param[in] argv The startup arguments array with `argc` number of arguments.
    //! @return Returns @ref AppResult::ok to indicate that the function is finished without any error, and we should continue running the application.
    //! 
    //! Returns @ref AppResult::failed to indicate that the function is failed. In such case, the system calls @ref app_close to close the application then returns one system 
    //! code that indicates abnormal exiting of the application.
    //! 
    //! Returns @ref AppResult::exiting to indicate that the function is finished without any error, and the application is exiting after this function. In such case, the system
    //! calls @ref app_close to close the application then returns one system code (usually 0) that indicates the application is exited normally.
    Window::AppResult app_init(opaque_t* app_state, int argc, char* argv[]);

    //! Called when the application state should be updated. This is usually called repeatly by system until application is closed.
    //! @param[in] app_state The opaque pointer that the application specify in @ref app_init.
    //! @return Returns @ref AppResult::ok to indicate that the function is finished without any error, and we should continue running the application.
    //! 
    //! Returns @ref AppResult::failed to indicate that the function is failed. In such case, the system calls @ref app_close to close the application then returns one system 
    //! code that indicates abnormal exiting of the application.
    //! 
    //! Returns @ref AppResult::exiting to indicate that the function is finished without any error, and the application is exiting after this function. In such case, the system
    //! calls @ref app_close to close the application then returns one system code (usually 0) that indicates the application is exited normally.
    Window::AppResult app_update(opaque_t app_state);

    //! Called when the application is exiting.
    //! @param[in] app_state The opaque pointer that the application specify in @ref app_init.
    void app_close(opaque_t app_state);
}

#ifdef LUNA_PLATFORM_WINDOWS
#include "Windows/AppMainWindows.inl"
#endif