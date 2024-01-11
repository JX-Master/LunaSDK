/*!
* This file is a portion of Luna SDK.
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
    namespace AHIError
    {
        LUNA_AHI_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("AHIError");
			return e;
		}
        LUNA_AHI_API ErrCode format_not_supported()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "format_not_supported");
			return e;
		}
        LUNA_AHI_API ErrCode device_type_not_supported()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "device_type_not_supported");
			return e;
		}
        LUNA_AHI_API ErrCode share_mode_not_supported()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "share_mode_not_supported");
			return e;
		}
        LUNA_AHI_API ErrCode no_backend()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "no_backend");
			return e;
		}
        LUNA_AHI_API ErrCode no_device()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "no_device");
			return e;
		}
        LUNA_AHI_API ErrCode api_not_found()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "api_not_found");
			return e;
		}
        LUNA_AHI_API ErrCode bad_device_config()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "bad_device_config");
			return e;
		}
        LUNA_AHI_API ErrCode loop()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "loop");
			return e;
		}
        LUNA_AHI_API ErrCode backend_not_enabled()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "backend_not_enabled");
			return e;
		}
        LUNA_AHI_API ErrCode device_not_started()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "device_not_started");
			return e;
		}
        LUNA_AHI_API ErrCode device_not_stopped()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "device_not_stopped");
			return e;
		}
        LUNA_AHI_API ErrCode failed_to_init_backend()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "failed_to_init_backend");
			return e;
		}
        LUNA_AHI_API ErrCode failed_to_open_backend_device()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "failed_to_open_backend_device");
			return e;
		}
        LUNA_AHI_API ErrCode failed_to_start_backend_device()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "failed_to_start_backend_device");
			return e;
		}
        LUNA_AHI_API ErrCode failed_to_stop_backend_device()
		{
			static ErrCode e = get_error_code_by_name("AHIError", "failed_to_stop_backend_device");
			return e;
		}
    }
}