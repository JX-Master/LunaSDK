/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResizableWindow.hpp
* @author JXMaster
* @date 2024/5/10
*/
#pragma once
#include "../../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ResizableWindowBuildData : WidgetBuildData
        {
            lustruct("GUI::ResizableWindowBuildData", "{e9e6a2ac-42b9-49bf-9d68-22fb604b94cb}");

            u32 ctx_width = 0;
            u32 ctx_height = 0;

            void update(IContext* ctx) override;
            RV build(IContext* ctx) override;
        };
    }
}