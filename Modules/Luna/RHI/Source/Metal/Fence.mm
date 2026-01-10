/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fence.mm
* @author JXMaster
* @date 2023/8/3
*/
#include "Fence.h"

namespace Luna
{
    namespace RHI
    {
        RV Fence::init()
        {
            m_fence = [m_device->m_device newFence];
            return ok;
        }
        void Fence::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_fence.label = label;
            }
        }
    }
}