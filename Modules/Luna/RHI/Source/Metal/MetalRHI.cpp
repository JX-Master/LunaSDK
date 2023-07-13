/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MetalRHI.cpp
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../RHI.hpp"
#include "Device.hpp"
#include "Adapter.hpp"
namespace Luna
{
    namespace RHI
    {
        RV render_api_init()
        {
            lutry
            {
                luexp(init_devices());
                luexp(init_main_device());
            }
            lucatchret;
            return ok;
        }
		void render_api_close()
        {
            g_main_device.reset();
        }
        LUNA_RHI_API APIType get_current_platform_api_type()
		{
			return APIType::metal;
		}
    }
}