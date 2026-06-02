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
        struct IContext : virtual Interface
        {
            luiid("{E58F6F6C-48A9-42AB-86F3-898419C207BC}");

            virtual void begin_frame(const FrameDesc& desc) = 0;
            virtual void add_input_event(const InputEvent& event) = 0;
            virtual void add_input_events(Span<const InputEvent> events) = 0;
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f)) = 0;
            virtual void pop_layer() = 0;
            virtual ItemHandle add_node(Ref<Node> node, const c8* label = nullptr, bool interactive = false) = 0;
            virtual u64 generation() const = 0;
            virtual object_t get_state(id_t id) = 0;
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;
            virtual void clear_state(id_t id) = 0;
            virtual R<Description> end_build() = 0;
            virtual RV submit(const Description& desc) = 0;
            virtual void set_clipboard_io(const ClipboardIO& io) = 0;
            virtual TextInputState get_text_input_state() = 0;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context(RHI::IDevice* device = nullptr);

        LUNA_GUI_API Module* module_gui();
    }
}
