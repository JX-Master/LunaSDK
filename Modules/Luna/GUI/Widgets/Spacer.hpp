/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Spacer.hpp
* @author JXMaster
* @date 2024/7/29
*/
#pragma once
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Spacer : Widget
        {
            lustruct("GUI::Spacer", "d2c97b89-4ba9-4f0e-a20e-5200a5264e12");

            // Left blank, since spacer is used only for alignment.
        };
    }
}