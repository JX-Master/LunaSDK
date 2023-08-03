/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fence.cpp
* @author JXMaster
* @date 2023/8/3
*/
#include "Fence.hpp"

namespace Luna
{
    namespace RHI
    {
        RV Fence::init()
        {
            m_fence = box(m_device->m_device->newFence());
            return ok;
        }
    }
}