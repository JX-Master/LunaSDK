/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.mm
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Adapter.h"

#import <Foundation/Foundation.h>

namespace Luna
{
    namespace RHI
    {
        Vector<Ref<IAdapter>> g_adapters;
        void Adapter::init()
        {
            @autoreleasepool
            {
                NSString* name = [m_device name];
                m_name = [name cStringUsingEncoding: NSUTF8StringEncoding];
            }
        }
        void init_adapters()
        {
            @autoreleasepool
            {
                g_adapters.clear();
                NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
                u32 num_devices = [devices count];
                for(u32 i = 0; i < num_devices; ++i)
                {
                    id<MTLDevice> dev = devices[i];
                    Ref<Adapter> adapter = new_object<Adapter>();
                    adapter->m_device = dev;
                    adapter->init();
                    g_adapters.push_back(Ref<IAdapter>(adapter));
                }
            }
        }
        LUNA_RHI_API Vector<Ref<IAdapter>> get_adapters()
        {
            return g_adapters;
        }
    }
}
