/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AHIError.hpp
* @author JXMaster
* @date 2023/10/15
*/
#pragma once
#include <Luna/Runtime/Error.hpp>

#ifndef LUNA_AHI_API
#define LUNA_AHI_API
#endif

namespace Luna
{
    namespace AHI
    {
        //! The AHI error category identifier.
        inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI);

        //! The audio format is not supported.
        inline constexpr ResultCode E_FORMAT_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -1);
        //! The audio device type is not supported.
        inline constexpr ResultCode E_DEVICE_TYPE_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -2);
        //! The requested sharing mode is not supported.
        inline constexpr ResultCode E_SHARE_MODE_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -3);
        //! No audio backend is available.
        inline constexpr ResultCode E_NO_BACKEND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -4);
        //! No matching audio device is available.
        inline constexpr ResultCode E_NO_DEVICE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -5);
        //! The requested audio API is not available.
        inline constexpr ResultCode E_API_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -6);
        //! The audio device configuration is invalid.
        inline constexpr ResultCode E_BAD_DEVICE_CONFIG = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -7);
        //! An audio routing loop was detected.
        inline constexpr ResultCode E_LOOP = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -8);
        //! The audio device has not been started.
        inline constexpr ResultCode E_DEVICE_NOT_STARTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -9);
        //! The audio device has not been stopped.
        inline constexpr ResultCode E_DEVICE_NOT_STOPPED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -10);
        //! The audio backend failed to initialize.
        inline constexpr ResultCode E_FAILED_TO_INIT_BACKEND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -11);
        //! The backend audio device failed to open.
        inline constexpr ResultCode E_FAILED_TO_OPEN_BACKEND_DEVICE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -12);
        //! The backend audio device failed to start.
        inline constexpr ResultCode E_FAILED_TO_START_BACKEND_DEVICE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -13);
        //! The backend audio device failed to stop.
        inline constexpr ResultCode E_FAILED_TO_STOP_BACKEND_DEVICE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -14);
        //! The requested audio backend is not enabled in this build.
        inline constexpr ResultCode E_BACKEND_NOT_ENABLED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::AHI, -15);

    }
}
