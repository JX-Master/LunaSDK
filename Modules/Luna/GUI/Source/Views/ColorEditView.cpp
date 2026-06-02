/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static void assign_color_binding(ColorBinding& binding, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count)
        {
            binding.f32_value = f32_value;
            binding.u8_value = u8_value;
            binding.u32_value = u32_value;
            binding.type = type;
            binding.value_count = count;
        }

        static void sync_color_picker_build_state(ColorPickerState& state, const Float4U& color)
        {
            ensure_color_picker_state_channels(state);
            state.color_picker_rgb[0] = (i32)color_channel_to_u8(color.x);
            state.color_picker_rgb[1] = (i32)color_channel_to_u8(color.y);
            state.color_picker_rgb[2] = (i32)color_channel_to_u8(color.z);
            state.color_picker_rgb[3] = (i32)color_channel_to_u8(color.w);
            f32 h = 0.0f;
            f32 s = 0.0f;
            f32 v = 0.0f;
            color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
            state.color_picker_hsv[0] = (i32)color_channel_to_u8(h);
            state.color_picker_hsv[1] = (i32)color_channel_to_u8(s);
            state.color_picker_hsv[2] = (i32)color_channel_to_u8(v);
        }

        static String color_edit_hex_text(const ColorBinding& binding)
        {
            Float4U color = read_color_value(binding);
            String hex;
            if(color_value_count(binding) > 3)
            {
                strprintf(hex, "#%02X%02X%02X%02X", color_channel_to_u8(color.x), color_channel_to_u8(color.y), color_channel_to_u8(color.z), color_channel_to_u8(color.w));
            }
            else
            {
                strprintf(hex, "#%02X%02X%02X", color_channel_to_u8(color.x), color_channel_to_u8(color.y), color_channel_to_u8(color.z));
            }
            return hex;
        }

        static ItemHandle add_flow_draw_rect_node(Context* ctx, const c8* label, const Size& size, const Float4U& color, f32 radius)
        {
            Ref<DrawRectNode> node = new_object<DrawRectNode>();
            node->color = color;
            node->radius = radius;
            apply_requested_size(*node, size);
            return ctx->add_node(Ref<Node>(node), label ? label : "DrawRect", false);
        }

        static ItemHandle add_flow_text_node(Context* ctx, const c8* text_value, f32 font_size, const Float4U& color = Float4U(1.0f))
        {
            Ref<TextNode> node = new_object<TextNode>();
            node->font_size = font_size;
            node->color = color;
            return ctx->add_node(Ref<Node>(node), text_value ? text_value : "", false);
        }

        static ItemHandle add_color_edit_preview(IContext* context, const c8* label, const ColorBinding& binding)
        {
            Context* ctx = context_from_interface(context);
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ItemHandle row_handle = begin_h_layout(context, label && label[0] ? label : "ColorEdit", row);

            if(label && label[0])
            {
                set_next_item_layout(context, LayoutStyle::fixed_width(120.0f));
                add_flow_text_node(ctx, label, 16.0f);
            }

            Ref<SelectableNode> preview_node = new_object<SelectableNode>();
            preview_node->label_layout = true;
            set_next_item_layout(context, LayoutStyle::fill_width());
            ItemHandle handle;
            Size preview_size;
            preview_size.height = 30.0f;
            ctx->begin_container(Ref<Node>(preview_node), "##ColorEditPreview", preview_size, &handle);
            Node& preview = ctx->m_build_desc.nodes.back();
            preview.layout_desc.padding = EdgeInsets::xy(4.0f, 3.0f);
            preview.layout_desc.gap = 8.0f;
            preview.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::center;

            set_next_item_layout(context, LayoutStyle::fixed(22.0f, 22.0f));
            add_flow_draw_rect_node(ctx, "Swatch", Size::fixed(22.0f, 22.0f), read_color_value(binding), 4.0f);
            String hex = color_edit_hex_text(binding);
            set_next_item_layout(context, LayoutStyle::fill_width());
            add_flow_text_node(ctx, hex.c_str(), 15.0f, Float4U(0.86f, 0.90f, 0.96f, 1.0f));
            ctx->end_container();
            ctx->m_last_item_id = handle.id;

            end_h_layout(context);
            ctx->m_last_item_id = handle.id;
            (void)row_handle;
            return handle;
        }

        static ItemHandle add_color_picker_node(IContext* context, const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count, id_t owner_id)
        {
            Context* ctx = context_from_interface(context);
            Ref<ColorPickerNode> picker_node = new_object<ColorPickerNode>();
            assign_color_binding(picker_node->binding, f32_value, u8_value, u32_value, type, count);
            picker_node->binding.owner_id = owner_id;
            return ctx->add_node(Ref<Node>(picker_node), label ? label : "ColorPicker", true);
        }

        static void tag_color_numeric_node(Context* ctx, ItemHandle handle, id_t owner_id, ColorChannelPart part)
        {
            if(Node* node = ctx->find_build_node(handle))
            {
                if(!numeric_drag(*node) || !numeric_value_i32(*node)) return;
                DragIntNode* drag_node = (DragIntNode*)node;
                drag_node->binding.color_owner_id = owner_id;
                drag_node->binding.color_part = part;
            }
        }

        static ItemHandle add_color_channel_drag(IContext* context, const c8* label, i32* value, id_t owner_id, ColorChannelPart part)
        {
            Context* ctx = context_from_interface(context);
            ItemHandle handle = drag_int(context, label, value, 1.0f, 0, 255, NumericEditFlag::input_on_double_click);
            tag_color_numeric_node(ctx, handle, owner_id, part);
            return handle;
        }

        static ItemHandle add_color_edit_view(IContext* context, const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count)
        {
            Context* ctx = context_from_interface(context);
            ColorBinding binding;
            assign_color_binding(binding, f32_value, u8_value, u32_value, type, count);
            write_color_value(binding, read_color_value(binding));

            ItemHandle handle = add_color_edit_preview(context, label, binding);
            bool clicked = is_item_clicked(handle);

            PopupDesc popup_desc;
            popup_desc.size = Size::fixed(476.0f, count == 4 ? 470.0f : 432.0f);
            popup_desc.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            ctx->push_id(handle.id);
            ItemHandle popup = ctx->begin_popup("##ColorEditPopup", popup_desc);
            Node& popup_node = ctx->m_build_desc.nodes.back();
            popup_node.set_popup_owner(handle.id);
            popup_node.layout_desc.padding = EdgeInsets::all(10.0f);
            popup_node.layout_desc.gap = 8.0f;
            popup_node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;

            set_next_item_layout(context, LayoutStyle::fixed_height(300.0f));
            ItemHandle picker = add_color_picker_node(context, "##ColorPicker", f32_value, u8_value, u32_value, type, count, 0);
            Ref<ColorPickerState> color_state = ctx->get_or_create_widget_state<ColorPickerState>(picker.id);
            color_picker_axis_ref(*color_state) = clamp(color_picker_axis_ref(*color_state), 0, 5);
            ensure_color_picker_state_channels(*color_state);
            sync_color_picker_build_state(*color_state, read_color_value(binding));

            if(clicked)
            {
                Ref<PopupAnchorState> popup_state = ctx->get_or_create_widget_state<PopupAnchorState>(popup.id);
                popup_state->popup_anchor_position = get_pointer_position(context);
                popup_state->popup_anchor_placement = PopupAnchorPlacement::pointer;
                popup_state->popup_anchor_valid = true;
                if(ctx->is_popup_open(popup.id))
                {
                    ctx->close_popup(popup);
                    color_state->color_picker_original_valid = false;
                    popup_state->popup_anchor_valid = false;
                }
                else
                {
                    color_state->color_picker_original = read_color_value(binding);
                    color_state->color_picker_original_valid = true;
                    ctx->open_popup(popup);
                }
            }
            ctx->get_or_create_query_state(handle.id)->states.insert_or_assign(Name("gui.open"), Any(ctx->is_popup_open(popup.id)));

            const c8* axis_items[] = { "H", "S", "V", "R", "G", "B" };
            set_next_item_layout(context, LayoutStyle::fixed_height(28.0f));
            button_group(context, "Channel", &color_picker_axis_ref(*color_state), Span<const c8*>(axis_items, 6));

            LayoutDesc row;
            row.gap = 6.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
            begin_h_layout(context, "RGB", row);
            add_color_channel_drag(context, "R", &color_state->color_picker_rgb[0], picker.id, ColorChannelPart::rgb);
            add_color_channel_drag(context, "G", &color_state->color_picker_rgb[1], picker.id, ColorChannelPart::rgb);
            add_color_channel_drag(context, "B", &color_state->color_picker_rgb[2], picker.id, ColorChannelPart::rgb);
            end_h_layout(context);

            set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
            begin_h_layout(context, "HSV", row);
            add_color_channel_drag(context, "H", &color_state->color_picker_hsv[0], picker.id, ColorChannelPart::hsv);
            add_color_channel_drag(context, "S", &color_state->color_picker_hsv[1], picker.id, ColorChannelPart::hsv);
            add_color_channel_drag(context, "V", &color_state->color_picker_hsv[2], picker.id, ColorChannelPart::hsv);
            end_h_layout(context);

            if(count == 4)
            {
                set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
                begin_h_layout(context, "Alpha", row);
                add_color_channel_drag(context, "A", &color_state->color_picker_rgb[3], picker.id, ColorChannelPart::rgb);
                end_h_layout(context);
            }

            ctx->end_popup();
            ctx->pop_id();
            ctx->m_last_item_id = handle.id;
            return handle;
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, f32* value)
        {
            return add_color_edit_view(context, label, value, nullptr, nullptr, ColorValueType::f32, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, f32* value)
        {
            return add_color_edit_view(context, label, value, nullptr, nullptr, ColorValueType::f32, 4);
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u8* value)
        {
            return add_color_edit_view(context, label, nullptr, value, nullptr, ColorValueType::u8, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u8* value)
        {
            return add_color_edit_view(context, label, nullptr, value, nullptr, ColorValueType::u8, 4);
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u32* value)
        {
            return add_color_edit_view(context, label, nullptr, nullptr, value, ColorValueType::rgba8, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u32* value)
        {
            return add_color_edit_view(context, label, nullptr, nullptr, value, ColorValueType::rgba8, 4);
        }
    }
}
