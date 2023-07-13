/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.cpp
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Adapter.hpp"

namespace Luna
{
    namespace RHI
    {
        NSPtr<NS::Array> g_devices;

        RV init_devices()
        {
            g_devices = box(MTL::CopyAllDevices());
        }
        void clear_devices()
        {
            g_devices.reset();
        }
        LUNA_RHI_API u32 get_num_adapters()
        {
            return (u32)g_devices->count();
        }
        LUNA_RHI_API AdapterDesc get_adapter_desc(u32 adapter_index)
        {
            lucheck(adapter_index < get_num_adapters());
            AutoreleasePool pool;
            MTL::Device* dev = g_devices->object<MTL::Device>(adapter_index);
            AdapterDesc desc;
            NS::String* name = dev->name();
            strncpy(desc.name, name->cString(NS::StringEncoding::UTF8StringEncoding), 256);
            desc.type = dev->lowPower() ? AdapterType::integrated_gpu : AdapterType::discrete_gpu;
            desc.local_memory = dev->recommendedMaxWorkingSetSize();
            desc.shared_memory = dev->recommendedMaxWorkingSetSize();
            return desc;
        }
    }
}