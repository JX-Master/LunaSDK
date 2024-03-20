/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.hpp
* @author JXMaster
* @date 2022/4/12
*/
#pragma once
#include "Device.hpp"
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif
namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI RHI
        //! Render Hardware Interface (RHI) module provides uniform API to use platform's graphcis hardware (GPU) to accelerate rendering and parallel computing tasks.
        //! @{
        
        //! Lists supported RHI backends (implementation APIs).
        enum class BackendType : u8
        {
            //! The Microsoft Direct3D 12 backend.
            d3d12,
            //! The Vulkan backend.
            vulkan,
            //! The Apple Metal backend.
            metal,
        };

        //! Gets the backend type.
        //! @details The backend type is decided when compiling RHI module and cannot be changed at run time.
        //! @return Returns the current backend type.
        LUNA_RHI_API BackendType get_backend_type();

        //! @}
    }
    //! @addtogroup RHI
    //! @{
    //! @defgroup RHIError RHI Errors
    //! @}
    namespace RHIError
    {
        //! @addtogroup RHIError
        //! @{
        
        LUNA_RHI_API errcat_t errtype();
        //! The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed.
        LUNA_RHI_API ErrCode device_hung();
        //! The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.
        LUNA_RHI_API ErrCode device_reset();
        //! The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device.
        //! Sending badly formed commands to the device will also cause device being (virtually) removed from the application.
        LUNA_RHI_API ErrCode device_removed();
        //! The driver encountered a problem and was put into the device removed state.
        LUNA_RHI_API ErrCode driver_internal_error();
        //! An event (for example, a power cycle) interrupted the gathering of presentation statistics.
        LUNA_RHI_API ErrCode frame_statistics_disjoint();
        //! The swap chain is no longer compatible with the surface and should be reset.
        LUNA_RHI_API ErrCode swap_chain_out_of_date();
        
        //! @}
    }
    
    struct Module;
    LUNA_RHI_API Module* module_rhi();
}