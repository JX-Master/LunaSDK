/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Rectangle.hpp
* @author JXMaster
* @date 2024/5/21
*/
#pragma once
#include "../../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Rectangle : Widget
        {
            lustruct("GUI::Rectangle", "{d0e27859-1439-4089-bf55-b8fe82b24c47}");

            virtual RV draw(IContext* ctx, IDrawList* draw_list) override;
        };
    }
}