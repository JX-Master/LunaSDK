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
            const Node* find_popup_node_in_desc(const Description& desc, id_t id)
            {
                for(const Node& node : desc.nodes)
                {
                    if(node.id == id && popup_layer(node))
                    {
                        return &node;
                    }
                }
                return nullptr;
            }

            const PopupBuildInfo* find_popup_info(const PopupStackManager& popup_stack, id_t id)
            {
                auto build_iter = popup_stack.build_infos.find(id);
                if(build_iter != popup_stack.build_infos.end())
                {
                    return &build_iter->second;
                }
                auto submitted_iter = popup_stack.submitted_infos.find(id);
                return submitted_iter == popup_stack.submitted_infos.end() ? nullptr : &submitted_iter->second;
            }
        }

        id_t Context::make_popup_id(const c8* label) const
        {
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            id_t parent_id = 0;
            if(parent == U32_MAX)
            {
                u32 layer_index = current_layer_index();
                parent_id = m_build_desc.layers[layer_index].id;
            }
            else
            {
                parent_id = m_build_desc.nodes[parent].id;
            }
            return make_node_id(parent_id, PopupNode::__guid, 0, label ? label : "Popup");
        }

        ItemHandle Context::popup_handle(const c8* label)
        {
            return ItemHandle{get_object(), make_popup_id(label), m_generation};
        }

        bool Context::begin_popup(const c8* label, const PopupDesc& desc, ItemHandle* out_handle)
        {
            ItemHandle handle = popup_handle(label);
            if(out_handle)
            {
                *out_handle = handle;
            }
            PopupBuildInfo info;
            info.parent_id = m_popup_stack.build_stack.empty() ? 0 : m_popup_stack.build_stack.back();
            info.flags = desc.flags;
            m_popup_stack.build_infos.insert_or_assign(handle.id, info);
            i32 existing = popup_stack_index(handle.id);
            if(existing >= 0)
            {
                m_popup_stack.open_stack[(usize)existing].parent_id = info.parent_id;
                m_popup_stack.open_stack[(usize)existing].flags = info.flags;
            }
            if(test_flags(desc.flags, PopupFlag::managed) && existing < 0)
            {
                return false;
            }
            id_t layer_id = handle.id;
            push_layer_internal(layer_id, desc.position);
            Ref<PopupNode> popup_node = new_object<PopupNode>();
            popup_node->flags = desc.flags;
            popup_node->parent_popup = info.parent_id;
            begin_container(Ref<Node>(popup_node), label ? label : "Popup", desc.size, &handle, layer_id);
            Node& node = m_build_desc.nodes.back();
            node.layout_desc.padding = EdgeInsets::all(6.0f);
            node.layout_desc.gap = 2.0f;
            m_popup_stack.build_stack.push_back(handle.id);
            return true;
        }

        void Context::end_popup()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            const Node& node = m_build_desc.nodes[m_parent_stack.back()];
            luassert(popup_layer(node));
            if(!m_popup_stack.build_stack.empty() && m_popup_stack.build_stack.back() == node.id)
            {
                m_popup_stack.build_stack.pop_back();
            }
            end_container();
            pop_layer();
        }

        i32 Context::popup_stack_index(id_t id) const
        {
            for(usize i = 0; i < m_popup_stack.open_stack.size(); ++i)
            {
                if(m_popup_stack.open_stack[i].id == id)
                {
                    return (i32)i;
                }
            }
            return -1;
        }

        id_t Context::current_clicked_item_id() const
        {
            for(const Node& node : m_submitted_desc.nodes)
            {
                const ItemQueryState* state = get_widget_state<ItemQueryState>(node.id);
                if(!state) continue;
                auto iter = state->states.find(Name("gui.clicked"));
                if(iter == state->states.end()) continue;
                const bool* clicked = iter->second.as<bool>();
                if(clicked && *clicked) return node.id;
            }
            return 0;
        }

        bool Context::is_popup_open(id_t id) const
        {
            return popup_stack_index(id) >= 0;
        }

        bool Context::is_popup_open(ItemHandle popup) const
        {
            if(!popup.id) return false;
            return is_popup_open(popup.id);
        }

        bool Context::is_popup_open(const c8* label) const
        {
            return is_popup_open(make_popup_id(label));
        }

        bool Context::popup_node_visible(const Node& node) const
        {
            if(!popup_layer(node)) return true;
            if(!test_flags(popup_flags(node), PopupFlag::managed)) return true;
            return is_popup_open(node.id);
        }

        u32 Context::find_submitted_node_index(id_t id) const
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

        void Context::rebuild_popup_node_indices()
        {
            m_popup_stack.node_indices.clear();
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(popup_layer(m_submitted_desc.nodes[i]))
                {
                    m_popup_stack.node_indices.insert_or_assign(m_submitted_desc.nodes[i].id, (u32)i);
                }
            }
        }

        void Context::close_popup_stack_from(usize index)
        {
            if(index >= m_popup_stack.open_stack.size()) return;
            for(usize i = index; i < m_popup_stack.open_stack.size(); ++i)
            {
                id_t id = m_popup_stack.open_stack[i].id;
                get_or_create_widget_state<DisclosureState>(id)->open = false;
                get_or_create_query_state(id)->states.insert_or_assign(Name("gui.open"), Any(false));
            }
            m_popup_stack.open_stack.erase(m_popup_stack.open_stack.begin() + index, m_popup_stack.open_stack.end());
        }

        void Context::prune_popup_stack()
        {
            for(usize i = 0; i < m_popup_stack.open_stack.size();)
            {
                auto iter = m_popup_stack.node_indices.find(m_popup_stack.open_stack[i].id);
                if(iter == m_popup_stack.node_indices.end())
                {
                    close_popup_stack_from(i);
                    break;
                }
                const Node& node = m_submitted_desc.nodes[iter->second];
                PopupFlag flags = popup_flags(node);
                id_t parent_id = popup_parent(node);
                if(!test_flags(flags, PopupFlag::managed))
                {
                    close_popup_stack_from(i);
                    break;
                }
                if(parent_id)
                {
                    auto parent_iter = m_popup_stack.node_indices.find(parent_id);
                    if(parent_iter != m_popup_stack.node_indices.end() &&
                        !popup_node_visible(m_submitted_desc.nodes[parent_iter->second]))
                    {
                        close_popup_stack_from(i);
                        break;
                    }
                }
                m_popup_stack.open_stack[i].parent_id = parent_id;
                m_popup_stack.open_stack[i].flags = flags;
                get_or_create_widget_state<DisclosureState>(node.id)->open = true;
                ++i;
            }
        }

        void Context::open_popup(ItemHandle popup)
        {
            lutsassert();
            if(!popup.id || popup.context != get_object())
            {
                m_popup_stack.next_opener_id = 0;
                return;
            }

            const Node* node = find_popup_node_in_desc(m_submitted_desc, popup.id);
            if(!node)
            {
                node = find_popup_node_in_desc(m_build_desc, popup.id);
            }

            PopupFlag flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            id_t parent_id = 0;
            id_t opener_id = m_popup_stack.next_opener_id ? m_popup_stack.next_opener_id : current_clicked_item_id();
            m_popup_stack.next_opener_id = 0;
            if(node)
            {
                flags = popup_flags(*node);
                parent_id = popup_parent(*node);
                if(!test_flags(flags, PopupFlag::managed))
                {
                    flags |= PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
                }
            }
            else if(const PopupBuildInfo* info = find_popup_info(m_popup_stack, popup.id))
            {
                flags = info->flags;
                parent_id = info->parent_id;
                if(!test_flags(flags, PopupFlag::managed))
                {
                    flags |= PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
                }
            }

            i32 existing = popup_stack_index(popup.id);
            if(existing >= 0)
            {
                close_popup_stack_from((usize)existing + 1);
                get_or_create_widget_state<DisclosureState>(popup.id)->open = true;
                get_or_create_query_state(popup.id)->states.insert_or_assign(Name("gui.open"), Any(true));
                if(opener_id)
                {
                    m_popup_stack.open_stack[(usize)existing].opener_id = opener_id;
                }
                return;
            }

            if(parent_id)
            {
                i32 parent_index = popup_stack_index(parent_id);
                bool parent_visible = parent_index >= 0;
                if(!parent_visible)
                {
                    auto parent_iter = m_popup_stack.node_indices.find(parent_id);
                    parent_visible = parent_iter != m_popup_stack.node_indices.end() && popup_node_visible(m_submitted_desc.nodes[parent_iter->second]);
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
            entry.opener_id = opener_id;
            entry.flags = flags;
            m_popup_stack.open_stack.push_back(entry);
            get_or_create_widget_state<DisclosureState>(popup.id)->open = true;
            get_or_create_query_state(popup.id)->states.insert_or_assign(Name("gui.open"), Any(true));
        }

        void Context::open_popup(const c8* label)
        {
            ItemHandle popup = popup_handle(label);
            if(m_popup_stack.build_infos.find(popup.id) == m_popup_stack.build_infos.end())
            {
                PopupBuildInfo info;
                info.parent_id = m_popup_stack.build_stack.empty() ? 0 : m_popup_stack.build_stack.back();
                info.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
                m_popup_stack.build_infos.insert_or_assign(popup.id, info);
            }
            open_popup(popup);
        }

        void Context::close_popup(ItemHandle popup)
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
                get_or_create_widget_state<DisclosureState>(popup.id)->open = false;
                get_or_create_query_state(popup.id)->states.insert_or_assign(Name("gui.open"), Any(false));
            }
        }

        void Context::close_popup(const c8* label)
        {
            close_popup(popup_handle(label));
        }

        void Context::close_current_popup()
        {
            lutsassert();
            if(m_popup_stack.open_stack.empty()) return;
            close_popup_stack_from(m_popup_stack.open_stack.size() - 1);
        }

        void Context::close_all_popups()
        {
            lutsassert();
            close_popup_stack_from(0);
        }

        i32 Context::popup_level_at_pos(const Float2U& pos) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return -1;
            for(usize i = m_popup_stack.open_stack.size(); i > 0; --i)
            {
                usize level = i - 1;
                auto iter = m_popup_stack.node_indices.find(m_popup_stack.open_stack[level].id);
                if(iter == m_popup_stack.node_indices.end()) continue;
                const NodeLayout& layout = m_layouts[iter->second];
                if(point_in_rect(pos, layout.rect) && point_in_rect(pos, layout.clip_rect))
                {
                    return (i32)level;
                }
                id_t opener_id = m_popup_stack.open_stack[level].opener_id;
                if(opener_id)
                {
                    for(usize node_index = 0; node_index < m_submitted_desc.nodes.size(); ++node_index)
                    {
                        if(m_submitted_desc.nodes[node_index].id != opener_id) continue;
                        const NodeLayout& opener_layout = m_layouts[node_index];
                        if(point_in_rect(pos, opener_layout.rect) && point_in_rect(pos, opener_layout.clip_rect))
                        {
                            return (i32)level;
                        }
                        break;
                    }
                }
            }
            return -1;
        }

        bool Context::close_popups_for_pointer_down(const Float2U& pos)
        {
            if(m_popup_stack.open_stack.empty()) return false;
            i32 level = popup_level_at_pos(pos);
            if(level < 0)
            {
                id_t target = hit_test(pos);
                Node* target_node = target ? find_node(target) : nullptr;
                bool menu_target = target_node && menu_node(*target_node);
                PopupFlag flags = m_popup_stack.open_stack.back().flags;
                id_t menu_popup_id = target_node ? menu_popup(*target_node) : 0;
                if(menu_target && menu_popup_id && is_popup_open(menu_popup_id))
                {
                    close_popup(ItemHandle{get_object(), menu_popup_id, m_generation});
                    return true;
                }
                if(test_flags(flags, PopupFlag::close_on_outside_click))
                {
                    close_all_popups();
                }
                return !menu_target;
            }
            if((usize)level + 1 < m_popup_stack.open_stack.size())
            {
                close_popup_stack_from((usize)level + 1);
            }
            return false;
        }
    }
}
