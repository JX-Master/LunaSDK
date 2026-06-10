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
        static ItemHandle add_flow_text_node(Context* ctx, const c8* text_value, f32 font_size, const Float4U& color = Float4U(1.0f))
        {
            Ref<TextNode> node = new_object<TextNode>();
            node->font_size = font_size;
            node->color = color;
            return ctx->add_node(Ref<Node>(node), text_value ? text_value : "", false);
        }

        static const c8* combo_selected_text(i32* current_item, Span<const c8*> items)
        {
            if(!current_item || *current_item < 0 || (usize)*current_item >= items.size())
            {
                return "";
            }
            const c8* item = items[(usize)*current_item];
            return item ? item : "";
        }

        static ItemHandle combo_preview(IContext* context, const c8* label, i32* current_item, Span<const c8*> items, bool open)
        {
            Context* ctx = context_from_interface(context);
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            begin_h_layout(context, label && label[0] ? label : "Combo", row);

            if(label && label[0])
            {
                set_next_item_layout(context, LayoutStyle::fixed_width(120.0f));
                add_flow_text_node(ctx, label, 16.0f);
            }

            Ref<SelectableNode> preview_node = new_object<SelectableNode>();
            preview_node->label_layout = true;
            preview_node->selected = open;
            set_next_item_layout(context, LayoutStyle::fill_width());
            ItemHandle handle;
            Size preview_size;
            preview_size.height = 30.0f;
            ctx->begin_container(Ref<Node>(preview_node), "##ComboPreview", preview_size, &handle);
            Node& preview = ctx->m_build_desc.nodes.back();
            preview.layout_desc.padding = EdgeInsets::xy(8.0f, 3.0f);
            preview.layout_desc.gap = 8.0f;
            preview.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::center;

            set_next_item_layout(context, LayoutStyle::fill_width());
            add_flow_text_node(ctx, combo_selected_text(current_item, items), 16.0f, Float4U(0.94f, 0.97f, 1.0f, 1.0f));

            set_next_item_layout(context, LayoutStyle::fixed(16.0f, 16.0f));
            Float4U arrow_color(0.94f, 0.97f, 1.0f, 1.0f);
            add_flow_text_node(ctx, open ? "^" : "v", 15.0f, arrow_color);

            ctx->end_container();
            ctx->m_last_item_id = handle.id;
            end_h_layout(context);
            ctx->m_last_item_id = handle.id;
            return handle;
        }

        LUNA_GUI_API ItemHandle combo(IContext* context, const c8* label, i32* current_item, Span<const c8*> items)
        {
            Context* ctx = context_from_interface(context);
            if(current_item && !items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)items.size() - 1);
            }

            bool open = false;
            ItemHandle handle = combo_preview(context, label, current_item, items, open);
            bool clicked = is_item_clicked(handle);

            PopupDesc popup_desc;
            popup_desc.size = Size::fixed(220.0f, max(26.0f, min((f32)items.size() * 26.0f + 10.0f, 320.0f)));
            popup_desc.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            ctx->push_id(handle.id);
            if(clicked)
            {
                if(ctx->is_popup_open("##ComboPopup"))
                {
                    ctx->close_popup("##ComboPopup");
                    open = false;
                }
                else if(current_item && !items.empty())
                {
                    ctx->open_popup("##ComboPopup");
                    open = true;
                }
            }

            ItemHandle popup;
            if(ctx->begin_popup("##ComboPopup", popup_desc, &popup))
            {
                Node& popup_node = ctx->m_build_desc.nodes.back();
                set_popup_owner(popup_node, handle.id);
                popup_node.layout_desc.padding = EdgeInsets::xy(6.0f, 5.0f);
                popup_node.layout_desc.gap = 1.0f;
                popup_node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;

                Ref<PopupAnchorState> popup_state = ctx->get_or_create_widget_state<PopupAnchorState>(popup.id);
                popup_state->popup_anchor_placement = PopupAnchorPlacement::owner_down;
                popup_state->popup_anchor_valid = true;

                for(usize i = 0; i < items.size(); ++i)
                {
                    const c8* item_text = items[i] ? items[i] : "";
                    push_id(context, (u64)i);
                    ItemHandle item = selectable(context, item_text, current_item && *current_item == (i32)i);
                    pop_id(context);
                    if(is_item_clicked(item))
                    {
                        if(current_item && *current_item != (i32)i)
                        {
                            *current_item = (i32)i;
                            ctx->get_or_create_query_state(handle.id)->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                        }
                        ctx->close_popup(popup);
                    }
                }

                ctx->end_popup();
            }
            open = ctx->is_popup_open(popup);
            ctx->pop_id();

            Ref<ItemQueryState> result = ctx->get_or_create_query_state(handle.id);
            result->states.insert_or_assign(Name("gui.open"), Any(open));
            if(Node* preview_node = ctx->find_build_node(handle))
            {
                if(preview_node->type_guid() == Meta::StructMetaData<SelectableNode>::__guid)
                {
                    ((SelectableNode*)preview_node)->selected = open;
                }
            }
            ctx->m_last_item_id = handle.id;
            return handle;
        }
    }
}
