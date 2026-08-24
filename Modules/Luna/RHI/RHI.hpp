/*!
* This file is a portion of LunaSDK.
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
    //! @defgroup RHIResultCodes RHI Result Codes
    //! @}
    namespace RHI
    {
        //! @addtogroup RHIResultCodes
        //! @{
        
        //! The RHI error category identifier.
        inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI);
        //! The device failed because the application submitted invalid commands.
        inline constexpr ResultCode E_DEVICE_HUNG = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -1);
        //! The device was reset and must be recreated.
        inline constexpr ResultCode E_DEVICE_RESET = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -2);
        //! The graphics device was removed or became unavailable.
        inline constexpr ResultCode E_DEVICE_REMOVED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -3);
        //! The graphics driver reported an internal error.
        inline constexpr ResultCode E_DRIVER_INTERNAL_ERROR = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -4);
        //! Presentation statistics were interrupted and are disjoint.
        inline constexpr ResultCode E_FRAME_STATISTICS_DISJOINT = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -5);
        //! The swap chain is no longer compatible with its surface.
        inline constexpr ResultCode E_SWAP_CHAIN_OUT_OF_DATE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -6);
        //! The requested color space is not supported.
        inline constexpr ResultCode E_COLOR_SPACE_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RHI, -7);

        //! @}
    }
    
    struct Module;
    LUNA_RHI_API Module* module_rhi();
}
