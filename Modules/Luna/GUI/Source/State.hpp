/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file State.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include "../Base.hpp"
#include <Luna/Runtime/Blob.hpp>
#include "State.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            enum class ActionType : u8
            {
                button,
                button_group,
                button_group_multi,
                choice,
                disclosure,
                input_text,
                slider_float,
                slider_int,
                drag_float,
                drag_int,
                scroll_view,
                tab_bar
            };

            enum class ChoiceOperation : u8
            {
                none,
                toggle_bool,
                set_bool,
                set_int
            };

            struct ButtonAction
            {
                GUICore::id_t id = 0;
                bool enabled = true;
                struct ButtonVisualState* state = nullptr;
            };

            struct ButtonGroupAction
            {
                i32* selected_index = nullptr;
                GUICore::id_t* item_ids = nullptr;
                usize item_count = 0;
                bool enabled = true;
                struct ButtonGroupState* state = nullptr;
            };

            struct ButtonGroupMultiAction
            {
                bool* selected = nullptr;
                GUICore::id_t* item_ids = nullptr;
                usize item_count = 0;
                bool enabled = true;
            };

            struct ChoiceAction
            {
                GUICore::id_t id = 0;
                ChoiceOperation operation = ChoiceOperation::none;
                bool* bool_value = nullptr;
                i32* int_value = nullptr;
                i32 set_value = 0;
                bool enabled = true;
                bool selected = false;
                struct ChoiceVisualState* state = nullptr;
            };

            struct DisclosureAction
            {
                GUICore::id_t id = 0;
                bool enabled = true;
                bool can_toggle = true;
                struct DisclosureState* state = nullptr;
            };

            struct TextInputAction
            {
                GUICore::id_t id = 0;
                String* value = nullptr;
                bool enabled = true;
                bool read_only = false;
                GUICore::FontDesc font;
                f32 font_size = 16.0f;
                f32 padding_x = 8.0f;
                struct TextInputState* state = nullptr;
            };

            struct SliderFloatAction
            {
                GUICore::id_t id = 0;
                f32* value = nullptr;
                f32 minimum = 0.0f;
                f32 maximum = 1.0f;
                f32 navigation_step = 0.01f;
                bool enabled = true;
            };

            struct SliderIntAction
            {
                GUICore::id_t id = 0;
                i32* value = nullptr;
                i32 minimum = 0;
                i32 maximum = 100;
                f32 navigation_step = 0.01f;
                bool enabled = true;
            };

            struct DragFloatAction
            {
                GUICore::id_t id = 0;
                f32* value = nullptr;
                u8 count = 1;
                f32 minimum = 0.0f;
                f32 maximum = 0.0f;
                f32 speed = 0.01f;
                bool enabled = true;
                struct DragState* state = nullptr;
            };

            struct DragIntAction
            {
                GUICore::id_t id = 0;
                i32* value = nullptr;
                u8 count = 1;
                i32 minimum = 0;
                i32 maximum = 0;
                f32 speed = 1.0f;
                bool enabled = true;
                struct DragState* state = nullptr;
            };

            struct ScrollAction
            {
                GUICore::id_t id = 0;
                GUICore::id_t horizontal_bar_id = 0;
                GUICore::id_t vertical_bar_id = 0;
                ScrollViewDesc desc;
                struct ScrollState* state = nullptr;
                GUICore::ScrollViewportLayoutDesc* layout_desc = nullptr;
            };

            struct TabAction
            {
                GUICore::id_t id = 0;
                i32* selected_index = nullptr;
                bool enabled = true;
                struct TabState* state = nullptr;
            };

            struct Action
            {
                ActionType type = ActionType::button_group;
                GUICore::id_t id = 0;
                void* data = nullptr;
            };

            struct ScrollBuildScope
            {
                GUICore::ElementHandle viewport;
                GUICore::ElementHandle content;
                ScrollAction* data = nullptr;
            };

            struct TabBuildScope
            {
                GUICore::ElementHandle bar;
                TabAction* data = nullptr;
                bool content_open = false;
                GUICore::ElementHandle content;
            };

            struct [[Luna::struct("{BF108424-36AE-4BA2-96F1-4533BD8A8FE9}")]] FrameState
            {
                u32 generation = 0;
                Vector<Blob> blocks;
                usize block_index = 0;
                usize offset = 0;
                Vector<Action> actions;
                Vector<ScrollBuildScope> scroll_stack;
                Vector<TabBuildScope> tab_stack;
            };

            struct [[Luna::struct("{66221FD7-35D2-4A64-816B-A9838E47621E}")]] ButtonGroupState
            {
                f32 animated_index = 0.0f;
                bool initialized = false;
            };

            struct [[Luna::struct("{08C40325-AE0B-4307-A1BB-5A227A647E41}")]] ButtonVisualState
            {
                f32 hovered = 0.0f;
                f32 active = 0.0f;
            };

            struct [[Luna::struct("{CC2C00CD-195D-47D6-9C2E-516A50C8C1B1}")]] ChoiceVisualState
            {
                f32 hovered = 0.0f;
                f32 active = 0.0f;
                f32 selected = 0.0f;
                bool initialized = false;
            };

            struct [[Luna::struct("{656D0143-755B-4F46-A471-27B0049B2DCE}")]] DisclosureState
            {
                bool open = true;
                bool initialized = false;
                f32 animation = 1.0f;
            };

            struct [[Luna::struct("{29355D28-48C6-457F-A396-F0459D4CC029}")]] DragState
            {
                bool dragging = false;
                f32 start_pointer_x = 0.0f;
                f32 start_float = 0.0f;
                i32 start_int = 0;
            };

            struct [[Luna::struct("{47C7A918-1B06-4351-B5DE-14469AF613B3}")]] TextInputState
            {
                usize cursor = 0;
                usize selection_anchor = USIZE_MAX;
                bool selecting = false;
                f32 blink_time = 0.0f;
                f32 scroll_x = 0.0f;
            };

            struct [[Luna::struct("{650D6CB0-0784-4012-9035-31D71C2A70D4}")]] ScrollState
            {
                Float2U offset = Float2U(0.0f);
                f32 visibility = 0.0f;
                f32 idle_time = 0.0f;
                bool dragging_horizontal = false;
                bool dragging_vertical = false;
            };

            struct [[Luna::struct("{060E52A8-BA28-4CA5-8F80-1B41DF0BAF12}")]] TabState
            {
                f32 animated_index = 0.0f;
                bool initialized = false;
                Vector<GUICore::id_t> header_ids;
                Vector<String> header_labels;
            };
        }
    }
}
