/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Description.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Layout.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class GUINodeKind : u8
        {
            root,
            v_layout,
            h_layout,
            scroll_view,
            window,
            table_layout,
            text,
            button,
            checkbox,
            input_text,
            image,
            collapsing_header,
            combo,
            slider_float,
            drag_float
        };

        struct GUINode
        {
            GUIID id = 0;
            GUINodeKind kind = GUINodeKind::root;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            String text;
            Ref<RHI::ITexture> texture;
            GUISize requested_size;
            GUILayoutStyle layout_style;
            GUILayoutDesc layout_desc;
            GUITableDesc table_desc;
            bool has_table_cell_color = false;
            Float4U table_cell_color = Float4U(0.0f);
            bool* bool_value = nullptr;
            String* string_value = nullptr;
            i32* i32_value = nullptr;
            f32* f32_value = nullptr;
            f32 min_value = 0.0f;
            f32 max_value = 0.0f;
            f32 step_value = 0.0f;
            Vector<String> items;
            bool interactive = false;
        };

        struct GUIDescription
        {
            u64 generation = 0;
            Vector<GUINode> nodes;
        };
    }
}
