/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIPopup.cpp
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
        namespace
        {
            const GUINode* find_popup_node_in_desc(const GUIDescription& desc, GUIID id)
            {
                for(const GUINode& node : desc.nodes)
                {
                    if(node.id == id && node.kind == GUINodeKind::popup)
                    {
                        return &node;
                    }
                }
                return nullptr;
            }
        }

        GUIItemHandle GUIContext::begin_popup(const c8* label, const GUIPopupDesc& desc)
        {
            GUIItemHandle handle;
            begin_container(GUINodeKind::popup, label ? label : "Popup", desc.size, &handle);
            GUINode& node = m_build_desc.nodes.back();
            node.render_layer = GUIRenderLayer::overlay;
            node.absolute_position = true;
            node.position = desc.position;
            node.popup_flags = desc.flags;
            node.popup_parent_id = m_popup_build_stack.empty() ? 0 : m_popup_build_stack.back();
            node.layout_desc.padding = GUIEdgeInsets::all(6.0f);
            node.layout_desc.gap = 2.0f;
            m_popup_build_stack.push_back(handle.id);
            return handle;
        }

        void GUIContext::end_popup()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            const GUINode& node = m_build_desc.nodes[m_parent_stack.back()];
            luassert(node.kind == GUINodeKind::popup);
            if(!m_popup_build_stack.empty() && m_popup_build_stack.back() == node.id)
            {
                m_popup_build_stack.pop_back();
            }
            end_container();
        }

        i32 GUIContext::popup_stack_index(GUIID id) const
        {
            for(usize i = 0; i < m_open_popup_stack.size(); ++i)
            {
                if(m_open_popup_stack[i].id == id)
                {
                    return (i32)i;
                }
            }
            return -1;
        }

        bool GUIContext::is_popup_open(GUIID id) const
        {
            return popup_stack_index(id) >= 0;
        }

        bool GUIContext::is_popup_open(GUIItemHandle popup) const
        {
            if(!popup.id) return false;
            return is_popup_open(popup.id);
        }

        bool GUIContext::popup_node_visible(const GUINode& node) const
        {
            if(node.kind != GUINodeKind::popup) return true;
            if(!test_flags(node.popup_flags, GUIPopupFlag::managed)) return true;
            return is_popup_open(node.id);
        }

        u32 GUIContext::find_submitted_node_index(GUIID id) const
        {
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    return (u32)i;
                }
            }
            return U32_MAX;
        }

        void GUIContext::rebuild_popup_node_indices()
        {
            m_popup_node_indices.clear();
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].kind == GUINodeKind::popup)
                {
                    m_popup_node_indices.insert_or_assign(m_submitted_desc.nodes[i].id, (u32)i);
                }
            }
        }

        void GUIContext::close_popup_stack_from(usize index)
        {
            if(index >= m_open_popup_stack.size()) return;
            for(usize i = index; i < m_open_popup_stack.size(); ++i)
            {
                GUIID id = m_open_popup_stack[i].id;
                PersistentItemState& persistent = get_or_create_persistent_state(id);
                persistent.open = false;
                ItemResult& result = get_or_create_current_result(id);
                result.states.insert_or_assign(Name("gui.open"), Any(false));
            }
            m_open_popup_stack.erase(m_open_popup_stack.begin() + index, m_open_popup_stack.end());
            m_layout_dirty = true;
        }

        void GUIContext::prune_popup_stack()
        {
            for(usize i = 0; i < m_open_popup_stack.size();)
            {
                auto iter = m_popup_node_indices.find(m_open_popup_stack[i].id);
                if(iter == m_popup_node_indices.end())
                {
                    close_popup_stack_from(i);
                    break;
                }
                const GUINode& node = m_submitted_desc.nodes[iter->second];
                if(!test_flags(node.popup_flags, GUIPopupFlag::managed))
                {
                    close_popup_stack_from(i);
                    break;
                }
                if(node.popup_parent_id)
                {
                    auto parent_iter = m_popup_node_indices.find(node.popup_parent_id);
                    if(parent_iter != m_popup_node_indices.end() &&
                        !popup_node_visible(m_submitted_desc.nodes[parent_iter->second]))
                    {
                        close_popup_stack_from(i);
                        break;
                    }
                }
                m_open_popup_stack[i].parent_id = node.popup_parent_id;
                m_open_popup_stack[i].flags = node.popup_flags;
                PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                persistent.open = true;
                ++i;
            }
        }

        void GUIContext::open_popup(GUIItemHandle popup)
        {
            lutsassert();
            if(!popup.id || popup.context != get_object()) return;

            const GUINode* node = find_popup_node_in_desc(m_submitted_desc, popup.id);
            if(!node)
            {
                node = find_popup_node_in_desc(m_build_desc, popup.id);
            }

            GUIPopupFlag flags = GUIPopupFlag::managed | GUIPopupFlag::close_on_outside_click | GUIPopupFlag::close_on_escape | GUIPopupFlag::close_on_blur;
            GUIID parent_id = 0;
            if(node)
            {
                flags = node->popup_flags;
                parent_id = node->popup_parent_id;
                if(!test_flags(flags, GUIPopupFlag::managed))
                {
                    flags |= GUIPopupFlag::managed | GUIPopupFlag::close_on_outside_click | GUIPopupFlag::close_on_escape | GUIPopupFlag::close_on_blur;
                }
            }

            i32 existing = popup_stack_index(popup.id);
            if(existing >= 0)
            {
                close_popup_stack_from((usize)existing + 1);
                PersistentItemState& persistent = get_or_create_persistent_state(popup.id);
                persistent.open = true;
                ItemResult& result = get_or_create_current_result(popup.id);
                result.states.insert_or_assign(Name("gui.open"), Any(true));
                return;
            }

            if(parent_id)
            {
                i32 parent_index = popup_stack_index(parent_id);
                bool parent_visible = parent_index >= 0;
                if(!parent_visible)
                {
                    auto parent_iter = m_popup_node_indices.find(parent_id);
                    parent_visible = parent_iter != m_popup_node_indices.end() && popup_node_visible(m_submitted_desc.nodes[parent_iter->second]);
                }
                if(parent_index >= 0)
                {
                    close_popup_stack_from((usize)parent_index + 1);
                }
                else if(!parent_visible)
                {
                    close_all_popups();
                    return;
                }
            }
            else
            {
                close_all_popups();
            }

            PopupStackEntry entry;
            entry.id = popup.id;
            entry.parent_id = parent_id;
            entry.flags = flags;
            m_open_popup_stack.push_back(entry);
            PersistentItemState& persistent = get_or_create_persistent_state(popup.id);
            persistent.open = true;
            ItemResult& result = get_or_create_current_result(popup.id);
            result.states.insert_or_assign(Name("gui.open"), Any(true));
            m_layout_dirty = true;
        }

        void GUIContext::close_popup(GUIItemHandle popup)
        {
            lutsassert();
            if(!popup.id || popup.context != get_object()) return;
            i32 index = popup_stack_index(popup.id);
            if(index >= 0)
            {
                close_popup_stack_from((usize)index);
            }
            else
            {
                PersistentItemState& persistent = get_or_create_persistent_state(popup.id);
                persistent.open = false;
                ItemResult& result = get_or_create_current_result(popup.id);
                result.states.insert_or_assign(Name("gui.open"), Any(false));
            }
        }

        void GUIContext::close_current_popup()
        {
            lutsassert();
            if(m_open_popup_stack.empty()) return;
            close_popup_stack_from(m_open_popup_stack.size() - 1);
        }

        void GUIContext::close_all_popups()
        {
            lutsassert();
            close_popup_stack_from(0);
        }

        i32 GUIContext::popup_level_at_pos(const Float2U& pos) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return -1;
            for(usize i = m_open_popup_stack.size(); i > 0; --i)
            {
                usize level = i - 1;
                auto iter = m_popup_node_indices.find(m_open_popup_stack[level].id);
                if(iter == m_popup_node_indices.end()) continue;
                const NodeLayout& layout = m_layouts[iter->second];
                if(point_in_rect(pos, layout.rect) && point_in_rect(pos, layout.clip_rect))
                {
                    return (i32)level;
                }
            }
            return -1;
        }

        bool GUIContext::close_popups_for_pointer_down(const Float2U& pos)
        {
            if(m_open_popup_stack.empty()) return false;
            i32 level = popup_level_at_pos(pos);
            if(level < 0)
            {
                GUIID target = hit_test(pos);
                GUINode* target_node = target ? find_node(target) : nullptr;
                bool menu_target = target_node && target_node->kind == GUINodeKind::menu;
                GUIPopupFlag flags = m_open_popup_stack.back().flags;
                if(menu_target && target_node->menu_popup_id && is_popup_open(target_node->menu_popup_id))
                {
                    close_popup(GUIItemHandle{get_object(), target_node->menu_popup_id, m_generation});
                    return true;
                }
                if(test_flags(flags, GUIPopupFlag::close_on_outside_click))
                {
                    close_all_popups();
                }
                return !menu_target;
            }
            if((usize)level + 1 < m_open_popup_stack.size())
            {
                close_popup_stack_from((usize)level + 1);
            }
            return false;
        }
    }
}
