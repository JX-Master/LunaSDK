/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITooltip.cpp
* @author JXMaster
* @date 2026/5/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        ItemHandle Context::begin_tooltip(ItemHandle owner, const c8* label, const TooltipDesc& desc)
        {
            ItemHandle handle;
            id_t layer_id = allocate_detached_layer_id(TooltipNode::__guid, label ? label : "Tooltip");
            push_layer_internal(layer_id, Float2U(0.0f));
            begin_container(Ref<Node>(new_object<TooltipNode>()), label ? label : "Tooltip", desc.size, &handle, layer_id);
            Node& node = m_build_desc.nodes.back();
            set_popup_owner(node, owner.context == get_object() ? owner.id : 0);
            TooltipNode* tooltip = tooltip_node(node);
            luassert(tooltip);
            tooltip->desc = desc;
            node.layout_desc.padding = EdgeInsets::xy(8.0f, 6.0f);
            node.layout_desc.gap = 4.0f;
            return handle;
        }

        void Context::end_tooltip()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            const Node& node = m_build_desc.nodes[m_parent_stack.back()];
            luassert(tooltip_layer(node));
            end_container();
            pop_layer();
        }

        bool Context::tooltip_node_visible(const Node& node) const
        {
            if(!tooltip_layer(node)) return true;
            id_t owner = popup_owner(node);
            if(!owner || !m_pointer_inside) return false;
            if(m_drag_drop_active || m_active_id) return false;
            if(m_hovered_id != owner || m_tooltip_hovered_id != owner) return false;
            return m_time - m_tooltip_hover_start >= max((f64)tooltip_desc(node).delay, 0.0);
        }

        LUNA_GUI_API ItemHandle begin_tooltip(IContext* context, ItemHandle owner, const c8* label, const TooltipDesc& desc)
        {
            return context_from_interface(context)->begin_tooltip(owner, label, desc);
        }

        LUNA_GUI_API void end_tooltip(IContext* context)
        {
            context_from_interface(context)->end_tooltip();
        }

        LUNA_GUI_API ItemHandle set_item_tooltip(IContext* context, ItemHandle owner, const c8* content, const TooltipDesc& desc)
        {
            ItemHandle handle = begin_tooltip(context, owner, "Tooltip", desc);
            text(context, content ? content : "");
            end_tooltip(context);
            return handle;
        }
    }
}
