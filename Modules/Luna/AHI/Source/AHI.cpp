/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AHI.cpp
* @author JXMaster
* @date 2023/10/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_AHI_API LUNA_EXPORT
#include "AHI.hpp"
#include "../AHIError.hpp"
#include <Luna/Runtime/Module.hpp>
namespace Luna
{
    static RV register_ahi_error_codes()
    {
        if (!register_error_category(AHI::ERROR_CATEGORY, "AHI") ||
            !register_error_code(AHI::E_FORMAT_NOT_SUPPORTED, "format_not_supported", "The audio format is not supported.") ||
            !register_error_code(AHI::E_DEVICE_TYPE_NOT_SUPPORTED, "device_type_not_supported", "The audio device type is not supported.") ||
            !register_error_code(AHI::E_SHARE_MODE_NOT_SUPPORTED, "share_mode_not_supported", "The audio sharing mode is not supported.") ||
            !register_error_code(AHI::E_NO_BACKEND, "no_backend", "No audio backend is available.") ||
            !register_error_code(AHI::E_NO_DEVICE, "no_device", "No matching audio device is available.") ||
            !register_error_code(AHI::E_API_NOT_FOUND, "api_not_found", "The requested audio API was not found.") ||
            !register_error_code(AHI::E_BAD_DEVICE_CONFIG, "bad_device_config", "The audio device configuration is invalid.") ||
            !register_error_code(AHI::E_LOOP, "loop", "An audio routing loop was detected.") ||
            !register_error_code(AHI::E_DEVICE_NOT_STARTED, "device_not_started", "The audio device has not been started.") ||
            !register_error_code(AHI::E_DEVICE_NOT_STOPPED, "device_not_stopped", "The audio device has not been stopped.") ||
            !register_error_code(AHI::E_FAILED_TO_INIT_BACKEND, "failed_to_init_backend", "The audio backend failed to initialize.") ||
            !register_error_code(AHI::E_FAILED_TO_OPEN_BACKEND_DEVICE, "failed_to_open_backend_device", "The backend audio device failed to open.") ||
            !register_error_code(AHI::E_FAILED_TO_START_BACKEND_DEVICE, "failed_to_start_backend_device", "The backend audio device failed to start.") ||
            !register_error_code(AHI::E_FAILED_TO_STOP_BACKEND_DEVICE, "failed_to_stop_backend_device", "The backend audio device failed to stop.") ||
            !register_error_code(AHI::E_BACKEND_NOT_ENABLED, "backend_not_enabled", "The requested audio backend is not enabled in this build."))
        {
            return set_error(E_ALREADY_EXISTS, "AHI error metadata conflicts with an existing registration.");
        }
        return ok;
    }

    namespace AHI
    {
        RV init()
        {
            return platform_init();
        }
        void close()
        {
            platform_close();
        }
        struct AHIModule : public Module
        {
            virtual const c8* get_name() override { return "AHI"; }
            virtual RV on_register() override
            {
                return register_ahi_error_codes();
            }
            virtual RV on_init() override
            {
                return platform_init();
            }
            virtual void on_close() override
            {
                platform_close();
            }
        };
    }
    LUNA_AHI_API Module* module_ahi()
    {
        static AHI::AHIModule m;
        return &m;
    }
}
