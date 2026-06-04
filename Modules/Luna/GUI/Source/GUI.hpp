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
#include "Nodes/Common.hpp"
#include "Nodes/LayoutNodes.hpp"
#include "Nodes/BasicNodes.hpp"
#include "Nodes/MenuNodes.hpp"
#include "Nodes/InputNodes.hpp"
#include "Nodes/ColorNodes.hpp"
#include "Nodes/ButtonGroupNodes.hpp"
#include "Nodes/DrawingNodes.hpp"
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
        struct Context;

        enum class HitTestFilter : u8
        {
            none,
            scroll_view
        };

        enum class NumericInteractionKind : u8
        {
            none,
            slider,
            input,
            drag
        };

        inline Name default_font_id()
        {
            return Name("gui.default_font");
        }

        inline Name font_style_entry_name()
        {
            return Name("gui.font");
        }

        struct FontResource
        {
            Ref<Font::IFontFile> font;
            u32 font_index = 0;
        };

        inline Context* context_from_interface(IContext* context)
        {
            luassert_msg(context, "GUI context must not be null.");
            return (Context*)context->get_object();
        }

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

        inline SizePolicy axis_policy(const LayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.width_policy : style.height_policy;
        }

        inline f32 axis_fixed_size(const LayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.fixed_width_value : style.fixed_height_value;
        }

        inline f32 axis_fill_weight(const LayoutStyle& style, bool x_axis)
        {
            return x_axis ? style.fill_weight_x : style.fill_weight_y;
        }

        template <typename T>
        inline T* cast_node(Node& node)
        {
            return node.type_guid() == T::__guid ? (T*)&node : nullptr;
        }

        template <typename T>
        inline const T* cast_node(const Node& node)
        {
            return node.type_guid() == T::__guid ? (const T*)&node : nullptr;
        }

        inline WindowNode* window_node(Node& node)
        {
            return cast_node<WindowNode>(node);
        }

        inline const WindowNode* window_node(const Node& node)
        {
            return cast_node<WindowNode>(node);
        }

        inline PopupNode* popup_node(Node& node)
        {
            return cast_node<PopupNode>(node);
        }

        inline const PopupNode* popup_node(const Node& node)
        {
            return cast_node<PopupNode>(node);
        }

        inline TooltipNode* tooltip_node(Node& node)
        {
            return cast_node<TooltipNode>(node);
        }

        inline const TooltipNode* tooltip_node(const Node& node)
        {
            return cast_node<TooltipNode>(node);
        }

        inline TableLayoutNode* table_layout_node(Node& node)
        {
            return cast_node<TableLayoutNode>(node);
        }

        inline const TableLayoutNode* table_layout_node(const Node& node)
        {
            return cast_node<TableLayoutNode>(node);
        }

        inline GridLayoutNode* grid_layout_node(Node& node)
        {
            return cast_node<GridLayoutNode>(node);
        }

        inline const GridLayoutNode* grid_layout_node(const Node& node)
        {
            return cast_node<GridLayoutNode>(node);
        }

        inline CanvasLayoutNode* canvas_layout_node(Node& node)
        {
            return cast_node<CanvasLayoutNode>(node);
        }

        inline const CanvasLayoutNode* canvas_layout_node(const Node& node)
        {
            return cast_node<CanvasLayoutNode>(node);
        }

        inline TabBarNode* tab_bar_node(Node& node)
        {
            return cast_node<TabBarNode>(node);
        }

        inline const TabBarNode* tab_bar_node(const Node& node)
        {
            return cast_node<TabBarNode>(node);
        }

        inline TabItemNode* tab_item_node(Node& node)
        {
            return cast_node<TabItemNode>(node);
        }

        inline const TabItemNode* tab_item_node(const Node& node)
        {
            return cast_node<TabItemNode>(node);
        }

        inline MenuItemNode* menu_item_node(Node& node)
        {
            return cast_node<MenuItemNode>(node);
        }

        inline const MenuItemNode* menu_item_node(const Node& node)
        {
            return cast_node<MenuItemNode>(node);
        }

        inline InputTextNode* input_text_node_cast(Node& node)
        {
            return cast_node<InputTextNode>(node);
        }

        inline const InputTextNode* input_text_node_cast(const Node& node)
        {
            return cast_node<InputTextNode>(node);
        }

        inline ColorPickerNode* color_picker_node_cast(Node& node)
        {
            return cast_node<ColorPickerNode>(node);
        }

        inline const ColorPickerNode* color_picker_node_cast(const Node& node)
        {
            return cast_node<ColorPickerNode>(node);
        }

        inline NumericBinding* numeric_binding(Node& node)
        {
            if(SliderFloatNode* typed = cast_node<SliderFloatNode>(node)) return &typed->binding;
            if(SliderIntNode* typed = cast_node<SliderIntNode>(node)) return &typed->binding;
            if(InputFloatNode* typed = cast_node<InputFloatNode>(node)) return &typed->binding;
            if(InputIntNode* typed = cast_node<InputIntNode>(node)) return &typed->binding;
            if(DragFloatNode* typed = cast_node<DragFloatNode>(node)) return &typed->binding;
            if(DragIntNode* typed = cast_node<DragIntNode>(node)) return &typed->binding;
            return nullptr;
        }

        inline const NumericBinding* numeric_binding(const Node& node)
        {
            if(const SliderFloatNode* typed = cast_node<SliderFloatNode>(node)) return &typed->binding;
            if(const SliderIntNode* typed = cast_node<SliderIntNode>(node)) return &typed->binding;
            if(const InputFloatNode* typed = cast_node<InputFloatNode>(node)) return &typed->binding;
            if(const InputIntNode* typed = cast_node<InputIntNode>(node)) return &typed->binding;
            if(const DragFloatNode* typed = cast_node<DragFloatNode>(node)) return &typed->binding;
            if(const DragIntNode* typed = cast_node<DragIntNode>(node)) return &typed->binding;
            return nullptr;
        }

        inline NumericInteractionKind numeric_interaction_kind(const Node& node)
        {
            if(cast_node<SliderFloatNode>(node) || cast_node<SliderIntNode>(node)) return NumericInteractionKind::slider;
            if(cast_node<InputFloatNode>(node) || cast_node<InputIntNode>(node)) return NumericInteractionKind::input;
            if(cast_node<DragFloatNode>(node) || cast_node<DragIntNode>(node)) return NumericInteractionKind::drag;
            return NumericInteractionKind::none;
        }

        inline ColorBinding* color_binding(Node& node)
        {
            if(ColorPickerNode* typed = color_picker_node_cast(node)) return &typed->binding;
            return nullptr;
        }

        inline const ColorBinding* color_binding(const Node& node)
        {
            if(const ColorPickerNode* typed = color_picker_node_cast(node)) return &typed->binding;
            return nullptr;
        }

        inline String* input_text_value(Node& node)
        {
            InputTextNode* typed = input_text_node_cast(node);
            return typed ? typed->value : nullptr;
        }

        inline const String* input_text_value(const Node& node)
        {
            const InputTextNode* typed = input_text_node_cast(node);
            return typed ? typed->value : nullptr;
        }

        inline id_t popup_owner(const Node& node)
        {
            if(const PopupNode* popup = popup_node(node)) return popup->owner;
            if(const TooltipNode* tooltip = tooltip_node(node)) return tooltip->owner;
            return 0;
        }

        inline void set_popup_owner(Node& node, id_t value)
        {
            if(PopupNode* popup = popup_node(node))
            {
                popup->owner = value;
            }
            else if(TooltipNode* tooltip = tooltip_node(node))
            {
                tooltip->owner = value;
            }
        }

        inline PopupFlag popup_flags(const Node& node)
        {
            const PopupNode* popup = popup_node(node);
            return popup ? popup->flags : PopupFlag::none;
        }

        inline id_t popup_parent(const Node& node)
        {
            const PopupNode* popup = popup_node(node);
            return popup ? popup->parent_popup : 0;
        }

        inline void set_popup_parent(Node& node, id_t value)
        {
            if(PopupNode* popup = popup_node(node))
            {
                popup->parent_popup = value;
            }
        }

        inline id_t menu_popup(const Node& node)
        {
            const MenuItemNode* menu = menu_item_node(node);
            return menu ? menu->popup_id : 0;
        }

        inline u32 table_columns(const Node& node)
        {
            const TableLayoutNode* typed = table_layout_node(node);
            const TableDesc* desc = typed ? &typed->desc : nullptr;
            return desc ? max(desc->columns, 1u) : 1u;
        }

        inline const TableDesc& table_desc(const Node& node)
        {
            const TableLayoutNode* typed = table_layout_node(node);
            luassert(typed);
            return typed->desc;
        }

        inline const TableCellAttachment* table_cell_attachment(const Node& node, u32 child_index)
        {
            const TableLayoutNode* table = table_layout_node(node);
            if(!table) return nullptr;
            for(const TableCellAttachment& attachment : table->cell_attachments)
            {
                if(attachment.child_index == child_index) return &attachment;
            }
            return nullptr;
        }

        inline const GridLayoutDesc& grid_desc(const Node& node)
        {
            const GridLayoutNode* typed = grid_layout_node(node);
            luassert(typed);
            return typed->desc;
        }

        inline const CanvasLayoutDesc& canvas_desc(const Node& node)
        {
            const CanvasLayoutNode* typed = canvas_layout_node(node);
            luassert(typed);
            return typed->desc;
        }

        inline const CanvasItemAttachment* canvas_item_attachment(const Node& node, u32 child_index)
        {
            const CanvasLayoutNode* canvas = canvas_layout_node(node);
            if(!canvas) return nullptr;
            for(const CanvasItemAttachment& attachment : canvas->item_attachments)
            {
                if(attachment.child_index == child_index) return &attachment;
            }
            return nullptr;
        }

        inline const DockPanelAttachment* dock_panel_attachment(const Node& node, u32 child_index)
        {
            const DockSpaceNode* dock_space = cast_node<DockSpaceNode>(node);
            if(!dock_space) return nullptr;
            for(const DockPanelAttachment& attachment : dock_space->panel_attachments)
            {
                if(attachment.child_index == child_index) return &attachment;
            }
            return nullptr;
        }

        inline const TooltipDesc& tooltip_desc(const Node& node)
        {
            const TooltipNode* typed = tooltip_node(node);
            luassert(typed);
            return typed->desc;
        }

        inline bool bool_value_open(const Node& node)
        {
            const TabItemNode* tab = tab_item_node(node);
            bool* value = tab ? tab->open : nullptr;
            if(!tab)
            {
                const WindowNode* window = window_node(node);
                value = window ? window->open : nullptr;
            }
            return !value || *value;
        }

        inline u32 table_child_count(const Description& desc, const Node& node)
        {
            u32 ret = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = desc.nodes[child].next_sibling)
            {
                ++ret;
            }
            return ret;
        }

        inline u32 table_rows(const Description& desc, const Node& node)
        {
            u32 columns = table_columns(node);
            u32 child_count = table_child_count(desc, node);
            const TableLayoutNode* table = table_layout_node(node);
            const TableDesc* table_desc = table ? &table->desc : nullptr;
            return max((child_count + columns - 1) / columns, table_desc ? (u32)table_desc->row_sizes.size() : 0u);
        }

        inline const TableTrackSize& table_track_size(const Node& node, bool column, u32 index)
        {
            static TableTrackSize default_size;
            const TableLayoutNode* table = table_layout_node(node);
            const TableDesc* desc = table ? &table->desc : nullptr;
            if(!desc) return default_size;
            const Vector<TableTrackSize>& sizes = column ? desc->column_sizes : desc->row_sizes;
            return index < sizes.size() ? sizes[index] : default_size;
        }

        inline bool table_track_is_fixed(const Node& node, bool column, u32 index)
        {
            return table_track_size(node, column, index).policy == TableTrackSizePolicy::fixed;
        }

        inline bool window_has_title_bar(const Node& node)
        {
            const WindowNode* window = window_node(node);
            return window && window->open;
        }

        inline f32 window_title_bar_height()
        {
            return 30.0f;
        }

        inline u32 f32_value_count(const Node& node)
        {
            const NumericBinding* numeric = numeric_binding(node);
            if(numeric) return clamp((u32)numeric->value_count, 1u, 4u);
            const ColorBinding* color = color_binding(node);
            return color ? clamp((u32)color->value_count, 1u, 4u) : 1u;
        }

        inline u32 i32_value_count(const Node& node)
        {
            const NumericBinding* numeric = numeric_binding(node);
            return numeric ? clamp((u32)numeric->value_count, 1u, 4u) : 1u;
        }

        inline bool numeric_value_f32(const Node& node)
        {
            const NumericBinding* binding = numeric_binding(node);
            return binding && binding->f32_value;
        }

        inline bool numeric_value_i32(const Node& node)
        {
            const NumericBinding* binding = numeric_binding(node);
            return binding && binding->i32_value;
        }

        inline bool numeric_node(const Node& node)
        {
            return numeric_binding(node) != nullptr;
        }

        inline bool numeric_text_editable(const Node& node)
        {
            NumericInteractionKind interaction = numeric_interaction_kind(node);
            const NumericBinding* binding = numeric_binding(node);
            if(interaction == NumericInteractionKind::input) return true;
            return binding && interaction == NumericInteractionKind::drag &&
                test_flags(binding->flags, NumericEditFlag::input_on_double_click);
        }

        inline bool numeric_pointer_editable(const Node& node)
        {
            NumericInteractionKind interaction = numeric_interaction_kind(node);
            return interaction == NumericInteractionKind::slider || interaction == NumericInteractionKind::drag;
        }

        inline bool numeric_slider(const Node& node)
        {
            return numeric_interaction_kind(node) == NumericInteractionKind::slider;
        }

        inline bool numeric_drag(const Node& node)
        {
            return numeric_interaction_kind(node) == NumericInteractionKind::drag;
        }

        inline u32 numeric_value_count(const Node& node)
        {
            return numeric_value_f32(node) ? f32_value_count(node) : i32_value_count(node);
        }

        inline f32 numeric_label_width(const Node& node, const RectF& rect)
        {
            if(node.text.empty()) return 0.0f;
            const NumericBinding* binding = numeric_binding(node);
            if(binding && binding->color_owner_id) return 48.0f;
            return min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
        }

        inline RectF numeric_component_rect(const Node& node, const RectF& rect, u32 component)
        {
            f32 label_w = numeric_label_width(node, rect);
            u32 value_count = numeric_value_count(node);
            f32 gap = 4.0f;
            f32 value_area_x = rect.offset_x + label_w;
            f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
            return RectF(value_area_x + (component_w + gap) * (f32)min(component, value_count - 1), rect.offset_y + 3.0f,
                component_w, max(rect.height - 6.0f, 1.0f));
        }

        inline String numeric_value_text(const Node& node, u32 component)
        {
            String value_text;
            const NumericBinding* binding = numeric_binding(node);
            if(numeric_value_f32(node))
            {
                f32* values = binding ? binding->f32_value : nullptr;
                f32 value = values ? values[min(component, f32_value_count(node) - 1)] : 0.0f;
                strprintf(value_text, "%.3f", value);
            }
            else
            {
                i32* values = binding ? binding->i32_value : nullptr;
                i32 value = values ? values[min(component, i32_value_count(node) - 1)] : 0;
                strprintf(value_text, "%d", value);
            }
            return value_text;
        }

        ItemHandle add_slider_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value);
        ItemHandle add_slider_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value);
        ItemHandle add_input_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value);
        ItemHandle add_input_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value);

        inline u8 color_channel_to_u8(f32 value)
        {
            return (u8)clamp(value * 255.0f + 0.5f, 0.0f, 255.0f);
        }

        inline f32 color_u8_to_channel(u8 value)
        {
            return (f32)value / 255.0f;
        }

        inline u8 color_value_count(const ColorBinding& binding)
        {
            return (u8)clamp((u32)binding.value_count, 1u, 4u);
        }

        inline Float4U read_color_value(const ColorBinding& binding)
        {
            u8* u8_values = binding.u8_value;
            u32* rgba8_value = binding.u32_value;
            f32* f32_values = binding.f32_value;
            u32 value_count = color_value_count(binding);
            if(binding.type == ColorValueType::u8 && u8_values)
            {
                return Float4U(
                    color_u8_to_channel(u8_values[0]),
                    color_u8_to_channel(u8_values[1]),
                    color_u8_to_channel(u8_values[2]),
                    value_count > 3 ? color_u8_to_channel(u8_values[3]) : 1.0f);
            }
            if(binding.type == ColorValueType::rgba8 && rgba8_value)
            {
                u32 value = *rgba8_value;
                return Float4U(
                    color_u8_to_channel((u8)(value & 0xffu)),
                    color_u8_to_channel((u8)((value >> 8) & 0xffu)),
                    color_u8_to_channel((u8)((value >> 16) & 0xffu)),
                    value_count > 3 ? color_u8_to_channel((u8)((value >> 24) & 0xffu)) : 1.0f);
            }
            if(f32_values)
            {
                return Float4U(
                    clamp(f32_values[0], 0.0f, 1.0f),
                    clamp(value_count > 1 ? f32_values[1] : 0.0f, 0.0f, 1.0f),
                    clamp(value_count > 2 ? f32_values[2] : 0.0f, 0.0f, 1.0f),
                    clamp(value_count > 3 ? f32_values[3] : 1.0f, 0.0f, 1.0f));
            }
            return Float4U(0.0f, 0.0f, 0.0f, 1.0f);
        }

        inline void write_color_value(ColorBinding& binding, Float4U color)
        {
            color.x = clamp(color.x, 0.0f, 1.0f);
            color.y = clamp(color.y, 0.0f, 1.0f);
            color.z = clamp(color.z, 0.0f, 1.0f);
            u32 value_count = color_value_count(binding);
            color.w = value_count > 3 ? clamp(color.w, 0.0f, 1.0f) : 1.0f;
            u8* u8_values = binding.u8_value;
            u32* rgba8_value = binding.u32_value;
            f32* f32_values = binding.f32_value;
            if(binding.type == ColorValueType::u8 && u8_values)
            {
                u8_values[0] = color_channel_to_u8(color.x);
                u8_values[1] = color_channel_to_u8(color.y);
                u8_values[2] = color_channel_to_u8(color.z);
                if(value_count > 3) u8_values[3] = color_channel_to_u8(color.w);
            }
            else if(binding.type == ColorValueType::rgba8 && rgba8_value)
            {
                u32 r = (u32)color_channel_to_u8(color.x);
                u32 g = (u32)color_channel_to_u8(color.y);
                u32 b = (u32)color_channel_to_u8(color.z);
                u32 a = value_count > 3 ? (u32)color_channel_to_u8(color.w) : 255u;
                *rgba8_value = r | (g << 8) | (b << 16) | (a << 24);
            }
            else if(f32_values)
            {
                f32_values[0] = color.x;
                f32_values[1] = color.y;
                f32_values[2] = color.z;
                if(value_count > 3) f32_values[3] = color.w;
            }
        }

        inline ColorBinding color_binding_from_node(const Node& node)
        {
            ColorBinding binding;
            const ColorBinding* color = color_binding(node);
            if(color)
            {
                binding = *color;
            }
            return binding;
        }

        inline Float4U read_color_value(const Node& node)
        {
            return read_color_value(color_binding_from_node(node));
        }

        inline void write_color_value(Node& node, Float4U color)
        {
            ColorBinding binding = color_binding_from_node(node);
            write_color_value(binding, color);
        }

        inline void color_rgb_to_hsv(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v)
        {
            f32 max_value = max(max(r, g), b);
            f32 min_value = min(min(r, g), b);
            f32 delta = max_value - min_value;
            v = max_value;
            s = max_value <= 0.0f ? 0.0f : delta / max_value;
            if(delta <= 0.000001f)
            {
                h = 0.0f;
            }
            else if(max_value == r)
            {
                h = (g - b) / delta;
                if(h < 0.0f) h += 6.0f;
                h /= 6.0f;
            }
            else if(max_value == g)
            {
                h = ((b - r) / delta + 2.0f) / 6.0f;
            }
            else
            {
                h = ((r - g) / delta + 4.0f) / 6.0f;
            }
            h = clamp(h, 0.0f, 1.0f);
        }

        inline Float4U color_hsv_to_rgb(f32 h, f32 s, f32 v, f32 a = 1.0f)
        {
            h = clamp(h, 0.0f, 1.0f);
            s = clamp(s, 0.0f, 1.0f);
            v = clamp(v, 0.0f, 1.0f);
            f32 r = v;
            f32 g = v;
            f32 b = v;
            if(s > 0.0f)
            {
                f32 scaled = h * 6.0f;
                i32 sector = (i32)floorf(scaled);
                f32 f = scaled - (f32)sector;
                f32 p = v * (1.0f - s);
                f32 q = v * (1.0f - s * f);
                f32 t = v * (1.0f - s * (1.0f - f));
                switch(sector % 6)
                {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                default: r = v; g = p; b = q; break;
                }
            }
            return Float4U(r, g, b, clamp(a, 0.0f, 1.0f));
        }

        inline void color_picker_channels_from_color(i32 axis, const Float4U& color, f32& x, f32& y, f32& bar)
        {
            axis = clamp(axis, 0, 5);
            if(axis < 3)
            {
                f32 h = 0.0f;
                f32 s = 0.0f;
                f32 v = 0.0f;
                color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
                if(axis == 0) { x = s; y = v; bar = h; }
                else if(axis == 1) { x = h; y = v; bar = s; }
                else { x = h; y = s; bar = v; }
            }
            else
            {
                if(axis == 3) { x = color.y; y = color.z; bar = color.x; }
                else if(axis == 4) { x = color.x; y = color.z; bar = color.y; }
                else { x = color.x; y = color.y; bar = color.z; }
            }
        }

        inline Float4U color_from_picker_channels(i32 axis, f32 x, f32 y, f32 bar, f32 alpha)
        {
            axis = clamp(axis, 0, 5);
            x = clamp(x, 0.0f, 1.0f);
            y = clamp(y, 0.0f, 1.0f);
            bar = clamp(bar, 0.0f, 1.0f);
            alpha = clamp(alpha, 0.0f, 1.0f);
            if(axis == 0) return color_hsv_to_rgb(bar, x, y, alpha);
            if(axis == 1) return color_hsv_to_rgb(x, bar, y, alpha);
            if(axis == 2) return color_hsv_to_rgb(x, y, bar, alpha);
            if(axis == 3) return Float4U(bar, x, y, alpha);
            if(axis == 4) return Float4U(x, bar, y, alpha);
            return Float4U(x, y, bar, alpha);
        }

        inline RectF color_picker_square_rect(const RectF& rect)
        {
            f32 right_width = 112.0f;
            f32 bar_width = 24.0f;
            f32 gap = 10.0f;
            f32 square_size = min(rect.height, max(rect.width - right_width - bar_width - gap * 2.0f, 1.0f));
            return RectF(rect.offset_x, rect.offset_y, square_size, square_size);
        }

        inline RectF color_picker_bar_rect(const RectF& rect)
        {
            RectF square = color_picker_square_rect(rect);
            return RectF(square.offset_x + square.width + 10.0f, square.offset_y, 24.0f, square.height);
        }

        inline RectF color_picker_current_rect(const RectF& rect)
        {
            RectF bar = color_picker_bar_rect(rect);
            return RectF(bar.offset_x + bar.width + 10.0f, bar.offset_y + 28.0f, 102.0f, 58.0f);
        }

        inline RectF color_picker_original_rect(const RectF& rect)
        {
            RectF cur = color_picker_current_rect(rect);
            return RectF(cur.offset_x, cur.offset_y + cur.height + 44.0f, cur.width, cur.height);
        }

        inline bool popup_layer(const Node& node)
        {
            return node.layer_role() == NodeLayerRole::popup;
        }

        inline bool tooltip_layer(const Node& node)
        {
            return node.layer_role() == NodeLayerRole::tooltip;
        }

        inline bool root_layer(const Node& node)
        {
            return node.layer_role() == NodeLayerRole::root;
        }

        inline bool input_text_node(const Node& node)
        {
            return input_text_node_cast(node) != nullptr;
        }

        inline bool menu_node(const Node& node)
        {
            const MenuItemNode* menu = menu_item_node(node);
            return menu && menu->popup_id;
        }

        inline bool color_picker_node(const Node& node)
        {
            return color_picker_node_cast(node) != nullptr;
        }

        inline bool scroll_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::scroll;
        }

        inline bool table_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::table;
        }

        inline bool grid_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::grid;
        }

        inline bool canvas_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::canvas;
        }

        inline bool dock_space_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::dock_space;
        }

        inline bool tab_bar_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::tab_bar;
        }

        inline bool tab_item_layout(const Node& node)
        {
            return node.layout_behavior() == NodeLayoutBehavior::tab_item;
        }

        inline bool horizontal_layout(const Node& node)
        {
            return node.layout_flow() == NodeLayoutFlow::horizontal;
        }

        inline bool absolute_node(const Node& node)
        {
            return node.absolute_position || popup_layer(node) || tooltip_layer(node);
        }

        inline bool contains_name(const Vector<Name>& names, const Name& name)
        {
            for(const Name& item : names)
            {
                if(item == name) return true;
            }
            return false;
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

        inline f32 tab_item_ideal_width(const Node& node)
        {
            const TabItemNode* tab = tab_item_node(node);
            luassert(tab);
            f32 font_size = 15.0f;
            f32 text_width = (f32)node.text.size() * font_size * 0.52f;
            f32 close_width = (tab->open && !test_flags(tab->flags, TabItemFlag::no_close_button)) ? 22.0f : 0.0f;
            f32 unsaved_width = test_flags(tab->flags, TabItemFlag::unsaved_document) ? 12.0f : 0.0f;
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

        inline f32 resolve_base_axis_size(const Node& node, const LayoutMetrics& metrics, bool x_axis)
        {
            if(axis_policy(node.layout_style, x_axis) == SizePolicy::fixed)
            {
                return clamp(axis_fixed_size(node.layout_style, x_axis),
                    axis_value(metrics.min_size, x_axis),
                    axis_value(metrics.max_size, x_axis));
            }
            return axis_value(metrics.preferred_size, x_axis);
        }

        inline LayoutMetrics apply_layout_style(const Node& node, LayoutMetrics metrics)
        {
            for(u32 axis = 0; axis < 2; ++axis)
            {
                bool x_axis = axis == 0;
                f32 min_size = max(axis_value(metrics.min_size, x_axis), axis_value(node.layout_style.min_size, x_axis));
                f32 max_size = min(axis_value(metrics.max_size, x_axis), axis_value(node.layout_style.max_size, x_axis));
                if(max_size < min_size) max_size = min_size;
                f32 preferred_size = clamp(axis_value(metrics.preferred_size, x_axis), min_size, max_size);
                if(axis_policy(node.layout_style, x_axis) == SizePolicy::fixed)
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

        inline void apply_requested_size(Node& node, const Size& size)
        {
            node.requested_size = size;
            if(size.width > 0.0f)
            {
                node.layout_style.width_policy = SizePolicy::fixed;
                node.layout_style.fixed_width_value = size.width;
            }
            if(size.height > 0.0f)
            {
                node.layout_style.height_policy = SizePolicy::fixed;
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

        inline bool has_modifier(KeyModifierFlag flags, KeyModifierFlag flag)
        {
            return (((u8)flags) & ((u8)flag)) != 0;
        }

        struct IdHash
        {
            usize operator()(id_t value) const
            {
                return (usize)value;
            }
        };

        struct DockPanelPersistentState
        {
            bool initialized = false;
            bool closed = false;
            DockPanelMode mode = DockPanelMode::docking;
            RectF rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
            f32 docking_height = 0.0f;
            u32 z_order = 0;
        };

        enum class DockSplitAxis : u8
        {
            x,
            y
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
            bool split = false;
            u32 parent = U32_MAX;
            u32 child0 = U32_MAX;
            u32 child1 = U32_MAX;
            DockSplitAxis split_axis = DockSplitAxis::x;
            f32 split_ratio = 0.5f;
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF split_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Vector<id_t> tabs;
            id_t selected_tab = 0;
        };

        struct StateRecord
        {
            ObjRef data;
            StateLifetime lifetime = StateLifetime::next_frame;
            u64 last_set_generation = 0;
        };

        struct InteractionState
        {
            lustruct("GUI::InteractionState", "{531E33F1-5236-4E30-8FC6-71D06B8EE0B6}");
            bool active = false;
            bool focused = false;
            bool pointer_down = false;
            f64 last_click_time = -1000.0;
            f64 last_right_click_time = -1000.0;
        };

        struct ScrollState
        {
            lustruct("GUI::ScrollState", "{1A6F29FC-6D16-4A44-954E-463563E26428}");
            f32 scroll_x = 0.0f;
            f32 scroll_y = 0.0f;
            f32 scrollbar_opacity = 0.35f;
        };

        struct NumericInteractionState
        {
            lustruct("GUI::NumericInteractionState", "{14494B92-C943-49E6-85EA-7CC0AAE026D0}");
            u32 active_float_component = U32_MAX;
            Float2U active_numeric_start_pos = Float2U(0.0f);
            bool active_numeric_defer_until_drag = false;
        };

        struct ColorPickerInteractionState
        {
            lustruct("GUI::ColorPickerInteractionState", "{E8F4622D-7EBF-46FE-9689-FA7AE1C3CC37}");
            u32 active_color_part = 0;
        };

        struct TableResizeInteractionState
        {
            lustruct("GUI::TableResizeInteractionState", "{1ADB536C-7F49-48A7-9363-0616CACED450}");
            id_t active_table_resize_id = 0;
            bool active_table_resize_column = false;
            u32 active_table_resize_index = U32_MAX;
        };

        struct ScrollbarInteractionState
        {
            lustruct("GUI::ScrollbarInteractionState", "{52DC17A8-55C6-4873-88A9-F9132874C22D}");
            id_t active_scrollbar_id = 0;
            bool active_scrollbar_vertical = false;
            f32 active_scrollbar_grab_offset = 0.0f;
        };

        struct DockInteractionState
        {
            lustruct("GUI::DockInteractionState", "{778DB35B-0546-4610-916F-842DCD36818B}");
            id_t active_dock_space_id = 0;
            id_t active_dock_panel_id = 0;
            bool active_dock_panel_resize = false;
            bool active_dock_panel_close = false;
            bool active_dock_panel_was_floating = false;
            bool active_dock_panel_title_drag = false;
            bool active_dock_panel_undocked = false;
            id_t active_dock_panel_resize_neighbor_id = 0;
            Float2U active_dock_panel_grab_offset = Float2U(0.0f);
            RectF active_dock_panel_start_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF active_dock_panel_restore_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF active_dock_panel_start_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            f32 active_dock_panel_start_neighbor_height = 0.0f;
            id_t active_dock_split_space_id = 0;
            u32 active_dock_split_node = U32_MAX;
            DockSplitAxis active_dock_split_axis = DockSplitAxis::x;
            f32 active_dock_split_start_ratio = 0.5f;
            Float2U active_dock_split_start_pos = Float2U(0.0f);
        };

        struct TabInteractionState
        {
            lustruct("GUI::TabInteractionState", "{DF073278-FA77-4C3B-B33C-36B74AF9EBEE}");
            id_t active_tab_bar_id = 0;
            id_t active_tab_item_id = 0;
            bool active_tab_close = false;
            bool active_tab_reorder_allowed = false;
            bool active_tab_reordering = false;
            Float2U active_tab_start_pos = Float2U(0.0f);
            id_t active_tab_scroll_id = 0;
            bool active_tab_scroll_left = false;
        };

        struct TooltipInteractionState
        {
            lustruct("GUI::TooltipInteractionState", "{6FA7C420-CA7E-487C-BFB3-247792931FD7}");
            id_t tooltip_hovered_id = 0;
            f64 tooltip_hover_start = 0.0;
        };

        struct BuildHintState
        {
            lustruct("GUI::BuildHintState", "{B11BEC18-AD1E-4A26-8462-F13D9D65AB76}");
            bool has_next_item_layout = false;
            LayoutStyle next_item_layout;
            bool has_next_item_enabled = false;
            bool next_item_enabled = true;
            bool has_next_canvas_item_layout = false;
            CanvasItemLayout next_canvas_item_layout;
            bool has_next_table_cell_color = false;
            Float4U next_table_cell_color = Float4U(0.0f);
            bool has_next_dock_panel_style = false;
            DockPanelStyle next_dock_panel_style;
            bool* next_dock_panel_open = nullptr;
            bool has_next_render_proxy = false;
            RenderProxyDesc next_render_proxy;
        };

        struct DragDropPayloadStorage
        {
            Name type;
            Vector<byte_t> data;
            ItemHandle source;
            ItemHandle target;
            bool preview = false;
            bool delivery = false;
        };

        struct DragDropTargetScope
        {
            ItemHandle target;
            Name type;
        };

        struct DragDropManager
        {
            id_t candidate_source_id = 0;
            Name candidate_type;
            Float2U start_pos = Float2U(0.0f);
            bool active = false;
            bool payload_set = false;
            bool preview_built = false;
            id_t source_id = 0;
            Name type;
            Vector<byte_t> payload_data;
            HashMap<id_t, DragDropPayloadStorage, IdHash> last_deliveries;
            HashMap<id_t, DragDropPayloadStorage, IdHash> current_deliveries;
            Vector<DragDropTargetScope> target_stack;
            DragDropPayload payload_view;

            void begin_frame()
            {
                last_deliveries = current_deliveries;
                current_deliveries.clear();
                preview_built = false;
                target_stack.clear();
                if(active)
                {
                    payload_set = false;
                    payload_data.clear();
                }
            }

            void start(id_t source, const Name& payload_type)
            {
                active = true;
                source_id = source;
                type = payload_type;
                payload_set = false;
                payload_data.clear();
            }

            void clear()
            {
                candidate_source_id = 0;
                candidate_type.reset();
                active = false;
                payload_set = false;
                source_id = 0;
                type.reset();
                payload_data.clear();
            }

            void set_payload(const void* data, usize data_size)
            {
                payload_data.resize(data_size);
                if(data_size && data)
                {
                    memcpy(payload_data.data(), data, data_size);
                }
                payload_set = true;
            }
        };

        struct PopupStackEntry
        {
            id_t id = 0;
            id_t parent_id = 0;
            id_t opener_id = 0;
            PopupFlag flags = PopupFlag::none;
        };

        struct PopupBuildInfo
        {
            id_t parent_id = 0;
            PopupFlag flags = PopupFlag::none;
        };

        struct PopupStackManager
        {
            Vector<PopupStackEntry> open_stack;
            Vector<id_t> build_stack;
            HashMap<id_t, u32, IdHash> node_indices;
            HashMap<id_t, PopupBuildInfo, IdHash> build_infos;
            HashMap<id_t, PopupBuildInfo, IdHash> submitted_infos;
            id_t next_opener_id = 0;

            void begin_frame()
            {
                build_stack.clear();
                build_infos.clear();
            }
        };

        struct DockSpaceState
        {
            lustruct("GUI::DockSpaceState", "{EF185DED-76E2-4448-A137-004051FFFD5B}");
            u32 dock_next_z_order = 1;
            HashMap<id_t, DockPanelPersistentState, IdHash> dock_panels;
            Vector<DockTreeNode> dock_nodes;
            u32 dock_root_node = U32_MAX;
        };

        struct InputEditState
        {
            lustruct("GUI::InputEditState", "{DC801B89-9DEE-4456-8036-9F8C9A7C8A8A}");
            usize text_cursor = USIZE_MAX;
            usize text_select_anchor = USIZE_MAX;
            bool text_selecting = false;
            f64 text_cursor_blink_start = 0.0;
            u32 numeric_edit_component = 0;
            String numeric_edit_text;
            bool numeric_editing = false;
        };

        struct TableLayoutState
        {
            lustruct("GUI::TableLayoutState", "{79096C88-F7D0-4EF3-9521-7800D6F70F16}");
            Vector<f32> table_column_sizes;
            Vector<f32> table_row_sizes;
        };

        struct TabBarState
        {
            lustruct("GUI::TabBarState", "{AEA771D2-1441-4CE9-9876-38F1787F2C49}");
            id_t tab_selected_id = 0;
            f32 tab_scroll_x = 0.0f;
            Vector<id_t> tab_order;
        };

        struct ColorPickerState
        {
            lustruct("GUI::ColorPickerState", "{A9483A32-872C-47B0-9AAF-26468F6D411F}");
            Vector<i32> color_picker_axis;
            Vector<i32> color_picker_rgb;
            Vector<i32> color_picker_hsv;
            Float4U color_picker_original = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            bool color_picker_original_valid = false;
        };

        enum class PopupAnchorPlacement : u8
        {
            pointer,
            owner_down
        };

        struct PopupAnchorState
        {
            lustruct("GUI::PopupAnchorState", "{9BEED835-1593-4FAF-B0F7-FC753D462883}");
            Float2U popup_anchor_position = Float2U(0.0f, 0.0f);
            PopupAnchorPlacement popup_anchor_placement = PopupAnchorPlacement::pointer;
            bool popup_anchor_valid = false;
        };

        inline void ensure_color_picker_state_channels(ColorPickerState& state)
        {
            if(state.color_picker_axis.size() != 1)
            {
                state.color_picker_axis.resize(1, 0);
            }
            if(state.color_picker_rgb.size() != 4)
            {
                state.color_picker_rgb.resize(4, 0);
                state.color_picker_rgb[3] = 255;
            }
            if(state.color_picker_hsv.size() != 3)
            {
                state.color_picker_hsv.resize(3, 0);
            }
        }

        inline i32& color_picker_axis_ref(ColorPickerState& state)
        {
            ensure_color_picker_state_channels(state);
            return state.color_picker_axis[0];
        }

        inline bool tab_order_contains(const TabBarState& state, id_t id)
        {
            for(id_t item : state.tab_order)
            {
                if(item == id) return true;
            }
            return false;
        }

        struct TabBuildScope
        {
            id_t tab_bar_id = 0;
            id_t selected_id = 0;
            id_t first_open_id = 0;
            TabBarFlag flags = TabBarFlag::none;
            bool had_existing_tabs = false;
            bool visible_tab_chosen = false;
        };

        struct TabBuildState
        {
            lustruct("GUI::TabBuildState", "{9BC38163-D6FD-436F-8087-637E952960A4}");
            Vector<TabBuildScope> stack;
        };

        inline void input_text_selection_range(const String& value, const InputEditState& state, usize& out_begin, usize& out_end)
        {
            usize cursor = clamp_utf8_cursor(value, state.text_cursor);
            usize anchor = state.text_select_anchor == USIZE_MAX ? cursor : clamp_utf8_cursor(value, state.text_select_anchor);
            out_begin = min(cursor, anchor);
            out_end = max(cursor, anchor);
        }

        inline bool input_text_has_selection(const String& value, const InputEditState& state)
        {
            usize begin = 0;
            usize end = 0;
            input_text_selection_range(value, state, begin, end);
            return begin != end;
        }

        inline void input_text_clear_selection(InputEditState& state)
        {
            state.text_select_anchor = USIZE_MAX;
            state.text_selecting = false;
        }

        struct NodeLayout
        {
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            LayoutMetrics metrics;
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
            id_t dock_space_id = 0;
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            DockPanelStyle dock_panel_style;
            u32 dock_panel_z_order = 0;
            u32 dock_leaf_index = U32_MAX;
        };

        inline RectF dock_panel_content_rect(const RectF& panel_rect, const DockPanelStyle& style)
        {
            f32 border = max(style.border_size, 0.0f);
            f32 title_height = style.title_bar ? max(style.title_bar_height, 0.0f) : 0.0f;
            return RectF(
                panel_rect.offset_x + border,
                panel_rect.offset_y + border + title_height,
                max(panel_rect.width - border * 2.0f, 1.0f),
                max(panel_rect.height - border * 2.0f - title_height, 1.0f));
        }

        inline RectF dock_panel_title_rect(const RectF& panel_rect, const DockPanelStyle& style)
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

        inline RectF dock_panel_resize_rect(const RectF& panel_rect, const DockPanelStyle& style)
        {
            f32 size = max(style.resize_border_size, 1.0f);
            return RectF(
                panel_rect.offset_x + max(panel_rect.width - size, 0.0f),
                panel_rect.offset_y + max(panel_rect.height - size, 0.0f),
                size,
                size);
        }

        inline RectF dock_panel_docked_resize_rect(const RectF& panel_rect, const DockPanelStyle& style)
        {
            f32 size = max(style.resize_border_size, 4.0f);
            return RectF(
                panel_rect.offset_x,
                panel_rect.offset_y + max(panel_rect.height - size * 0.5f, 0.0f),
                max(panel_rect.width, 1.0f),
                size);
        }

        inline f32 dock_panel_min_height(const DockPanelStyle& style)
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

        inline RectF dock_drop_icon_rect(const RectF& parent, DockDropDirection direction)
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
            case DockDropDirection::center:
                return RectF(cx - center_size * 0.5f, cy - center_size * 0.5f, center_size, center_size);
            case DockDropDirection::left:
                return RectF(cx - offset - side_h * 0.5f, cy - side_w * 0.5f, side_h, side_w);
            case DockDropDirection::right:
                return RectF(cx + offset - side_h * 0.5f, cy - side_w * 0.5f, side_h, side_w);
            case DockDropDirection::up:
                return RectF(cx - side_w * 0.5f, cy - offset - side_h * 0.5f, side_w, side_h);
            case DockDropDirection::down:
                return RectF(cx - side_w * 0.5f, cy + offset - side_h * 0.5f, side_w, side_h);
            default:
                return RectF(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        inline RectF dock_drop_preview_rect(const RectF& parent, DockDropDirection direction)
        {
            switch(direction)
            {
            case DockDropDirection::center:
                return dock_panel_content_rect(parent, DockPanelStyle());
            case DockDropDirection::left:
                return RectF(parent.offset_x, parent.offset_y, parent.width * 0.5f, parent.height);
            case DockDropDirection::right:
                return RectF(parent.offset_x + parent.width * 0.5f, parent.offset_y, parent.width * 0.5f, parent.height);
            case DockDropDirection::up:
                return RectF(parent.offset_x, parent.offset_y, parent.width, parent.height * 0.5f);
            case DockDropDirection::down:
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

        inline RectF scroll_vertical_thumb_rect(const NodeLayout& layout, const ScrollState& state)
        {
            RectF track = scroll_vertical_track_rect(layout);
            f32 ratio = layout.scroll_content_size.y > 0.0f ? clamp(layout.scroll_viewport_size.y / layout.scroll_content_size.y, 0.0f, 1.0f) : 1.0f;
            f32 thumb_height = min(max(track.height * ratio, min(scroll_min_thumb_size(), track.height)), track.height);
            f32 travel = max(track.height - thumb_height, 0.0f);
            f32 t = scroll_max_y(layout) > 0.0f ? clamp(state.scroll_y / scroll_max_y(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x, track.offset_y + travel * t, track.width, thumb_height);
        }

        inline RectF scroll_horizontal_thumb_rect(const NodeLayout& layout, const ScrollState& state)
        {
            RectF track = scroll_horizontal_track_rect(layout);
            f32 ratio = layout.scroll_content_size.x > 0.0f ? clamp(layout.scroll_viewport_size.x / layout.scroll_content_size.x, 0.0f, 1.0f) : 1.0f;
            f32 thumb_width = min(max(track.width * ratio, min(scroll_min_thumb_size(), track.width)), track.width);
            f32 travel = max(track.width - thumb_width, 0.0f);
            f32 t = scroll_max_x(layout) > 0.0f ? clamp(state.scroll_x / scroll_max_x(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x + travel * t, track.offset_y, thumb_width, track.height);
        }

        struct Context : IContext
        {
            lustruct("GUI::Context", "{BF721C36-C7C2-4B49-89E6-22F0B3BE56F5}");
            luiimpl();
            lutsassert_lock();

            Ref<RHI::IDevice> m_device;
            FrameDesc m_frame_desc;
            Description m_build_desc;
            Description m_submitted_desc;
            Vector<NodeLayout> m_layouts;
            Vector<InputEvent> m_input_events;
#ifdef LUNA_GUI_ENABLE_DEBUG
            Vector<DebugInputEventInfo> m_debug_input_events;
#endif
            Vector<u32> m_parent_stack;
            Vector<u32> m_layer_stack;
            Vector<id_t> m_id_stack;
            Vector<RectF> m_clip_stack;
            Vector<Name> m_style_stack;
            Vector<bool> m_enabled_stack;
            Vector<u32> m_child_ordinals;
            HashMap<id_t, StateRecord, IdHash> m_states;
            HashMap<Name, Style> m_styles;
            HashMap<Name, FontResource> m_fonts;
            ClipboardIO m_clipboard_io;
            id_t m_active_id = 0;
            id_t m_focused_id = 0;
            id_t m_hovered_id = 0;
            Float2U m_pointer_pos = Float2U(0.0f);
            Float2U m_pointer_delta = Float2U(0.0f);
            bool m_pointer_inside = false;
            bool m_submitted = false;
            id_t m_last_item_id = 0;
            u32 m_tree_depth = 0;
            bool m_layout_dirty = false;
            PopupStackManager m_popup_stack;
            DragDropManager m_drag_drop;
            bool m_pointer_button_down[5] = {};
            bool m_key_down[256] = {};
            KeyModifierFlag m_key_modifiers = KeyModifierFlag::none;
            u64 m_generation = 0;
            f64 m_time = 0.0;
            Ref<VG::IShapeDrawList> m_shape_draw_list;
            Ref<IDrawList> m_feedback_draw_list;
            Vector<Ref<IDrawList>> m_layer_draw_lists;
            IDrawList* m_active_draw_list = nullptr;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;

            Context();

            virtual void begin_frame(const FrameDesc& desc) override;
            virtual void add_input_event(const InputEvent& event) override;
            virtual void add_input_events(Span<const InputEvent> events) override;
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f)) override;
            virtual void pop_layer() override;
            virtual ItemHandle add_node(Ref<Node> node, const c8* label = nullptr, bool interactive = false) override;
            virtual u64 generation() const override;
            virtual object_t get_state(id_t id) override;
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) override;
            virtual void clear_state(id_t id) override;
            virtual void define_style(const Name& name, const Name& parent = Name()) override;
            virtual void set_style_parent(const Name& name, const Name& parent) override;
            virtual void set_style_value(const Name& style, const Name& entry, const StyleValue& value) override;
            virtual void inherit_style_entry(const Name& style, const Name& entry) override;
            virtual void unset_style_entry(const Name& style, const Name& entry) override;
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) override;
            virtual void push_style(const Name& style) override;
            virtual void pop_style() override;
            virtual RV register_font(const Name& id, Font::IFontFile* font, u32 font_index = 0) override;
            virtual FontDesc get_font(const Name& id) override;
            virtual void set_next_item_render_proxy(const RenderProxyDesc& proxy) override;
            virtual void set_next_item_enabled(bool enabled) override;
            virtual void push_enabled(bool enabled) override;
            virtual void pop_enabled() override;
            virtual R<Description> end_build() override;
            virtual RV submit(const Description& desc) override;
            virtual void set_clipboard_io(const ClipboardIO& io) override;
            virtual TextInputState get_text_input_state() override;
#ifdef LUNA_GUI_ENABLE_DEBUG
            virtual R<DebugInfo> dump_debug_info() override;
#endif
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) override;

            u32 current_layer_index() const;
            id_t make_node_id(id_t parent_id, const Guid& node_type, u32 ordinal, const c8* text) const;
            id_t allocate_detached_layer_id(const Guid& node_type, const c8* text);
            id_t make_popup_id(const c8* label) const;
            ItemHandle popup_handle(const c8* label);
            void push_layer_internal(id_t id, const Float2U& screen_position);
            ItemHandle add_node_internal(Ref<Node> node, const c8* text, bool interactive, id_t forced_id = 0);
            void begin_container(Ref<Node> node, const c8* label, const Size& size, ItemHandle* out_handle, id_t forced_id = 0);
            void end_container();
            bool begin_popup(const c8* label, const PopupDesc& desc, ItemHandle* out_handle = nullptr);
            void end_popup();
            void open_popup(const c8* label);
            void open_popup(ItemHandle popup);
            void close_popup(const c8* label);
            void close_popup(ItemHandle popup);
            void close_current_popup();
            void close_all_popups();
            bool is_popup_open(const c8* label) const;
            bool is_popup_open(ItemHandle popup) const;
            bool is_popup_open(id_t id) const;
            bool popup_node_visible(const Node& node) const;
            bool tooltip_node_visible(const Node& node) const;
            u32 find_submitted_node_index(id_t id) const;
            void rebuild_popup_node_indices();
            void prune_popup_stack();
            void close_popup_stack_from(usize index);
            i32 popup_stack_index(id_t id) const;
            id_t current_clicked_item_id() const;
            i32 popup_level_at_pos(const Float2U& pos) const;
            bool close_popups_for_pointer_down(const Float2U& pos);
            void open_menu_popup(id_t menu_id);
            void update_menu_hover();
            ItemHandle begin_tooltip(ItemHandle owner, const c8* label, const TooltipDesc& desc);
            void end_tooltip();
            const Any* get_item_query_state(ItemHandle handle, const Name& key);
            void set_item_query_state(ItemHandle handle, const Name& key, const Any& value);
            void set_item_query_state_if_absent(id_t id, const Name& key, const Any& value);
            void remove_item_query_state(ItemHandle handle, const Name& key);
            void set_next_item_layout(const LayoutStyle& style);
            void set_next_item_enabled_internal(bool enabled);
            void set_next_canvas_item_layout(const CanvasItemLayout& layout);
            void set_next_table_cell_color(const Float4U& color);
            void set_next_dock_panel_style(const DockPanelStyle& style, bool* open);
            bool style_parent_cycle(const Name& name, const Name& parent) const;
            void push_id(id_t id);
            void pop_id();
            void push_clip_rect(const RectF& rect);
            void pop_clip_rect();
            void tree_push();
            void tree_push(ItemHandle node);
            void tree_pop();
            bool begin_drag_drop_source(ItemHandle source, const Name& payload_type);
            void set_drag_drop_payload(const void* data, usize data_size);
            void end_drag_drop_source();
            bool begin_drag_drop_target(ItemHandle target, const Name& payload_type);
            const DragDropPayload* accept_drag_drop_payload(const Name& payload_type);
            const DragDropPayload* accept_drag_drop_payload(ItemHandle target, const Name& payload_type);
            void end_drag_drop_target();
            bool is_drag_drop_active() const;
            const DragDropPayload* get_drag_drop_payload();
            const DragDropPayload* make_drag_drop_payload_view(const DragDropPayloadStorage& storage);

            object_t get_state_object(id_t id) const;
            void gc_states();
            ItemQueryState* get_query_state(ItemHandle handle);
            Ref<ItemQueryState> get_or_create_query_state(id_t id);
            void touch_state(id_t id, StateLifetime lifetime = StateLifetime::next_frame);
            StyleValue get_style_value_unlocked(const Name& style, const Name& entry, const StyleValue& default_value) const;
            Name node_font_id(const Node& node) const;
            FontDesc resolve_font(const Name& id) const;
            LayoutMetrics measure_text_with_font(const c8* text, usize text_size, f32 font_size, f32 max_width, const Name& font_id) const;
            f32 text_cursor_x(const String& value, usize cursor, f32 font_size, const Name& font_id) const;
            usize text_cursor_from_x(const String& value, f32 x, f32 font_size, const Name& font_id) const;
            template <typename T>
            void touch_widget_state(id_t owner_id, StateLifetime lifetime = StateLifetime::next_frame)
            {
                touch_state(make_state_id<T>(owner_id), lifetime);
            }
            template <typename T>
            T& get_or_create_context_state(StateLifetime lifetime = StateLifetime::process)
            {
                return *get_or_create_widget_state<T>(0, lifetime);
            }
            template <typename T>
            const T& get_context_state() const
            {
                static T default_state;
                T* state = get_widget_state<T>(0);
                return state ? *state : default_state;
            }
            NumericInteractionState& numeric_interaction_state() { return get_or_create_context_state<NumericInteractionState>(); }
            const NumericInteractionState& numeric_interaction_state() const { return get_context_state<NumericInteractionState>(); }
            ColorPickerInteractionState& color_picker_interaction_state() { return get_or_create_context_state<ColorPickerInteractionState>(); }
            const ColorPickerInteractionState& color_picker_interaction_state() const { return get_context_state<ColorPickerInteractionState>(); }
            TableResizeInteractionState& table_resize_interaction_state() { return get_or_create_context_state<TableResizeInteractionState>(); }
            const TableResizeInteractionState& table_resize_interaction_state() const { return get_context_state<TableResizeInteractionState>(); }
            ScrollbarInteractionState& scrollbar_interaction_state() { return get_or_create_context_state<ScrollbarInteractionState>(); }
            const ScrollbarInteractionState& scrollbar_interaction_state() const { return get_context_state<ScrollbarInteractionState>(); }
            DockInteractionState& dock_interaction_state() { return get_or_create_context_state<DockInteractionState>(); }
            const DockInteractionState& dock_interaction_state() const { return get_context_state<DockInteractionState>(); }
            TabInteractionState& tab_interaction_state() { return get_or_create_context_state<TabInteractionState>(); }
            const TabInteractionState& tab_interaction_state() const { return get_context_state<TabInteractionState>(); }
            TooltipInteractionState& tooltip_interaction_state() { return get_or_create_context_state<TooltipInteractionState>(); }
            const TooltipInteractionState& tooltip_interaction_state() const { return get_context_state<TooltipInteractionState>(); }
            BuildHintState& build_hint_state() { return get_or_create_context_state<BuildHintState>(StateLifetime::current_frame); }
            TabBuildState& tab_build_state() { return get_or_create_context_state<TabBuildState>(StateLifetime::current_frame); }
            template <typename T>
            T* get_widget_state(id_t owner_id) const
            {
                object_t obj = get_state_object(make_state_id<T>(owner_id));
                return obj ? cast_object<T>(obj) : nullptr;
            }
            template <typename T>
            Ref<T> get_or_create_widget_state(id_t owner_id, StateLifetime lifetime = StateLifetime::next_frame)
            {
                id_t state_id = make_state_id<T>(owner_id);
                object_t existing = get_state_object(state_id);
                if(existing)
                {
                    Ref<T> state;
                    object_retain(existing);
                    state.attach(existing);
                    RV r = set_state(state_id, state.object(), lifetime);
                    luassert_always(succeeded(r));
                    return state;
                }
                Ref<T> state = new_object<T>();
                RV r = set_state(state_id, state.object(), lifetime);
                luassert_always(succeeded(r));
                return state;
            }
            Node* find_build_node(ItemHandle handle);
            DockPanelPersistentState& get_or_create_dock_panel_state(DockSpaceState& dock_state, id_t panel_id);
            u32 new_dock_leaf(DockSpaceState& dock_state, id_t panel_id, u32 parent = U32_MAX);
            void dock_tree_add_panel(DockSpaceState& dock_state, id_t panel_id);
            bool dock_tree_contains_panel(const DockSpaceState& dock_state, id_t panel_id) const;
            bool dock_tree_remove_panel(DockSpaceState& dock_state, id_t panel_id);
            void dock_tree_dock_panel(DockSpaceState& dock_state, id_t panel_id, u32 target_leaf, DockDropDirection direction);
            void dock_tree_prune_missing(DockSpaceState& dock_state, const HashSet<id_t, IdHash>& live_panels);
            id_t dock_tree_selected_panel(DockSpaceState& dock_state, u32 leaf_index);
            void arrange_dock_tree_node(id_t dock_space_id, u32 node_index, const RectF& rect, const RectF& clip_rect, const HashMap<id_t, u32, IdHash>& panel_indices);
            RectF layout_layer_root_rect(u32 layer_index);
            void layout_layers();
            RectF layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            LayoutMetrics measure_node(u32 node_index);
            LayoutMetrics measure_grid_node(u32 node_index, f32 available_width);
            void measure_table_tracks(u32 node_index, Vector<f32>& out_column_widths, Vector<f32>& out_row_heights, bool preferred);
            void arrange_table_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_grid_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_canvas_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_tab_bar_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            void arrange_dock_space_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            bool hit_test_table_separator(const Float2U& pos, id_t& out_id, bool& out_column, u32& out_index) const;
            void update_table_resize_from_pointer(const Float2U& pos);
            bool hit_test_dock_panel_layer(u32 layer_index, const Float2U& pos) const;
            bool hit_test_dock_panel(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id) const;
            bool hit_test_dock_panel_chrome(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id, bool& out_resize, bool& out_close) const;
            void update_dock_panel_from_pointer(const Float2U& pos);
            bool hit_test_dock_panel_tab(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id, u32& out_leaf_index) const;
            bool hit_test_dock_splitter(const Float2U& pos, id_t& out_space_id, u32& out_node_index, DockSplitAxis& out_axis) const;
            void update_dock_splitter_from_pointer(const Float2U& pos);
            bool find_dock_drop_target(id_t payload_panel, const Float2U& pos, id_t& out_space_id, u32& out_leaf_index, DockDropDirection& out_direction) const;
            void render_dock_preview();
            void raise_dock_panel(id_t dock_space_id, id_t panel_id);
            DockPanelPersistentState* find_dock_panel_state(id_t dock_space_id, id_t panel_id);
            void clamp_scroll_state(id_t id);
            bool hit_test_scrollbar(const Float2U& pos, id_t& out_id, bool& out_vertical, RectF& out_thumb_rect) const;
            void update_scrollbar_from_pointer(const Float2U& pos);
            id_t hit_test_drag_drop_source(const Float2U& pos, Name& out_type) const;
            id_t hit_test_drag_drop_target(const Name& type, const Float2U& pos) const;
            void start_drag_drop(id_t source_id, const Name& type);
            void clear_drag_drop();
            void deliver_drag_drop_payload(id_t target_id);
            bool hit_test_tab_header(const Float2U& pos, id_t& out_tab_bar_id, id_t& out_tab_item_id, bool& out_close) const;
            bool hit_test_tab_scroll_button(const Float2U& pos, id_t& out_tab_bar_id, bool& out_left) const;
            id_t hit_test_tab_scroll_area(const Float2U& pos) const;
            void select_tab_item(id_t tab_bar_id, id_t tab_item_id);
            id_t fallback_tab_item(id_t tab_bar_id, id_t excluded_tab_item_id) const;
            bool reorder_tab_item_from_pointer(id_t tab_bar_id, id_t tab_item_id, const Float2U& pos);
            void scroll_tab_bar(id_t tab_bar_id, f32 delta);
            id_t hit_test_node(u32 node_index, const Float2U& pos, HitTestFilter filter) const;
            u32 hit_test_layer_index(const Float2U& pos) const;
            id_t hit_test(const Float2U& pos) const;
            id_t hit_test_filtered(const Float2U& pos, HitTestFilter filter) const;
            Node* find_node(id_t id);
            u32 hit_test_numeric_component(const Node& node, const RectF& rect, const Float2U& pos) const;
            void update_numeric_node_from_pointer(id_t id, const Float2U& pos, const Float2U* old_pos = nullptr);
            void update_color_picker_from_pointer(id_t id, const Float2U& pos);
            void sync_color_picker_numeric_state(id_t owner_id);
            void apply_color_picker_numeric_state(id_t owner_id, ColorChannelPart part);
            bool input_text_cursor_from_pointer(id_t id, const Float2U& pos, usize& out_cursor);
            bool update_input_text_selection_from_pointer(id_t id, const Float2U& pos);
            bool numeric_text_cursor_from_pointer(id_t id, const Float2U& pos, usize& out_cursor);
            bool update_numeric_text_selection_from_pointer(id_t id, const Float2U& pos);
            void begin_numeric_text_edit(id_t id, const Float2U& pos, u32 component, bool select_all);
            void mark_value_changed(id_t id);
            void process_input_events();
            void render_node(u32 node_index);
            void render_drag_drop_overlay();
            void render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                RHI::ITexture* texture = nullptr, ImageFlag image_flags = ImageFlag::none);
            void render_gradient_rect(const RectF& rect, const RectF& clip_rect,
                const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left);
            void render_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                bool top_left, bool top_right, bool bottom_right, bool bottom_left);
            void render_color_swatch(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius);
            void render_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color);
            void render_line_segment(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width);
            void render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color,
                VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment = VG::TextAlignment::center, const Name& font_id = Name());
            RectF to_vg_rect(const RectF& rect) const;
        };

    }
}
