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
        GUIItemHandle GUIContext::begin_tooltip(GUIItemHandle owner, const c8* label, const GUITooltipDesc& desc)
        {
            GUIItemHandle handle;
            begin_container(GUINodeKind::tooltip, label ? label : "Tooltip", desc.size, &handle);
            GUINode& node = m_build_desc.nodes.back();
            node.render_layer = GUIRenderLayer::overlay;
            node.absolute_position = true;
            node.popup_owner_id = owner.context == get_object() ? owner.id : 0;
            node.tooltip_desc = desc;
            node.layout_desc.padding = GUIEdgeInsets::xy(8.0f, 6.0f);
            node.layout_desc.gap = 4.0f;
            return handle;
        }

        void GUIContext::end_tooltip()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            const GUINode& node = m_build_desc.nodes[m_parent_stack.back()];
            luassert(node.kind == GUINodeKind::tooltip);
            end_container();
        }

        bool GUIContext::tooltip_node_visible(const GUINode& node) const
        {
            if(node.kind != GUINodeKind::tooltip) return true;
            if(!node.popup_owner_id || !m_pointer_inside) return false;
            if(m_drag_drop_active || m_active_id) return false;
            if(m_hovered_id != node.popup_owner_id || m_tooltip_hovered_id != node.popup_owner_id) return false;
            return m_time - m_tooltip_hover_start >= max((f64)node.tooltip_desc.delay, 0.0);
        }

        LUNA_GUI_API GUIItemHandle BeginTooltip(GUIItemHandle owner, const c8* label, const GUITooltipDesc& desc)
        {
            return require_current_context()->begin_tooltip(owner, label, desc);
        }

        LUNA_GUI_API void EndTooltip()
        {
            require_current_context()->end_tooltip();
        }

        LUNA_GUI_API GUIItemHandle SetItemTooltip(GUIItemHandle owner, const c8* text, const GUITooltipDesc& desc)
        {
            GUIItemHandle handle = BeginTooltip(owner, "Tooltip", desc);
            Text(text ? text : "");
            EndTooltip();
            return handle;
        }
    }
}
