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
#include "../Workspace.hpp"
#include <Luna/Runtime/Blob.hpp>
#include "State.generated.hpp"

namespace Luna
{
    namespace EditorGUI
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
                tab_bar,
                table,
                popup,
                color_edit,
                dock_space
            };

            enum class ColorStorage : u8
            {
                f32,
                u8,
                rgba8
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
                GUI::id_t id = 0;
                bool enabled = true;
                struct ButtonVisualState* state = nullptr;
            };

            struct ButtonGroupAction
            {
                i32* selected_index = nullptr;
                GUI::id_t* item_ids = nullptr;
                usize item_count = 0;
                bool enabled = true;
                f32 animation_duration = 0.25f;
                struct ButtonGroupState* state = nullptr;
            };

            struct ButtonGroupMultiAction
            {
                bool* selected = nullptr;
                GUI::id_t* item_ids = nullptr;
                usize item_count = 0;
                bool enabled = true;
            };

            struct ChoiceAction
            {
                GUI::id_t id = 0;
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
                GUI::id_t id = 0;
                bool enabled = true;
                bool can_toggle = true;
                bool open_on_arrow = false;
                f32 arrow_min_x = 0.0f;
                f32 arrow_max_x = 0.0f;
                struct DisclosureState* state = nullptr;
            };

            struct TextInputAction
            {
                GUI::id_t id = 0;
                String* value = nullptr;
                bool enabled = true;
                bool read_only = false;
                GUI::FontDesc font;
                f32 font_size = 16.0f;
                f32 padding_x = 8.0f;
                struct TextInputState* state = nullptr;
            };

            struct SliderFloatAction
            {
                GUI::id_t id = 0;
                f32* value = nullptr;
                f32 minimum = 0.0f;
                f32 maximum = 1.0f;
                f32 navigation_step = 0.01f;
                bool enabled = true;
            };

            struct SliderIntAction
            {
                GUI::id_t id = 0;
                i32* value = nullptr;
                i32 minimum = 0;
                i32 maximum = 100;
                f32 navigation_step = 0.01f;
                bool enabled = true;
            };

            struct DragFloatAction
            {
                GUI::id_t id = 0;
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
                GUI::id_t id = 0;
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
                GUI::id_t id = 0;
                GUI::id_t scrollbar_group_id = 0;
                GUI::id_t horizontal_bar_id = 0;
                GUI::id_t vertical_bar_id = 0;
                ScrollViewDesc desc;
                struct ScrollState* state = nullptr;
                GUI::ScrollViewportLayoutDesc* layout_desc = nullptr;
            };

            struct TabAction
            {
                GUI::id_t id = 0;
                i32* selected_index = nullptr;
                bool enabled = true;
                TabBarFittingMode fitting_mode = TabBarFittingMode::none;
                struct TabState* state = nullptr;
            };

            struct TableAction
            {
                GUI::id_t id = 0;
                TableDesc desc;
                struct TableState* state = nullptr;
                GUI::TableLayoutDesc* layout_desc = nullptr;
                GUI::TableTrackDesc* mutable_columns = nullptr;
                GUI::id_t* splitter_ids = nullptr;
                usize splitter_count = 0;
            };

            struct PopupAction
            {
                id_t id = 0;
                GUI::ElementHandle root;
                PopupFlag flags = PopupFlag::none;
            };

            struct ColorEditAction
            {
                id_t id = 0;
                id_t popup_id = 0;
                id_t picker_id = 0;
                f32* f32_value = nullptr;
                u8* u8_value = nullptr;
                u32* rgba8_value = nullptr;
                ColorStorage storage = ColorStorage::f32;
                u8 value_count = 3;
                bool enabled = true;
                i32 rgb_before[4] = { 0, 0, 0, 255 };
                i32 hsv_before[3] = { 0, 0, 0 };
                struct ColorEditState* state = nullptr;
            };

            enum class DockDragMode : u8
            {
                none,
                floating_move,
                floating_resize,
                docked_title,
                splitter
            };

            enum class DockDropDirection : u8
            {
                none,
                center,
                left,
                right,
                up,
                down
            };

            struct DockTreeNode
            {
                bool active = true;
                bool split = false;
                DockSplitAxis split_axis = DockSplitAxis::x;
                f32 split_ratio = 0.5f;
                u32 parent = U32_MAX;
                u32 child0 = U32_MAX;
                u32 child1 = U32_MAX;
                Vector<id_t> tabs;
                id_t selected_tab = 0;
                f32 tab_indicator_x = 0.0f;
                f32 tab_indicator_width = 0.0f;
                bool tab_indicator_initialized = false;
                RectF rect;
                RectF splitter_rect;
            };

            struct DockPanelPersistentData
            {
                id_t id = 0;
                DockPanelMode mode = DockPanelMode::docking;
                RectF floating_rect = RectF(40.0f, 40.0f, 360.0f, 240.0f);
                RectF restored_floating_rect = RectF(40.0f, 40.0f, 360.0f, 240.0f);
                u32 z_order = 0;
            };

            struct [[Luna::struct("{91AB521B-6B03-447F-96F4-586811B4EDB3}")]] DockSpaceState
            {
                Vector<DockTreeNode> nodes;
                u32 root_node = U32_MAX;
                Vector<DockPanelPersistentData> panels;
                u32 next_z_order = 1;
                Float2U screen_origin = Float2U(0.0f);
                RectF dock_rect;
                DockDragMode drag_mode = DockDragMode::none;
                id_t drag_panel = 0;
                u32 drag_splitter = U32_MAX;
                Float2U drag_start_pointer = Float2U(0.0f);
                RectF drag_start_rect;
                f32 drag_start_ratio = 0.5f;
                u32 drop_target = U32_MAX;
                bool drop_target_available = false;
                DockDropDirection drop_direction = DockDropDirection::none;
            };

            struct DockPanelBuildInfo
            {
                id_t id = 0;
                String label;
                bool* open = nullptr;
                DockPanelDesc desc;
                bool submitted = false;
                bool visible = false;
                bool floating = false;
                id_t layer_id = 0;
                GUI::ElementHandle root;
                GUI::ElementHandle title;
                GUI::ElementHandle close;
                GUI::ElementHandle resize;
                GUI::ElementHandle raise;
                GUI::ElementHandle content;
            };

            struct DockSpaceBuildScope
            {
                id_t id = 0;
                GUI::ElementHandle root;
                DockSpaceDesc desc;
                DockSpaceState* state = nullptr;
                Vector<DockPanelBuildInfo> panels;
                i32 open_panel = -1;
            };

            struct DockPanelActionInfo
            {
                id_t id = 0;
                const c8* label = nullptr;
                bool* open = nullptr;
                DockPanelDesc desc;
                bool visible = false;
                bool floating = false;
                id_t layer_id = 0;
                GUI::ElementHandle root;
                GUI::ElementHandle title;
                GUI::ElementHandle close;
                GUI::ElementHandle resize;
                GUI::ElementHandle raise;
            };

            struct DockSpaceAction
            {
                id_t id = 0;
                GUI::ElementHandle root;
                DockSpaceDesc desc;
                DockSpaceState* state = nullptr;
                id_t indicator_layer_id = 0;
                DockPanelActionInfo* panels = nullptr;
                usize panel_count = 0;
            };

            struct Action
            {
                ActionType type = ActionType::button_group;
                GUI::id_t id = 0;
                void* data = nullptr;
            };

            struct ScrollBuildScope
            {
                GUI::ElementHandle viewport;
                GUI::ElementHandle content;
                ScrollAction* data = nullptr;
            };

            struct TabBuildScope
            {
                GUI::ElementHandle bar;
                TabAction* data = nullptr;
                bool content_open = false;
                GUI::ElementHandle content;
            };

            struct TableBuildScope
            {
                GUI::ElementHandle table;
                TableDesc desc;
                Vector<GUI::TableTrackDesc> columns;
                Vector<GUI::TableTrackDesc> rows;
                Vector<GUI::TableLayoutCell> cells;
                struct TableState* state = nullptr;
                bool column_widths_applied = false;
                bool row_open = false;
                bool row_visible = true;
                u32 current_row = 0;
                u32 row_previous_last_child = GUI::INVALID_ELEMENT;
                u32 max_columns = 0;
                f32 overscan_y = 0.0f;
            };

            struct PopupBuildScope
            {
                id_t popup_id = 0;
                GUI::ElementHandle root;
                id_t hovered_menu_item = 0;
            };

            struct MenuBarBuildScope
            {
                GUI::ElementHandle root;
                f32 gap = 4.0f;
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
                Vector<TableBuildScope> table_stack;
                Vector<PopupBuildScope> popup_stack;
                Vector<MenuBarBuildScope> menu_bar_stack;
                Vector<DockSpaceBuildScope> dock_space_stack;
            };

            struct [[Luna::struct("{66221FD7-35D2-4A64-816B-A9838E47621E}")]] ButtonGroupState
            {
                f32 animated_index = 0.0f;
                f32 animation_start_index = 0.0f;
                f32 animation_target_index = 0.0f;
                f32 animation_elapsed = 0.0f;
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
                bool editing = false;
                bool select_all = false;
                f32 start_pointer_x = 0.0f;
                f32 start_float = 0.0f;
                i32 start_int = 0;
                f32 edit_original_float = 0.0f;
                i32 edit_original_int = 0;
                String edit_text;
            };

            struct [[Luna::struct("{47C7A918-1B06-4351-B5DE-14469AF613B3}")]] TextInputState
            {
                usize cursor = 0;
                usize selection_anchor = USIZE_MAX;
                bool selecting = false;
                f32 blink_time = 0.0f;
                f32 scroll_x = 0.0f;
            };

            struct [[Luna::struct("{D562461E-6F3E-4640-A9A6-BBD24A69B562}")]] ColorEditState
            {
                Vector<i32> rgb;
                Vector<i32> hsv;
                i32 axis = 0;
                Float4U original = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
                Float2U popup_position = Float2U(0.0f);
                u32 active_part = 0;
                bool original_valid = false;
                bool initialized = false;
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
                Vector<GUI::id_t> header_ids;
                Vector<String> header_labels;
            };

            struct [[Luna::struct("{2F3F1D61-3E0D-4D1C-8B0A-C98CAC4C91B9}")]] TableState
            {
                Vector<f32> column_widths;
                Vector<f32> column_source_widths;
                u32 inferred_column_count = 0;
                i32 resizing_column = -1;
                f32 resize_start_pointer_x = 0.0f;
                f32 resize_start_width = 0.0f;
                f32 visible_min_y = 0.0f;
                f32 visible_max_y = 0.0f;
                u32 layout_generation = U32_MAX;
            };

            struct [[Luna::struct("{7A960E14-A855-4E01-97E1-A32ECC2518D9}")]] PopupState
            {
                bool open = false;
                PopupFlag flags = PopupFlag::none;
            };

            struct [[Luna::struct("{A93C2B07-43B7-4627-88E1-FDD26DF88FFF}")]] TooltipState
            {
                id_t hovered_owner = 0;
                f32 hover_time = 0.0f;
            };

            struct [[Luna::struct("{FF49D59C-89E8-4F2D-893F-93FB877335C3}")]] PopupPlacementState
            {
                Float2U position = Float2U(0.0f);
            };

            struct [[Luna::struct("{681AD6CF-46C0-442C-827F-B24E40571FD5}")]] MenuBarState
            {
                id_t active_popup = 0;
            };
        }
    }
}
