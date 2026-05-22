/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Description.hpp"

namespace Luna
{
    namespace GUI
    {
        struct IGUIContext : virtual Interface
        {
            luiid("{E58F6F6C-48A9-42AB-86F3-898419C207BC}");

            virtual void begin_frame(const GUIFrameDesc& desc) = 0;
            virtual void add_input_event(const GUIInputEvent& event) = 0;
            virtual void add_input_events(Span<const GUIInputEvent> events) = 0;
            virtual R<GUIDescription> end_build() = 0;
            virtual RV submit(const GUIDescription& desc) = 0;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) = 0;
        };

        LUNA_GUI_API Ref<IGUIContext> new_context(RHI::IDevice* device = nullptr);

        LUNA_GUI_API void set_current_context(IGUIContext* context);
        LUNA_GUI_API IGUIContext* get_current_context();

        LUNA_GUI_API Module* module_gui();
    }
}
