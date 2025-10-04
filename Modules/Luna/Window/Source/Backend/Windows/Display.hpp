/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.hpp
* @author JXMaster
* @date 2025/10/2
*/
#pragma once
#include "../../../Display.hpp"
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace Window
    {
        RV display_init();
        void display_close();
    }
}