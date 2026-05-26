/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.hpp
* @author JXMaster
* @date 2026/5/21
*/
#pragma once
#include "../GUI.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace GUI
    {
        struct GUIContext;

        extern GUIContext* g_current_context;

        inline constexpr u64 FNV_OFFSET = 14695981039346656037ull;
        inline constexpr u64 FNV_PRIME = 1099511628211ull;

        inline u64 hash_bytes(const void* data, usize size, u64 h = FNV_OFFSET)
        {
            const byte_t* p = (const byte_t*)data;
            for(usize i = 0; i < size; ++i)
            {
                h ^= (u64)p[i];
                h *= FNV_PRIME;
            }
            return h;
        }

        inline u64 hash_cstr(const c8* str, u64 h)
        {
            if(!str) return h;
            while(*str)
            {
                h ^= (u64)(byte_t)*str;
                h *= FNV_PRIME;
                ++str;
            }
            return h;
        }

        inline u64 hash_u64(u64 value, u64 h = FNV_OFFSET)
        {
            return hash_bytes(&value, sizeof(value), h);
        }

        inline bool point_in_rect(const Float2U& p, const RectF& r)
        {
            return p.x >= r.offset_x && p.y >= r.offset_y &&
                p.x < r.offset_x + r.width && p.y < r.offset_y + r.height;
        }

        inline RectF intersect_rect(const RectF& a, const RectF& b)
        {
            f32 min_x = max(a.offset_x, b.offset_x);
            f32 min_y = max(a.offset_y, b.offset_y);
            f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
            f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
            return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
        }

        inline f32 axis_value(const Float2U& value, bool x_axis)
        {
            return x_axis ? value.x : value.y;
        }

        inline void set_axis_value(Float2U& value, bool x_axis, f32 axis_value)
        {
            if(x_axis) value.x = axis_value;
            else value.y = axis_value;
        }

        inline GUISizePolicy axis_policy(const GUILayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.width_policy : style.height_policy;
        }

        inline f32 axis_fixed_size(const GUILayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.fixed_width_value : style.fixed_height_value;
        }

        inline f32 axis_fill_weight(const GUILayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.fill_weight_x : style.fill_weight_y;
        }

        inline u32 table_columns(const GUINode& node)
        {
            return max(node.table_desc.columns, 1u);
        }

        inline u32 table_child_count(const GUIDescription& desc, const GUINode& node)
        {
            u32 ret = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = desc.nodes[child].next_sibling)
            {
                ++ret;
            }
            return ret;
        }

        inline u32 table_rows(const GUIDescription& desc, const GUINode& node)
        {
            u32 columns = table_columns(node);
            u32 child_count = table_child_count(desc, node);
            return max((child_count + columns - 1) / columns, (u32)node.table_desc.row_sizes.size());
        }

        inline const GUITableTrackSize& table_track_size(const GUINode& node, bool column, u32 index)
        {
            static GUITableTrackSize default_size;
            const Vector<GUITableTrackSize>& sizes = column ? node.table_desc.column_sizes : node.table_desc.row_sizes;
            return index < sizes.size() ? sizes[index] : default_size;
        }

        inline bool table_track_is_fixed(const GUINode& node, bool column, u32 index)
        {
            return table_track_size(node, column, index).policy == GUITableTrackSizePolicy::fixed;
        }

        inline bool window_has_title_bar(const GUINode& node)
        {
            return node.kind == GUINodeKind::window && node.bool_value;
        }

        inline f32 window_title_bar_height()
        {
            return 30.0f;
        }

        inline u32 f32_value_count(const GUINode& node)
        {
            return clamp((u32)node.f32_value_count, 1u, 4u);
        }

        inline f32 combo_label_width(const GUINode& node, const RectF& rect)
        {
            return min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
        }

        inline RectF combo_value_rect(const GUINode& node, const RectF& rect)
        {
            f32 label_w = combo_label_width(node, rect);
            return RectF(rect.offset_x + label_w, rect.offset_y, max(rect.width - label_w, 1.0f), rect.height);
        }

        inline bool radio_button_selected(const GUINode& node)
        {
            if(node.i32_value) return *node.i32_value == node.item_value;
            if(node.bool_value) return *node.bool_value;
            return node.selected;
        }

        inline RectF button_group_item_rect(const GUINode& node, const RectF& rect, u32 index)
        {
            u32 count = max((u32)node.items.size(), 1u);
            f32 item_width = rect.width / (f32)count;
            f32 x = rect.offset_x + item_width * (f32)index;
            f32 w = index + 1 == count ? max(rect.offset_x + rect.width - x, 1.0f) : max(item_width, 1.0f);
            return RectF(x, rect.offset_y, w, rect.height);
        }

        inline i32 button_group_item_at(const GUINode& node, const RectF& rect, const Float2U& pos)
        {
            u32 count = (u32)node.items.size();
            if(!count || !point_in_rect(pos, rect)) return -1;
            f32 item_width = max(rect.width / (f32)count, 1.0f);
            i32 index = (i32)((pos.x - rect.offset_x) / item_width);
            return index >= 0 && (u32)index < count ? index : (i32)count - 1;
        }

        inline f32 combo_item_height()
        {
            return 26.0f;
        }

        inline RectF combo_dropdown_rect(const GUINode& node, const RectF& rect, const Float2U& surface_size)
        {
            RectF value = combo_value_rect(node, rect);
            f32 item_height = combo_item_height();
            f32 dropdown_width = max(value.width, 120.0f);
            f32 dropdown_height = max((f32)node.items.size() * item_height, item_height);
            dropdown_width = min(dropdown_width, max(surface_size.x, 1.0f));
            dropdown_height = min(dropdown_height, max(surface_size.y, item_height));
            f32 x = min(value.offset_x, max(surface_size.x - dropdown_width, 0.0f));
            f32 y = value.offset_y + value.height + 2.0f;
            if(y + dropdown_height > surface_size.y && value.offset_y - dropdown_height - 2.0f >= 0.0f)
            {
                y = value.offset_y - dropdown_height - 2.0f;
            }
            y = min(y, max(surface_size.y - dropdown_height, 0.0f));
            return RectF(x, y, dropdown_width, dropdown_height);
        }

        inline i32 combo_dropdown_item_at(const GUINode& node, const RectF& dropdown_rect, const Float2U& pos)
        {
            if(!point_in_rect(pos, dropdown_rect) || node.items.empty()) return -1;
            i32 index = (i32)((pos.y - dropdown_rect.offset_y) / combo_item_height());
            return index >= 0 && (usize)index < node.items.size() ? index : -1;
        }

        inline bool is_absolute_node(const GUINode& node)
        {
            return node.absolute_position || node.kind == GUINodeKind::popup || node.kind == GUINodeKind::tooltip;
        }

        inline bool is_overlay_node(const GUINode& node)
        {
            return node.render_layer == GUIRenderLayer::overlay;
        }

        inline bool contains_name(const Vector<Name>& names, const Name& name)
        {
            for(const Name& item : names)
            {
                if(item == name) return true;
            }
            return false;
        }

        inline bool tree_node_is_leaf(const GUINode& node)
        {
            return test_flags(node.tree_flags, GUITreeNodeFlag::leaf);
        }

        inline f32 tree_node_indent_width()
        {
            return 18.0f;
        }

        inline RectF tree_node_arrow_rect(const GUINode& node, const RectF& rect)
        {
            f32 x = rect.offset_x + 4.0f + tree_node_indent_width() * (f32)node.tree_depth;
            f32 y = rect.offset_y + max((rect.height - 18.0f) * 0.5f, 0.0f);
            return RectF(x, y, 18.0f, 18.0f);
        }

        inline f32 tab_bar_header_height()
        {
            return 32.0f;
        }

        inline f32 tab_item_min_width()
        {
            return 56.0f;
        }

        inline f32 tab_scroll_button_size()
        {
            return 24.0f;
        }

        inline f32 tab_item_ideal_width(const GUINode& node)
        {
            f32 font_size = 15.0f;
            f32 text_width = (f32)node.text.size() * font_size * 0.52f;
            f32 close_width = (node.bool_value && !test_flags(node.tab_item_flags, GUITabItemFlag::no_close_button)) ? 22.0f : 0.0f;
            f32 unsaved_width = test_flags(node.tab_item_flags, GUITabItemFlag::unsaved_document) ? 12.0f : 0.0f;
            return max(text_width + 24.0f + close_width + unsaved_width, tab_item_min_width());
        }

        inline RectF window_close_rect(const RectF& window_rect)
        {
            f32 size = 22.0f;
            return RectF(
                window_rect.offset_x + max(window_rect.width - size - 4.0f, 0.0f),
                window_rect.offset_y + 4.0f,
                size,
                size);
        }

        inline f32 resolve_base_axis_size(const GUINode& node, const GUILayoutMetrics& metrics, bool x_axis)
        {
            if(axis_policy(node.layout_style, x_axis) == GUISizePolicy::fixed)
            {
                return clamp(axis_fixed_size(node.layout_style, x_axis),
                    axis_value(metrics.min_size, x_axis),
                    axis_value(metrics.max_size, x_axis));
            }
            return axis_value(metrics.preferred_size, x_axis);
        }

        inline GUILayoutMetrics apply_layout_style(const GUINode& node, GUILayoutMetrics metrics)
        {
            for(u32 axis = 0; axis < 2; ++axis)
            {
                bool x_axis = axis == 0;
                f32 min_size = max(axis_value(metrics.min_size, x_axis), axis_value(node.layout_style.min_size, x_axis));
                f32 max_size = min(axis_value(metrics.max_size, x_axis), axis_value(node.layout_style.max_size, x_axis));
                if(max_size < min_size) max_size = min_size;
                f32 preferred_size = clamp(axis_value(metrics.preferred_size, x_axis), min_size, max_size);
                if(axis_policy(node.layout_style, x_axis) == GUISizePolicy::fixed)
                {
                    preferred_size = clamp(axis_fixed_size(node.layout_style, x_axis),
                        axis_value(node.layout_style.min_size, x_axis),
                        axis_value(node.layout_style.max_size, x_axis));
                    min_size = preferred_size;
                    max_size = preferred_size;
                }
                set_axis_value(metrics.min_size, x_axis, min_size);
                set_axis_value(metrics.preferred_size, x_axis, preferred_size);
                set_axis_value(metrics.max_size, x_axis, max_size);
            }
            return metrics;
        }

        inline GUILayoutStyle default_layout_style(GUINodeKind kind)
        {
            GUILayoutStyle style;
            if(kind == GUINodeKind::input_text ||
                kind == GUINodeKind::combo ||
                kind == GUINodeKind::slider_float ||
                kind == GUINodeKind::drag_float ||
                kind == GUINodeKind::menu_bar ||
                kind == GUINodeKind::menu_item ||
                kind == GUINodeKind::menu_separator ||
                kind == GUINodeKind::selectable ||
                kind == GUINodeKind::tree_node ||
                kind == GUINodeKind::table_layout ||
                kind == GUINodeKind::dock_space ||
                kind == GUINodeKind::tab_bar)
            {
                style.width_policy = GUISizePolicy::fill;
            }
            if(kind == GUINodeKind::dock_space || kind == GUINodeKind::tab_bar)
            {
                style.height_policy = GUISizePolicy::fill;
            }
            return style;
        }

        inline f32 menu_bar_height()
        {
            return 30.0f;
        }

        inline f32 menu_item_height()
        {
            return 26.0f;
        }

        inline f32 menu_separator_height()
        {
            return 7.0f;
        }

        inline f32 menu_text_width(const String& text, f32 font_size = 15.0f)
        {
            return (f32)text.size() * font_size * 0.52f;
        }

        inline void apply_requested_size(GUINode& node, const GUISize& size)
        {
            node.requested_size = size;
            if(size.width > 0.0f)
            {
                node.layout_style.width_policy = GUISizePolicy::fixed;
                node.layout_style.fixed_width_value = size.width;
            }
            if(size.height > 0.0f)
            {
                node.layout_style.height_policy = GUISizePolicy::fixed;
                node.layout_style.fixed_height_value = size.height;
            }
        }

        inline void pop_utf8_codepoint(String& value)
        {
            usize size = value.size();
            if(!size) return;
            usize begin = size - 1;
            while(begin > 0 && (((u8)value[begin]) & 0xC0) == 0x80)
            {
                --begin;
            }
            value.erase(begin, size - begin);
        }

        inline usize clamp_utf8_cursor(const String& value, usize cursor)
        {
            if(cursor == USIZE_MAX || cursor > value.size())
            {
                return value.size();
            }
            while(cursor > 0 && cursor < value.size() && (((u8)value[cursor]) & 0xC0) == 0x80)
            {
                --cursor;
            }
            return cursor;
        }

        inline usize previous_utf8_cursor(const String& value, usize cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(!cursor) return 0;
            --cursor;
            while(cursor > 0 && (((u8)value[cursor]) & 0xC0) == 0x80)
            {
                --cursor;
            }
            return cursor;
        }

        inline usize next_utf8_cursor(const String& value, usize cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(cursor >= value.size()) return value.size();
            usize len = utf8_charlen(value.c_str() + cursor);
            return min(cursor + len, value.size());
        }

        inline void erase_previous_utf8_codepoint(String& value, usize& cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            usize begin = previous_utf8_cursor(value, cursor);
            if(begin == cursor) return;
            value.erase(begin, cursor - begin);
            cursor = begin;
        }

        inline void erase_utf8_codepoint_at(String& value, usize& cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(cursor >= value.size()) return;
            usize end = next_utf8_cursor(value, cursor);
            value.erase(cursor, end - cursor);
        }

        inline VG::TextArrangeResult arrange_input_text_for_cursor(const String& value, f32 font_size)
        {
            VG::TextArrangeSection section;
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            section.font_size = font_size;
            section.num_chars = value.size();
            return VG::arrange_text(value.c_str(), value.size(), {&section, 1},
                RectF(0.0f, 0.0f, 1000000.0f, font_size * 2.0f),
                VG::TextAlignment::center, VG::TextAlignment::begin);
        }

        inline f32 measure_input_text_width(const String& value, usize bytes, f32 font_size)
        {
            bytes = clamp_utf8_cursor(value, bytes);
            if(!bytes) return 0.0f;
            String view(value.c_str(), bytes);
            VG::TextArrangeResult arranged = arrange_input_text_for_cursor(view, font_size);
            return arranged.bounding_rect.width;
        }

        inline f32 input_text_cursor_x(const String& value, usize cursor, f32 font_size)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            return measure_input_text_width(value, cursor, font_size);
        }

        inline usize input_text_cursor_from_x(const String& value, f32 x, f32 font_size)
        {
            if(x <= 0.0f) return 0;
            VG::TextArrangeResult arranged = arrange_input_text_for_cursor(value, font_size);
            if(arranged.lines.empty()) return value.size();
            const VG::TextLineArrangeResult& line = arranged.lines[0];
            if(line.glyphs.empty()) return value.size();
            for(usize i = 0; i < line.glyphs.size(); ++i)
            {
                const VG::TextGlyphArrangeResult& glyph = line.glyphs[i];
                f32 next_origin = i + 1 < line.glyphs.size() ?
                    line.glyphs[i + 1].origin_offset :
                    glyph.origin_offset + glyph.advance_length;
                f32 threshold = (glyph.origin_offset + next_origin) * 0.5f;
                if(x < threshold)
                {
                    return glyph.index;
                }
            }
            return value.size();
        }

        inline bool has_modifier(GUIKeyModifierFlag flags, GUIKeyModifierFlag flag)
        {
            return (((u8)flags) & ((u8)flag)) != 0;
        }

        struct GUIIDHash
        {
            usize operator()(GUIID value) const
            {
                return (usize)value;
            }
        };

        struct DockPanelPersistentState
        {
            bool initialized = false;
            bool closed = false;
            GUIDockPanelMode mode = GUIDockPanelMode::docking;
            RectF rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
            f32 docking_height = 0.0f;
            u32 z_order = 0;
        };

        enum class GUIDockSplitAxis : u8
        {
            x,
            y
        };

        enum class GUIDockDropDirection : u8
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
            bool split = false;
            u32 parent = U32_MAX;
            u32 child0 = U32_MAX;
            u32 child1 = U32_MAX;
            GUIDockSplitAxis split_axis = GUIDockSplitAxis::x;
            f32 split_ratio = 0.5f;
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF split_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Vector<GUIID> tabs;
            GUIID selected_tab = 0;
        };

        struct ItemResult
        {
            u64 generation = 0;
            HashMap<Name, Any> states;
        };

        struct DragDropPayloadStorage
        {
            Name type;
            Vector<byte_t> data;
            GUIItemHandle source;
            GUIItemHandle target;
            bool preview = false;
            bool delivery = false;
        };

        struct DragDropTargetScope
        {
            GUIItemHandle target;
            Name type;
        };

        struct PopupStackEntry
        {
            GUIID id = 0;
            GUIID parent_id = 0;
            GUIPopupFlag flags = GUIPopupFlag::none;
        };

        struct PersistentItemState
        {
            bool open = true;
            bool open_initialized = false;
            bool active = false;
            bool focused = false;
            bool pointer_down = false;
            f32 scroll_x = 0.0f;
            f32 scroll_y = 0.0f;
            f32 scrollbar_opacity = 0.35f;
            f32 switch_animation = 0.0f;
            bool switch_animation_initialized = false;
            f32 button_group_selection_animation = 0.0f;
            bool button_group_selection_animation_initialized = false;
            Vector<f32> button_group_item_animations;
            u32 dock_next_z_order = 1;
            HashMap<GUIID, DockPanelPersistentState, GUIIDHash> dock_panels;
            Vector<DockTreeNode> dock_nodes;
            u32 dock_root_node = U32_MAX;
            f64 last_click_time = -1000.0;
            f64 last_right_click_time = -1000.0;
            usize text_cursor = USIZE_MAX;
            usize text_select_anchor = USIZE_MAX;
            bool text_selecting = false;
            f64 text_cursor_blink_start = 0.0;
            Vector<f32> table_column_sizes;
            Vector<f32> table_row_sizes;
            GUIID tab_selected_id = 0;
            f32 tab_scroll_x = 0.0f;
            Vector<GUIID> tab_order;
        };

        inline bool tab_order_contains(const PersistentItemState& state, GUIID id)
        {
            for(GUIID item : state.tab_order)
            {
                if(item == id) return true;
            }
            return false;
        }

        struct TabBuildScope
        {
            GUIID tab_bar_id = 0;
            GUIID selected_id = 0;
            GUIID first_open_id = 0;
            GUITabBarFlag flags = GUITabBarFlag::none;
            bool had_existing_tabs = false;
            bool visible_tab_chosen = false;
        };

        inline void input_text_selection_range(const String& value, const PersistentItemState& state, usize& out_begin, usize& out_end)
        {
            usize cursor = clamp_utf8_cursor(value, state.text_cursor);
            usize anchor = state.text_select_anchor == USIZE_MAX ? cursor : clamp_utf8_cursor(value, state.text_select_anchor);
            out_begin = min(cursor, anchor);
            out_end = max(cursor, anchor);
        }

        inline bool input_text_has_selection(const String& value, const PersistentItemState& state)
        {
            usize begin = 0;
            usize end = 0;
            input_text_selection_range(value, state, begin, end);
            return begin != end;
        }

        inline void input_text_clear_selection(PersistentItemState& state)
        {
            state.text_select_anchor = USIZE_MAX;
            state.text_selecting = false;
        }

        struct NodeLayout
        {
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            GUILayoutMetrics metrics;
            bool metrics_valid = false;
            Vector<f32> table_column_offsets;
            Vector<f32> table_column_widths;
            Vector<f32> table_row_offsets;
            Vector<f32> table_row_heights;
            u32 table_columns = 0;
            u32 table_rows = 0;
            RectF tab_header_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_header_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_header_area_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool tab_scrollable = false;
            f32 tab_scroll_max = 0.0f;
            bool tab_content_visible = true;
            Float2U scroll_content_size = Float2U(0.0f);
            Float2U scroll_viewport_size = Float2U(0.0f);
            bool scroll_has_vertical = false;
            bool scroll_has_horizontal = false;
            bool dock_panel_child = false;
            bool dock_panel_visible = true;
            bool dock_panel_floating = false;
            GUIID dock_space_id = 0;
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            GUIDockPanelStyle dock_panel_style;
            u32 dock_panel_z_order = 0;
            u32 dock_leaf_index = U32_MAX;
        };

        inline RectF dock_panel_content_rect(const RectF& panel_rect, const GUIDockPanelStyle& style)
        {
            f32 border = max(style.border_size, 0.0f);
            f32 title_height = style.title_bar ? max(style.title_bar_height, 0.0f) : 0.0f;
            return RectF(
                panel_rect.offset_x + border,
                panel_rect.offset_y + border + title_height,
                max(panel_rect.width - border * 2.0f, 1.0f),
                max(panel_rect.height - border * 2.0f - title_height, 1.0f));
        }

        inline RectF dock_panel_title_rect(const RectF& panel_rect, const GUIDockPanelStyle& style)
        {
            f32 border = max(style.border_size, 0.0f);
            return RectF(
                panel_rect.offset_x + border,
                panel_rect.offset_y + border,
                max(panel_rect.width - border * 2.0f, 1.0f),
                max(style.title_bar_height, 1.0f));
        }

        inline RectF dock_panel_close_rect(const RectF& title_rect)
        {
            f32 size = 20.0f;
            return RectF(
                title_rect.offset_x + max(title_rect.width - size - 4.0f, 0.0f),
                title_rect.offset_y + max((title_rect.height - size) * 0.5f, 0.0f),
                size,
                size);
        }

        inline RectF dock_panel_resize_rect(const RectF& panel_rect, const GUIDockPanelStyle& style)
        {
            f32 size = max(style.resize_border_size, 1.0f);
            return RectF(
                panel_rect.offset_x + max(panel_rect.width - size, 0.0f),
                panel_rect.offset_y + max(panel_rect.height - size, 0.0f),
                size,
                size);
        }

        inline RectF dock_panel_docked_resize_rect(const RectF& panel_rect, const GUIDockPanelStyle& style)
        {
            f32 size = max(style.resize_border_size, 4.0f);
            return RectF(
                panel_rect.offset_x,
                panel_rect.offset_y + max(panel_rect.height - size * 0.5f, 0.0f),
                max(panel_rect.width, 1.0f),
                size);
        }

        inline f32 dock_panel_min_height(const GUIDockPanelStyle& style)
        {
            f32 chrome_height = max(style.border_size, 0.0f) * 2.0f + (style.title_bar ? max(style.title_bar_height, 0.0f) : 0.0f);
            return max(max(style.min_floating_size.y, chrome_height + 24.0f), 32.0f);
        }

        inline f32 dock_panel_splitter_size()
        {
            return 6.0f;
        }

        inline RectF dock_panel_tab_rect(const RectF& title_rect, usize tab_index, usize tab_count, bool has_close_button)
        {
            f32 right_reserved = has_close_button ? 28.0f : 4.0f;
            f32 tab_area_width = max(title_rect.width - right_reserved - 4.0f, 1.0f);
            f32 tab_width = max(tab_area_width / (f32)max(tab_count, (usize)1), 48.0f);
            return RectF(
                title_rect.offset_x + 4.0f + tab_width * (f32)tab_index,
                title_rect.offset_y + 3.0f,
                min(tab_width, max(tab_area_width - tab_width * (f32)tab_index, 1.0f)),
                max(title_rect.height - 4.0f, 1.0f));
        }

        inline RectF dock_drop_icon_rect(const RectF& parent, GUIDockDropDirection direction)
        {
            f32 shorter = min(parent.width, parent.height);
            f32 center_size = clamp(shorter * 0.12f, 22.0f, 36.0f);
            f32 side_w = center_size * 1.2f;
            f32 side_h = center_size * 0.78f;
            f32 offset = center_size * 1.95f;
            f32 cx = parent.offset_x + parent.width * 0.5f;
            f32 cy = parent.offset_y + parent.height * 0.5f;
            switch(direction)
            {
            case GUIDockDropDirection::center:
                return RectF(cx - center_size * 0.5f, cy - center_size * 0.5f, center_size, center_size);
            case GUIDockDropDirection::left:
                return RectF(cx - offset - side_h * 0.5f, cy - side_w * 0.5f, side_h, side_w);
            case GUIDockDropDirection::right:
                return RectF(cx + offset - side_h * 0.5f, cy - side_w * 0.5f, side_h, side_w);
            case GUIDockDropDirection::up:
                return RectF(cx - side_w * 0.5f, cy - offset - side_h * 0.5f, side_w, side_h);
            case GUIDockDropDirection::down:
                return RectF(cx - side_w * 0.5f, cy + offset - side_h * 0.5f, side_w, side_h);
            default:
                return RectF(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        inline RectF dock_drop_preview_rect(const RectF& parent, GUIDockDropDirection direction)
        {
            switch(direction)
            {
            case GUIDockDropDirection::center:
                return dock_panel_content_rect(parent, GUIDockPanelStyle());
            case GUIDockDropDirection::left:
                return RectF(parent.offset_x, parent.offset_y, parent.width * 0.5f, parent.height);
            case GUIDockDropDirection::right:
                return RectF(parent.offset_x + parent.width * 0.5f, parent.offset_y, parent.width * 0.5f, parent.height);
            case GUIDockDropDirection::up:
                return RectF(parent.offset_x, parent.offset_y, parent.width, parent.height * 0.5f);
            case GUIDockDropDirection::down:
                return RectF(parent.offset_x, parent.offset_y + parent.height * 0.5f, parent.width, parent.height * 0.5f);
            default:
                return RectF(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        inline f32 scroll_bar_size()
        {
            return 10.0f;
        }

        inline f32 scroll_bar_margin()
        {
            return 3.0f;
        }

        inline f32 scroll_bar_padding()
        {
            return scroll_bar_size() + scroll_bar_margin() * 2.0f;
        }

        inline f32 scroll_min_thumb_size()
        {
            return 24.0f;
        }

        inline f32 scroll_max_x(const NodeLayout& layout)
        {
            return max(layout.scroll_content_size.x - layout.scroll_viewport_size.x, 0.0f);
        }

        inline f32 scroll_max_y(const NodeLayout& layout)
        {
            return max(layout.scroll_content_size.y - layout.scroll_viewport_size.y, 0.0f);
        }

        inline bool scroll_has_vertical_bar(const NodeLayout& layout)
        {
            return layout.scroll_has_vertical && scroll_max_y(layout) > 0.0f;
        }

        inline bool scroll_has_horizontal_bar(const NodeLayout& layout)
        {
            return layout.scroll_has_horizontal && scroll_max_x(layout) > 0.0f;
        }

        inline RectF scroll_vertical_track_rect(const NodeLayout& layout)
        {
            f32 size = scroll_bar_size();
            f32 margin = scroll_bar_margin();
            f32 bottom_reserved = scroll_has_horizontal_bar(layout) ? size + margin : 0.0f;
            return RectF(
                layout.rect.offset_x + max(layout.rect.width - size - margin, 0.0f),
                layout.rect.offset_y + margin,
                size,
                max(layout.rect.height - margin * 2.0f - bottom_reserved, 1.0f));
        }

        inline RectF scroll_horizontal_track_rect(const NodeLayout& layout)
        {
            f32 size = scroll_bar_size();
            f32 margin = scroll_bar_margin();
            f32 right_reserved = scroll_has_vertical_bar(layout) ? size + margin : 0.0f;
            return RectF(
                layout.rect.offset_x + margin,
                layout.rect.offset_y + max(layout.rect.height - size - margin, 0.0f),
                max(layout.rect.width - margin * 2.0f - right_reserved, 1.0f),
                size);
        }

        inline RectF scroll_vertical_thumb_rect(const NodeLayout& layout, const PersistentItemState& state)
        {
            RectF track = scroll_vertical_track_rect(layout);
            f32 ratio = layout.scroll_content_size.y > 0.0f ? clamp(layout.scroll_viewport_size.y / layout.scroll_content_size.y, 0.0f, 1.0f) : 1.0f;
            f32 thumb_height = min(max(track.height * ratio, min(scroll_min_thumb_size(), track.height)), track.height);
            f32 travel = max(track.height - thumb_height, 0.0f);
            f32 t = scroll_max_y(layout) > 0.0f ? clamp(state.scroll_y / scroll_max_y(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x, track.offset_y + travel * t, track.width, thumb_height);
        }

        inline RectF scroll_horizontal_thumb_rect(const NodeLayout& layout, const PersistentItemState& state)
        {
            RectF track = scroll_horizontal_track_rect(layout);
            f32 ratio = layout.scroll_content_size.x > 0.0f ? clamp(layout.scroll_viewport_size.x / layout.scroll_content_size.x, 0.0f, 1.0f) : 1.0f;
            f32 thumb_width = min(max(track.width * ratio, min(scroll_min_thumb_size(), track.width)), track.width);
            f32 travel = max(track.width - thumb_width, 0.0f);
            f32 t = scroll_max_x(layout) > 0.0f ? clamp(state.scroll_x / scroll_max_x(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x + travel * t, track.offset_y, thumb_width, track.height);
        }

        struct GUIContext : IGUIContext
        {
            lustruct("GUI::GUIContext", "{BF721C36-C7C2-4B49-89E6-22F0B3BE56F5}");
            luiimpl();
            lutsassert_lock();

            Ref<RHI::IDevice> m_device;
            GUIFrameDesc m_frame_desc;
            GUIDescription m_build_desc;
            GUIDescription m_submitted_desc;
            Vector<NodeLayout> m_layouts;
            Vector<GUIInputEvent> m_input_events;
            Vector<u32> m_parent_stack;
            Vector<GUIID> m_id_stack;
            Vector<RectF> m_clip_stack;
            Vector<u32> m_child_ordinals;
            HashMap<GUIID, ItemResult, GUIIDHash> m_last_results;
            HashMap<GUIID, ItemResult, GUIIDHash> m_current_results;
            HashMap<GUIID, PersistentItemState, GUIIDHash> m_persistent_states;
            GUIClipboardIO m_clipboard_io;
            GUIID m_active_id = 0;
            GUIID m_focused_id = 0;
            GUIID m_hovered_id = 0;
            Float2U m_pointer_pos = Float2U(0.0f);
            bool m_pointer_inside = false;
            u32 m_active_float_component = U32_MAX;
            bool m_submitted = false;
            bool m_has_next_item_layout = false;
            GUILayoutStyle m_next_item_layout;
            bool m_has_next_table_cell_color = false;
            Float4U m_next_table_cell_color = Float4U(0.0f);
            bool m_has_next_dock_panel_style = false;
            GUIDockPanelStyle m_next_dock_panel_style;
            bool* m_next_dock_panel_open = nullptr;
            Vector<TabBuildScope> m_tab_build_stack;
            GUIID m_last_item_id = 0;
            u32 m_tree_depth = 0;
            bool m_layout_dirty = false;
            GUIID m_active_table_resize_id = 0;
            bool m_active_table_resize_column = false;
            u32 m_active_table_resize_index = U32_MAX;
            GUIID m_active_scrollbar_id = 0;
            bool m_active_scrollbar_vertical = false;
            f32 m_active_scrollbar_grab_offset = 0.0f;
            GUIID m_active_dock_space_id = 0;
            GUIID m_active_dock_panel_id = 0;
            bool m_active_dock_panel_resize = false;
            bool m_active_dock_panel_close = false;
            bool m_active_dock_panel_was_floating = false;
            bool m_active_dock_panel_title_drag = false;
            bool m_active_dock_panel_undocked = false;
            GUIID m_active_dock_panel_resize_neighbor_id = 0;
            Float2U m_active_dock_panel_grab_offset = Float2U(0.0f);
            RectF m_active_dock_panel_start_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF m_active_dock_panel_restore_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF m_active_dock_panel_start_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            f32 m_active_dock_panel_start_neighbor_height = 0.0f;
            GUIID m_active_dock_split_space_id = 0;
            u32 m_active_dock_split_node = U32_MAX;
            GUIDockSplitAxis m_active_dock_split_axis = GUIDockSplitAxis::x;
            f32 m_active_dock_split_start_ratio = 0.5f;
            Float2U m_active_dock_split_start_pos = Float2U(0.0f);
            GUIID m_open_combo_id = 0;
            Vector<PopupStackEntry> m_open_popup_stack;
            Vector<GUIID> m_popup_build_stack;
            HashMap<GUIID, u32, GUIIDHash> m_popup_node_indices;
            GUIID m_active_tab_bar_id = 0;
            GUIID m_active_tab_item_id = 0;
            bool m_active_tab_close = false;
            bool m_active_tab_reorder_allowed = false;
            bool m_active_tab_reordering = false;
            Float2U m_active_tab_start_pos = Float2U(0.0f);
            GUIID m_active_tab_scroll_id = 0;
            bool m_active_tab_scroll_left = false;
            GUIID m_drag_drop_candidate_source_id = 0;
            Name m_drag_drop_candidate_type;
            Float2U m_drag_drop_start_pos = Float2U(0.0f);
            bool m_drag_drop_active = false;
            bool m_drag_drop_payload_set = false;
            bool m_drag_drop_preview_built = false;
            GUIID m_drag_drop_source_id = 0;
            Name m_drag_drop_type;
            Vector<byte_t> m_drag_drop_payload_data;
            HashMap<GUIID, DragDropPayloadStorage, GUIIDHash> m_last_drag_drop_deliveries;
            HashMap<GUIID, DragDropPayloadStorage, GUIIDHash> m_current_drag_drop_deliveries;
            Vector<DragDropTargetScope> m_drag_drop_target_stack;
            GUIDragDropPayload m_drag_drop_payload_view;
            GUIID m_tooltip_hovered_id = 0;
            f64 m_tooltip_hover_start = 0.0;
            u64 m_generation = 0;
            f64 m_time = 0.0;
            Ref<VG::IShapeDrawList> m_shape_draw_list;
            Ref<IDrawList> m_main_draw_list;
            Ref<IDrawList> m_overlay_draw_list;
            IDrawList* m_active_draw_list = nullptr;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;

            GUIContext();

            virtual void begin_frame(const GUIFrameDesc& desc) override;
            virtual void add_input_event(const GUIInputEvent& event) override;
            virtual void add_input_events(Span<const GUIInputEvent> events) override;
            virtual R<GUIDescription> end_build() override;
            virtual RV submit(const GUIDescription& desc) override;
            virtual void set_clipboard_io(const GUIClipboardIO& io) override;
            virtual GUITextInputState get_text_input_state() override;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) override;

            GUIItemHandle add_node(GUINodeKind kind, const c8* text, bool interactive);
            void begin_container(GUINodeKind kind, const c8* label, const GUISize& size, GUIItemHandle* out_handle);
            void end_container();
            GUIItemHandle begin_popup(const c8* label, const GUIPopupDesc& desc);
            void end_popup();
            void open_popup(GUIItemHandle popup);
            void close_popup(GUIItemHandle popup);
            void close_current_popup();
            void close_all_popups();
            bool is_popup_open(GUIItemHandle popup) const;
            bool is_popup_open(GUIID id) const;
            bool popup_node_visible(const GUINode& node) const;
            bool tooltip_node_visible(const GUINode& node) const;
            u32 find_submitted_node_index(GUIID id) const;
            void rebuild_popup_node_indices();
            void prune_popup_stack();
            void close_popup_stack_from(usize index);
            i32 popup_stack_index(GUIID id) const;
            i32 popup_level_at_pos(const Float2U& pos) const;
            bool close_popups_for_pointer_down(const Float2U& pos);
            void open_menu_popup(GUIID menu_id);
            void update_menu_hover();
            GUIItemHandle begin_tooltip(GUIItemHandle owner, const c8* label, const GUITooltipDesc& desc);
            void end_tooltip();
            const Any* get_state(GUIItemHandle handle, const Name& key);
            void set_state(GUIItemHandle handle, const Name& key, const Any& value);
            void remove_state(GUIItemHandle handle, const Name& key);
            void set_next_item_layout(const GUILayoutStyle& style);
            void set_next_table_cell_color(const Float4U& color);
            void set_next_dock_panel_style(const GUIDockPanelStyle& style, bool* open);
            void push_id(GUIID id);
            void pop_id();
            void push_clip_rect(const RectF& rect);
            void pop_clip_rect();
            void tree_push();
            void tree_pop();
            bool begin_drag_drop_source(GUIItemHandle source, const Name& payload_type);
            void set_drag_drop_payload(const void* data, usize data_size);
            void end_drag_drop_source();
            bool begin_drag_drop_target(GUIItemHandle target, const Name& payload_type);
            const GUIDragDropPayload* accept_drag_drop_payload(const Name& payload_type);
            const GUIDragDropPayload* accept_drag_drop_payload(GUIItemHandle target, const Name& payload_type);
            void end_drag_drop_target();
            bool is_drag_drop_active() const;
            const GUIDragDropPayload* get_drag_drop_payload();
            const GUIDragDropPayload* make_drag_drop_payload_view(const DragDropPayloadStorage& storage);

            ItemResult* get_query_result(GUIItemHandle handle);
            ItemResult& get_or_create_current_result(GUIID id);
            PersistentItemState& get_or_create_persistent_state(GUIID id);
            GUINode* find_build_node(GUIItemHandle handle);
            DockPanelPersistentState& get_or_create_dock_panel_state(PersistentItemState& dock_state, GUIID panel_id);
            u32 new_dock_leaf(PersistentItemState& dock_state, GUIID panel_id, u32 parent = U32_MAX);
            void dock_tree_add_panel(PersistentItemState& dock_state, GUIID panel_id);
            bool dock_tree_contains_panel(const PersistentItemState& dock_state, GUIID panel_id) const;
            bool dock_tree_remove_panel(PersistentItemState& dock_state, GUIID panel_id);
            void dock_tree_dock_panel(PersistentItemState& dock_state, GUIID panel_id, u32 target_leaf, GUIDockDropDirection direction);
            void dock_tree_prune_missing(PersistentItemState& dock_state, const HashSet<GUIID, GUIIDHash>& live_panels);
            GUIID dock_tree_selected_panel(PersistentItemState& dock_state, u32 leaf_index);
            void arrange_dock_tree_node(GUIID dock_space_id, u32 node_index, const RectF& rect, const RectF& clip_rect, const HashMap<GUIID, u32, GUIIDHash>& panel_indices);
            RectF layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            GUILayoutMetrics measure_node(u32 node_index);
            void measure_table_tracks(u32 node_index, Vector<f32>& out_column_widths, Vector<f32>& out_row_heights, bool preferred);
            void arrange_table_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_tab_bar_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_dock_space_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void render_table_node(u32 node_index);
            void render_tab_item(u32 node_index);
            void render_tab_scroll_buttons(u32 node_index);
            void render_dock_panel_chrome(u32 node_index);
            bool hit_test_table_separator(const Float2U& pos, GUIID& out_id, bool& out_column, u32& out_index) const;
            void update_table_resize_from_pointer(const Float2U& pos);
            bool hit_test_dock_panel(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id) const;
            bool hit_test_dock_panel_chrome(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id, bool& out_resize, bool& out_close) const;
            void update_dock_panel_from_pointer(const Float2U& pos);
            bool hit_test_dock_panel_tab(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id, u32& out_leaf_index) const;
            bool hit_test_dock_splitter(const Float2U& pos, GUIID& out_space_id, u32& out_node_index, GUIDockSplitAxis& out_axis) const;
            void update_dock_splitter_from_pointer(const Float2U& pos);
            bool find_dock_drop_target(GUIID payload_panel, const Float2U& pos, GUIID& out_space_id, u32& out_leaf_index, GUIDockDropDirection& out_direction) const;
            void render_dock_preview();
            void raise_dock_panel(GUIID dock_space_id, GUIID panel_id);
            DockPanelPersistentState* find_dock_panel_state(GUIID dock_space_id, GUIID panel_id);
            void clamp_scroll_state(GUIID id);
            bool hit_test_scrollbar(const Float2U& pos, GUIID& out_id, bool& out_vertical, RectF& out_thumb_rect) const;
            void update_scrollbar_from_pointer(const Float2U& pos);
            GUIID hit_test_drag_drop_source(const Float2U& pos, Name& out_type) const;
            GUIID hit_test_drag_drop_target(const Name& type, const Float2U& pos) const;
            void start_drag_drop(GUIID source_id, const Name& type);
            void clear_drag_drop();
            void deliver_drag_drop_payload(GUIID target_id);
            bool hit_test_combo_dropdown(const Float2U& pos, GUIID& out_id, i32& out_item) const;
            void close_combo_dropdowns_except(GUIID keep_id);
            bool hit_test_tab_header(const Float2U& pos, GUIID& out_tab_bar_id, GUIID& out_tab_item_id, bool& out_close) const;
            bool hit_test_tab_scroll_button(const Float2U& pos, GUIID& out_tab_bar_id, bool& out_left) const;
            GUIID hit_test_tab_scroll_area(const Float2U& pos) const;
            void select_tab_item(GUIID tab_bar_id, GUIID tab_item_id);
            GUIID fallback_tab_item(GUIID tab_bar_id, GUIID excluded_tab_item_id) const;
            bool reorder_tab_item_from_pointer(GUIID tab_bar_id, GUIID tab_item_id, const Float2U& pos);
            void scroll_tab_bar(GUIID tab_bar_id, f32 delta);
            GUIID hit_test_node(u32 node_index, const Float2U& pos, bool filter_kind, GUINodeKind kind) const;
            GUIID hit_test(const Float2U& pos) const;
            GUIID hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const;
            GUINode* find_node(GUIID id);
            u32 hit_test_float_component(const GUINode& node, const RectF& rect, const Float2U& pos) const;
            void update_float_node_from_pointer(GUIID id, const Float2U& pos, const Float2U* old_pos = nullptr);
            bool input_text_cursor_from_pointer(GUIID id, const Float2U& pos, usize& out_cursor);
            bool update_input_text_selection_from_pointer(GUIID id, const Float2U& pos);
            void process_input_events();
            void render_node(u32 node_index);
            void render_combo_dropdown(const GUINode& node, const RectF& rect);
            void render_drag_drop_overlay();
            void render_scrollbars(u32 node_index);
            void render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture = nullptr);
            void render_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                bool top_left, bool top_right, bool bottom_right, bool bottom_left);
            void render_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color);
            void render_line_segment(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width);
            void render_line(const GUINode& node, const RectF& rect, const RectF& clip_rect);
            void render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color, VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment = VG::TextAlignment::center);
            RectF to_vg_rect(const RectF& rect) const;
        };

        GUIContext* require_current_context();
    }
}
