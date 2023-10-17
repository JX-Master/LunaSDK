/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.cpp
* @author JXMaster
* @date 2023/10/15
*/
#include "../AHI.hpp"
#include "Common.hpp"
#include "Adapter.hpp"
#include "Device.hpp"

namespace Luna
{
    namespace AHI
    {
        ma_context g_context;

        RV platform_init()
        {
            register_boxed_type<Adapter>();
            impl_interface_for_type<Adapter, IAdapter>();
            register_boxed_type<Device>();
            impl_interface_for_type<Device, IDevice>();
            auto r = ma_context_init(NULL, 0, NULL, &g_context);
            if(r != MA_SUCCESS)
            {
                return translate_ma_result(r);
            }
            return ok;
        }
        void platform_close()
        {
            ma_context_uninit(&g_context);
        }
    }
}