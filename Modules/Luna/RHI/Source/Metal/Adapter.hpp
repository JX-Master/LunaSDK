/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
    namespace RHI
    {
        extern NSPtr<NS::Array> g_devices;

        RV init_devices();
        void clear_devices();
    }
}