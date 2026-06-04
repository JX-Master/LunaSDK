/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MetalRHI.mm
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../RHI.hpp"
#include "MetalDevice.h"
#include "MetalAdapter.h"
#include "MetalCommandBuffer.h"
#include "MetalDescriptorSet.h"
#include "MetalResource.h"
#include "MetalFence.h"
#include "MetalPipelineState.h"
#include "MetalQueryHeap.h"
#include "MetalPipelineLayout.h"
#include "MetalSwapChain.h"
#include "RHI.meta.generated.hpp"
namespace Luna
{
    namespace RHI
    {
        RV render_api_init()
        {
            lutry
            {
                Meta::register_RHI_types();
                init_adapters();
                luexp(init_main_device());
            }
            lucatchret;
            return ok;
        }
        void render_api_close()
        {
            g_main_device.reset();
            g_adapters.clear();
            g_adapters.shrink_to_fit();
        }
        LUNA_RHI_API BackendType get_backend_type()
        {
            return BackendType::metal;
        }
    }
}
