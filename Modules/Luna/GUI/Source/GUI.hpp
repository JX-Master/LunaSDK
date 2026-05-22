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
                kind == GUINodeKind::table_layout)
            {
                style.width_policy = GUISizePolicy::fill;
            }
            return style;
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

        struct GUIIDHash
        {
            usize operator()(GUIID value) const
            {
                return (usize)value;
            }
        };

        struct ItemResult
        {
            u64 generation = 0;
            HashMap<Name, Any> states;
        };

        struct PersistentItemState
        {
            bool open = true;
            bool active = false;
            bool focused = false;
            bool pointer_down = false;
            f32 scroll_y = 0.0f;
            f64 last_click_time = -1000.0;
            Vector<f32> table_column_sizes;
            Vector<f32> table_row_sizes;
        };

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
        };

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
            Vector<u32> m_child_ordinals;
            HashMap<GUIID, ItemResult, GUIIDHash> m_last_results;
            HashMap<GUIID, ItemResult, GUIIDHash> m_current_results;
            HashMap<GUIID, PersistentItemState, GUIIDHash> m_persistent_states;
            GUIID m_active_id = 0;
            GUIID m_focused_id = 0;
            GUIID m_hovered_id = 0;
            Float2U m_pointer_pos = Float2U(0.0f);
            bool m_pointer_inside = false;
            bool m_submitted = false;
            bool m_has_next_item_layout = false;
            GUILayoutStyle m_next_item_layout;
            bool m_has_next_table_cell_color = false;
            Float4U m_next_table_cell_color = Float4U(0.0f);
            bool m_layout_dirty = false;
            GUIID m_active_table_resize_id = 0;
            bool m_active_table_resize_column = false;
            u32 m_active_table_resize_index = U32_MAX;
            u64 m_generation = 0;
            f64 m_time = 0.0;
            Ref<VG::IShapeDrawList> m_shape_draw_list;
            Ref<IDrawList> m_gui_draw_list;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;

            GUIContext();

            virtual void begin_frame(const GUIFrameDesc& desc) override;
            virtual void add_input_event(const GUIInputEvent& event) override;
            virtual void add_input_events(Span<const GUIInputEvent> events) override;
            virtual R<GUIDescription> end_build() override;
            virtual RV submit(const GUIDescription& desc) override;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) override;

            GUIItemHandle add_node(GUINodeKind kind, const c8* text, bool interactive);
            void begin_container(GUINodeKind kind, const c8* label, const GUISize& size, GUIItemHandle* out_handle);
            void end_container();
            const Any* get_state(GUIItemHandle handle, const Name& key);
            void set_state(GUIItemHandle handle, const Name& key, const Any& value);
            void remove_state(GUIItemHandle handle, const Name& key);
            void set_next_item_layout(const GUILayoutStyle& style);
            void set_next_table_cell_color(const Float4U& color);
            void push_id(GUIID id);
            void pop_id();

            ItemResult* get_query_result(GUIItemHandle handle);
            ItemResult& get_or_create_current_result(GUIID id);
            PersistentItemState& get_or_create_persistent_state(GUIID id);
            RectF layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            GUILayoutMetrics measure_node(u32 node_index);
            void measure_table_tracks(u32 node_index, Vector<f32>& out_column_widths, Vector<f32>& out_row_heights, bool preferred);
            void arrange_table_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void render_table_node(u32 node_index);
            bool hit_test_table_separator(const Float2U& pos, GUIID& out_id, bool& out_column, u32& out_index) const;
            void update_table_resize_from_pointer(const Float2U& pos);
            GUIID hit_test(const Float2U& pos) const;
            GUIID hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const;
            GUINode* find_node(GUIID id);
            void update_float_node_from_pointer(GUIID id, const Float2U& pos);
            void process_input_events();
            void render_node(u32 node_index);
            void render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture = nullptr);
            void render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color, VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment = VG::TextAlignment::center);
            RectF to_vg_rect(const RectF& rect) const;
        };

        GUIContext* require_current_context();
    }
}
