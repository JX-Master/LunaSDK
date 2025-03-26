/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainCallbacks.hpp
* @author JXMaster
* @date 2025/3/21
*/
#pragma once

#include <Luna/Runtime/Base.hpp>

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

        //! Specifies the application state.
        enum class AppStatus : i32
        {
            //! The application is running.
            running = 0,
            //! The application has encountered a fatal error and is exiting with abnormal exit code.
            failing = 1,
            //! The application is exiting with a normal exit code.
            exiting = 2,
        };

        //! @}
    }

    //! @addtogroup Window
    //! @{

    //! Called when the application is initialized.
    //! @param[out] app_state The opaque pointer that can be set by the user program to the application state object.
    //! This pointer will be passed to all application callbacks.
    //! @param[in] argc The number of startup arguments, usually passed from the command line.
    //! @param[in] argv The startup arguments array with `argc` number of arguments.
    //! @return Returns @ref AppStatus::running to indicate that the function is finished without any error, and we should continue running the application.
    //! 
    //! Returns @ref AppStatus::failing to indicate that the function is failed. In such case, the system calls @ref app_close to close the application then returns one system 
    //! code that indicates abnormal exiting of the application.
    //! 
    //! Returns @ref AppStatus::exiting to indicate that the function is finished without any error, and the application is exiting after this function. In such case, the system
    //! calls @ref app_close to close the application then returns one system code (usually 0) that indicates the application is exited normally.
    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[]);

    //! Called when the application state should be updated. This is usually called repeatly by system until application is closed.
    //! @param[in] app_state The opaque pointer that the application specify in @ref app_init.
    //! @return Returns @ref AppStatus::running to indicate that the function is finished without any error, and we should continue running the application.
    //! 
    //! Returns @ref AppStatus::failing to indicate that the function is failed. In such case, the system calls @ref app_close to close the application then returns one system 
    //! code that indicates abnormal exiting of the application.
    //! 
    //! Returns @ref AppStatus::exiting to indicate that the function is finished without any error, and the application is exiting after this function. In such case, the system
    //! calls @ref app_close to close the application then returns one system code (usually 0) that indicates the application is exited normally.
    Window::AppStatus app_update(opaque_t app_state);

    //! Called when the application is exiting.
    //! @param[in] app_state The opaque pointer that the application specify in @ref app_init.
    //! @param[in] status The application status when termination the app. This will be either @ref Window::AppStatus::exiting (for normal termination) 
    //! or @ref Window::AppStatus::failing (for abnormal termination).
    void app_close(opaque_t app_state, Window::AppStatus status);

    //! @}
}