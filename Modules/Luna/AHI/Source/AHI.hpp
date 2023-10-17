/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AHI.hpp
* @author JXMaster
* @date 2023/10/15
*/
#pragma once
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace AHI
    {
        RV platform_init();
        void platform_close();
    }
}