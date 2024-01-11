/*!
* This file is a portion of Luna SDK.
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
    namespace AHIError
    {
        LUNA_AHI_API errcat_t errtype();
        LUNA_AHI_API ErrCode format_not_supported();
        LUNA_AHI_API ErrCode device_type_not_supported();
        LUNA_AHI_API ErrCode share_mode_not_supported();
        LUNA_AHI_API ErrCode no_backend();
        LUNA_AHI_API ErrCode no_device();
        LUNA_AHI_API ErrCode api_not_found();
        LUNA_AHI_API ErrCode bad_device_config();
        LUNA_AHI_API ErrCode loop();
        LUNA_AHI_API ErrCode backend_not_enabled();
        LUNA_AHI_API ErrCode device_not_started();
        LUNA_AHI_API ErrCode device_not_stopped();
        LUNA_AHI_API ErrCode failed_to_init_backend();
        LUNA_AHI_API ErrCode failed_to_open_backend_device();
        LUNA_AHI_API ErrCode failed_to_start_backend_device();
        LUNA_AHI_API ErrCode failed_to_stop_backend_device();
    }
}