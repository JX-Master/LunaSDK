/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.cpp
* @author JXMaster
* @date 2023/10/15
*/
#include "../AHI.hpp"
#include "Common.hpp"
#include "AdapterImpl.hpp"
#include "DeviceImpl.hpp"
#include "AHI.meta.generated.hpp"

namespace Luna
{
    namespace AHI
    {
        ma_context g_context;

        RV platform_init()
        {
            Meta::register_AHI_types();
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
