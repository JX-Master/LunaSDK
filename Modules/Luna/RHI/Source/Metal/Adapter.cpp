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
        Vector<Ref<IAdapter>> g_adapters;
        void Adapter::init()
        {
            AutoreleasePool pool;
            NS::String* name = m_device->name();
            strncpy(m_name, name->cString(NS::StringEncoding::UTF8StringEncoding), 256);
        }
        void init_adapters()
        {
            AutoreleasePool pool;
            g_adapters.clear();
            NSPtr<NS::Array> devices = box(MTL::CopyAllDevices());
            u32 num_devices = devices->count();
            for(u32 i = 0; i < num_devices; ++i)
            {
                MTL::Device* dev = devices->object<MTL::Device>(i);
                Ref<Adapter> adapter = new_object<Adapter>();
                adapter->m_device = retain(dev);
                adapter->init();
                g_adapters.push_back(Ref<IAdapter>(adapter));
            }
        }
        LUNA_RHI_API Vector<Ref<IAdapter>> get_adapters()
        {
            return g_adapters;
        }
    }
}
