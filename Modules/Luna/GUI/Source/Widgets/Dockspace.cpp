/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Dockspace.cpp
* @author JXMaster
* @date 2024/8/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Widgets/Dockspace.hpp"
#include "../../Context.hpp"
#include "../../WidgetDraw.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include "../../WidgetBuilder.hpp"
#include "../../Event.hpp"

namespace Luna
{
    namespace GUI
    {
        static UniquePtr<DockNodeBase> detach_widget_node(Dockspace* dockspace, WidgetDockNode* node)
        {
            UniquePtr<DockNodeBase> ret;
            if(!node->parent)
            {
                // This is a root node, remove from dockspace directly.
                if(dockspace->m_state->root.get() == node)
                {
                    ret = move(dockspace->m_state->root);
                }
                return ret;
            }
            luassert(node->parent->type == DockNodeType::vertical || node->parent->type == DockNodeType::horizontal);
            // Replace parent node with the other child node.
            BinaryDockNode* parent = (BinaryDockNode*)node->parent;
            UniquePtr<DockNodeBase> other_node;
            if(node == parent->first_child.get())
            {
                other_node = move(parent->second_child);
                ret = move(parent->first_child);
            }
            else
            {
                other_node = move(parent->first_child);
                ret = move(parent->second_child);
            }
            if(!parent->parent)
            {
                // Parent is root node, replace in dockspace directly.
                other_node->parent = nullptr;
                if(dockspace->m_state->root.get() == parent)
                {
                    dockspace->m_state->root = move(other_node);
                }
            }
            else
            {
                luassert(parent->parent->type == DockNodeType::vertical || parent->parent->type == DockNodeType::horizontal);
                BinaryDockNode* pp = (BinaryDockNode*)parent->parent;
                other_node->parent = pp;
                if(pp->first_child.get() == parent)
                {
                    pp->first_child = move(other_node);
                }
                else
                {
                    pp->second_child = move(other_node);
                }
            }
            return ret;
        }
        static void add_widget_to_node(Dockspace* dockspace, widget_hash_t widget_hash, WidgetDockNode* target_node, u32 target_side)
        {
            if(target_side == 4)
            {
                // add to target node directly.
                WidgetDockNode::WidgetItem item;
                item.hash = widget_hash;
                item.tab_rect_left = 0;
                item.tab_rect_right = 0;
                target_node->widgets.push_back(item);
            }
            else
            {
                // create new binary node to separate space.
                UniquePtr<DockNodeBase> new_bnode(memnew<BinaryDockNode>(target_side < 2 ? DockNodeType::horizontal : DockNodeType::vertical));
                BinaryDockNode* bnode = (BinaryDockNode*)new_bnode.get();
                // insert new binary node to node tree.
                BinaryDockNode* parent = target_node->parent;
                UniquePtr<DockNodeBase> old_node;
                if(parent)
                {
                    if(parent->first_child.get() == target_node)
                    {
                        old_node = move(parent->first_child);
                        parent->first_child = move(new_bnode);
                    }
                    else
                    {
                        old_node = move(parent->second_child);
                        parent->second_child = move(new_bnode);
                    }
                    bnode->parent = parent;
                }
                else
                {
                    // This node is root node.
                    if(dockspace->m_state->root.get() == target_node)
                    {
                        old_node = move(dockspace->m_state->root);
                        dockspace->m_state->root = move(new_bnode);
                    }
                }
                // Create and insert new widget node.
                WidgetDockNode* new_widget_node;
                if(target_side == 0 || target_side == 2)
                {
                    // Add new widget to first child.
                    bnode->first_child.reset(memnew<WidgetDockNode>());
                    bnode->second_child = move(old_node);
                    new_widget_node = (WidgetDockNode*)bnode->first_child.get();
                }
                else
                {
                    // Add new widget to second child.
                    bnode->second_child.reset(memnew<WidgetDockNode>());
                    bnode->first_child = move(old_node);
                    new_widget_node = (WidgetDockNode*)bnode->second_child.get();
                }
                target_node->parent = bnode;
                new_widget_node->parent = bnode;
                // Add widget to new node.
                WidgetDockNode::WidgetItem item;
                item.hash = widget_hash;
                item.tab_rect_left = 0;
                item.tab_rect_right = 0;
                new_widget_node->widgets.push_back(item);
            }
        }
        static void remove_widget_from_node(Dockspace* dockspace, WidgetDockNode* source_node, widget_hash_t widget_hash)
        {
            for(auto iter = source_node->widgets.begin(); iter != source_node->widgets.end(); ++iter)
            {
                if(iter->hash == widget_hash)
                {
                    source_node->widgets.erase(iter);
                    source_node->current_tab = min<u32>(source_node->current_tab, source_node->widgets.size() - 1);
                    break;
                }
            }
            if(source_node->widgets.empty())
            {
                // remove node with no widgets.
                detach_widget_node(dockspace, source_node);
            }
        }
        static void refresh_widget_tree(Dockspace* dockspace)
        {
            // Collect existing widgets.
            HashSet<widget_hash_t> widgets;
            for(auto& w : dockspace->get_children())
            {
                if(w->get_hash() != 0)
                {
                    widgets.insert(w->get_hash());
                }
            }
            // Remove all widgets that does not exist anymore.
            RingDeque<DockNodeBase*> nodes;
            if(dockspace->m_state->root)
            {
                nodes.push_back(dockspace->m_state->root.get());
            }
            Vector<WidgetDockNode*> widget_nodes;
            HashSet<widget_hash_t> existing_widgets;
            while(!nodes.empty())
            {
                DockNodeBase* node = nodes.front();
                nodes.pop_front();
                switch(node->type)
                {
                    case DockNodeType::horizontal:
                    case DockNodeType::vertical:
                    {
                        BinaryDockNode* bnode = (BinaryDockNode*)node;
                        nodes.push_back(bnode->first_child.get());
                        nodes.push_back(bnode->second_child.get());
                    }
                    break;
                    case DockNodeType::widget:
                    {
                        WidgetDockNode* wnode = (WidgetDockNode*)node;
                        widget_nodes.push_back(wnode);
                        auto iter = wnode->widgets.begin();
                        while(iter != wnode->widgets.end())
                        {
                            existing_widgets.insert(iter->hash);
                            auto found = widgets.find(iter->hash);
                            if(found == widgets.end())
                            {
                                iter = wnode->widgets.erase(iter);
                            }
                            else
                            {
                                ++iter;
                            }
                        }
                    }
                    break;
                    default: lupanic();
                }
            }
            // Remove empty widget nodes.
            for(WidgetDockNode* node : widget_nodes)
            {
                if(node->widgets.empty())
                {
                    detach_widget_node(dockspace, node); // We don't catch the return value, so that it will be destructed.
                }
            }
            // Add new widgets.
            for(widget_hash_t hash : widgets)
            {
                if(!existing_widgets.contains(hash))
                {
                    if(!dockspace->m_state->root)
                    {
                        // Set this node as root.
                        WidgetDockNode* wnode = memnew<WidgetDockNode>();
                        WidgetDockNode::WidgetItem item;
                        item.hash = hash;
                        wnode->widgets.push_back(item);
                        dockspace->m_state->root.reset(wnode);
                    }
                    else
                    {
                        // Find first docknode in the dockspace.
                        RingDeque<DockNodeBase*> nodes;
                        nodes.push_back(dockspace->m_state->root.get());
                        bool inserted = false;
                        while(!nodes.empty())
                        {
                            DockNodeBase* node = nodes.front();
                            nodes.pop_front();
                            switch(node->type)
                            {
                                case DockNodeType::widget:
                                {
                                    WidgetDockNode* wnode = (WidgetDockNode*)node;
                                    WidgetDockNode::WidgetItem item;
                                    item.hash = hash;
                                    wnode->widgets.push_back(item);
                                    wnode->current_tab = (u32)wnode->widgets.size() - 1;
                                    inserted = true;
                                }
                                break;
                                case DockNodeType::horizontal:
                                case DockNodeType::vertical:
                                {
                                    BinaryDockNode* bnode = (BinaryDockNode*)node;
                                    nodes.push_back(bnode->first_child.get());
                                    nodes.push_back(bnode->second_child.get());
                                }
                                break;
                                default: lupanic();
                            }
                            if(inserted) break;
                        }                        
                        luassert(inserted); // The entire tree must have at least one widget node.
                    }
                }
            }
        }
        LUNA_GUI_API RV Dockspace::begin_update(IContext* ctx)
        {
            Ref<DockspaceState> s = cast_object<DockspaceState>(ctx->get_widget_state(get_hash()));
            if(!s)
            {
                s = new_object<DockspaceState>();
            }
            ctx->set_widget_state(get_hash(), s.get(), WidgetStateLifetime::persistent);
            m_state = s.get();
            if(m_state->clicking_node)
            {
                ctx->capture_event(this, typeof<MouseEvent>());
            }
            // Refresh widget tree.
            refresh_widget_tree(this);
            lutry
            {
                for(auto& c : get_children())
                {
                    luexp(c->begin_update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        constexpr f32 DOCKNODE_SEP_LINE_WIDTH = 2.0f;
        static Widget* find_widget_by_hash(Dockspace* dockspace, widget_hash_t widget_hash)
        {
            for(auto& c : dockspace->get_children())
            {
                if(c->get_hash() == widget_hash)
                {
                    return c.get();
                }
            }
            return nullptr;
        }
        static RV layout_docknode(Dockspace* dockspace, DockNodeBase* node, IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                node->layout_rect = layout_rect;
                switch(node->type)
                {
                case DockNodeType::widget:
                {
                    WidgetDockNode* cnode = (WidgetDockNode*)node;
                    cnode->title_rect = layout_rect;
                    f32 title_bar_height = dockspace->get_sattr(SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE) + 10.0f;
                    cnode->title_rect.bottom = min(cnode->title_rect.top + title_bar_height, layout_rect.bottom);
                    cnode->widget_rect = layout_rect;
                    cnode->widget_rect.top = cnode->title_rect.bottom;
                    // Layout tab rect.
                    f32 tab_rect_offset = 0;
                    f32 title_size = dockspace->get_sattr(SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                    Font::IFontFile* font = query_interface<Font::IFontFile>(dockspace->get_oattr(OATTR_FONT, true, Font::get_default_font()));
                    u32 font_index = dockspace->get_sattr(SATTR_FONT_INDEX, true, 0);
                    for(auto& w : cnode->widgets)
                    {
                        Widget* widget = find_widget_by_hash(dockspace, cnode->widgets[cnode->current_tab].hash);
                        Name title = widget->get_tattr(TATTR_TITLE, false, "Untitled");
                        VG::TextArrangeSection section;
                        section.font_file = font;
                        section.font_index = font_index;
                        section.font_size = title_size;
                        section.num_chars = title.size();
                        auto result = VG::arrange_text(title.c_str(), title.size(), {&section, 1}, RectF(0, 0, F32_MAX, F32_MAX), VG::TextAlignment::begin, VG::TextAlignment::begin);
                        w.tab_rect_left = tab_rect_offset;
                        w.tab_rect_right = w.tab_rect_left + result.bounding_rect.width + 10.0f;
                        tab_rect_offset = w.tab_rect_right;
                    }
                    Widget* widget = find_widget_by_hash(dockspace, cnode->widgets[cnode->current_tab].hash);
                    luexp(widget->layout(ctx, cnode->widget_rect));
                }
                break;
                case DockNodeType::horizontal:
                {
                    BinaryDockNode* bnode = (BinaryDockNode*)node;
                    OffsetRectF rect = layout_rect;
                    f32 offset = (rect.right - rect.left) * bnode->second_offset;
                    rect.right = rect.left + offset;
                    luexp(layout_docknode(dockspace, bnode->first_child.get(), ctx, rect));
                    rect.left = rect.right;
                    rect.right = layout_rect.right;
                    luexp(layout_docknode(dockspace, bnode->second_child.get(), ctx, rect));
                }
                break;
                case DockNodeType::vertical:
                {
                    BinaryDockNode* bnode = (BinaryDockNode*)node;
                    OffsetRectF rect = layout_rect;
                    f32 offset = (rect.bottom - rect.top) * bnode->second_offset;
                    rect.bottom = rect.top + offset;
                    luexp(layout_docknode(dockspace, bnode->first_child.get(), ctx, rect));
                    rect.top = rect.bottom;
                    rect.bottom = layout_rect.bottom;
                    luexp(layout_docknode(dockspace, bnode->second_child.get(), ctx, rect));
                }
                break;
                default: lupanic();
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Dockspace::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                if(m_state->root)
                {
                    luexp(layout_docknode(this, m_state->root.get(), ctx, layout_rect));
                }
            }
            lucatchret;
            return ok;
        }
        static RV docknode_handle_mouse_event(IContext* ctx, Dockspace* dockspace, DockNodeBase* node, MouseEvent* e, bool& handled)
        {
            lutry
            {
                switch(node->type)
                {
                    case DockNodeType::widget:
                    {
                        WidgetDockNode* wnode = (WidgetDockNode*)node;
                        MouseButtonEvent* be = cast_object<MouseButtonEvent>(e);
                        if(be && be->button == HID::MouseButton::left && be->pressed == true)
                        {
                            for(usize i = 0; i < wnode->widgets.size(); ++i)
                            {
                                auto& w = wnode->widgets[i];
                                if(in_bounds(Float2(e->x, e->y), Float2(wnode->title_rect.left + w.tab_rect_left, wnode->title_rect.top), Float2(wnode->title_rect.left + w.tab_rect_right, wnode->title_rect.bottom)))
                                {
                                    // We need to relayout the widget when we change to a new tab, since this tab is hidden when `layout` is called,
                                    // thus not get updated.
                                    Widget* widget = find_widget_by_hash(dockspace, w.hash);
                                    if(wnode->current_tab != i)
                                    {
                                        wnode->current_tab = (u32)i;
                                        luexp(widget->layout(ctx, wnode->widget_rect));
                                    }
                                    dockspace->m_state->clicking_node = wnode;
                                    dockspace->m_state->clicking_widget_index = i;
                                    dockspace->m_state->clicking_pos = Float2U{e->x, e->y};
                                    dockspace->m_state->clicking_node_rect = OffsetRectF(wnode->title_rect.left + w.tab_rect_left, wnode->title_rect.top, wnode->title_rect.left + w.tab_rect_right, wnode->title_rect.bottom);
                                    break;
                                }
                            }
                        }
                        // Broadcast event to child widgets if we are not currently dragging nodes.
                        if(in_bounds(Float2(e->x, e->y), Float2(wnode->widget_rect.left, wnode->widget_rect.top), Float2(wnode->widget_rect.right, wnode->widget_rect.bottom)))
                        {
                            Widget* widget = find_widget_by_hash(dockspace, wnode->widgets[wnode->current_tab].hash);
                            luexp(dispatch_event_by_pos(ctx, widget, e, e->x, e->y, handled));
                        }
                    }
                    break;
                    case DockNodeType::horizontal:
                    case DockNodeType::vertical:
                    {
                        BinaryDockNode* bnode = (BinaryDockNode*)node;
                        luexp(docknode_handle_mouse_event(ctx, dockspace, bnode->first_child.get(), e, handled));
                        luexp(docknode_handle_mouse_event(ctx, dockspace, bnode->second_child.get(), e, handled));
                    }
                    break;
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Dockspace::handle_event(IContext* ctx, object_t e, bool& handled)
        {
            lutry
            {
                // Handle window move event.
                MouseEvent* mouse_event = cast_object<MouseEvent>(e);
                if(mouse_event)
                {
                    MouseMoveEvent* me = cast_object<MouseMoveEvent>(e);
                    if(me && m_state->clicking_node && !m_state->dragging && distance_squared(m_state->clicking_pos, Float2(me->x, me->y)) > 25.0f)
                    {
                        // Start dragging.
                        m_state->dragging = true;
                    }
                    m_state->dragging_dock_target = nullptr;
                    if(!m_state->dragging)
                    {
                        luexp(docknode_handle_mouse_event(ctx, this, m_state->root.get(), mouse_event, handled));
                    }
                    else
                    {
                        m_state->dragging_mouse_pos = Float2U(mouse_event->x, mouse_event->y);
                        // Check target.
                        RingDeque<DockNodeBase*> scan_queue;
                        scan_queue.push_back(m_state->root.get());
                        while(!scan_queue.empty())
                        {
                            DockNodeBase* node = scan_queue.front();
                            scan_queue.pop_front();
                            if(node->type == DockNodeType::widget)
                            {
                                WidgetDockNode* wnode = (WidgetDockNode*)node;
                                f32 width = wnode->widget_rect.right - wnode->widget_rect.left;
                                f32 height = wnode->widget_rect.bottom - wnode->widget_rect.top;
                                if(in_bounds(Float2(m_state->dragging_mouse_pos), 
                                    Float2(wnode->widget_rect.left, wnode->widget_rect.top), 
                                    Float2(wnode->widget_rect.right, wnode->widget_rect.bottom)))
                                {
                                    m_state->dragging_dock_target = wnode;
                                    if(in_bounds(Float2(m_state->dragging_mouse_pos), 
                                        Float2(wnode->widget_rect.left, wnode->widget_rect.top), 
                                        Float2(wnode->widget_rect.left + width * 0.1f, wnode->widget_rect.bottom)))
                                    {
                                        // Left
                                        m_state->dragging_dock_side = 0;
                                    }
                                    else if(in_bounds(Float2(m_state->dragging_mouse_pos), 
                                        Float2(wnode->widget_rect.right - width * 0.1f, wnode->widget_rect.top), 
                                        Float2(wnode->widget_rect.right, wnode->widget_rect.bottom)))
                                    {
                                        // Right
                                        m_state->dragging_dock_side = 1;
                                    }
                                    else if(in_bounds(Float2(m_state->dragging_mouse_pos), 
                                        Float2(wnode->widget_rect.left, wnode->widget_rect.top), 
                                        Float2(wnode->widget_rect.right, wnode->widget_rect.top + height * 0.1f)))
                                    {
                                        // Top
                                        m_state->dragging_dock_side = 2;
                                    }
                                    else if(in_bounds(Float2(m_state->dragging_mouse_pos), 
                                        Float2(wnode->widget_rect.left, wnode->widget_rect.bottom - height * 0.1f), 
                                        Float2(wnode->widget_rect.right, wnode->widget_rect.bottom)))
                                    {
                                        // Bottom
                                        m_state->dragging_dock_side = 3;
                                    }
                                    else
                                    {
                                        // Center.
                                        m_state->dragging_dock_side = 4;
                                    }   
                                }
                            }
                            else
                            {
                                BinaryDockNode* bnode = (BinaryDockNode*)node;
                                scan_queue.push_back(bnode->first_child.get());
                                scan_queue.push_back(bnode->second_child.get());
                            }
                        }
                    }
                    MouseButtonEvent* be = cast_object<MouseButtonEvent>(e);
                    if(be && be->button == HID::MouseButton::left && be->pressed == false)
                    {
                        if(m_state->dragging && m_state->dragging_dock_target)
                        {
                            // Drop the node to the target.
                            auto widget_hash = m_state->clicking_node->widgets[m_state->clicking_widget_index].hash;
                            add_widget_to_node(this, widget_hash, m_state->dragging_dock_target, m_state->dragging_dock_side);
                            remove_widget_from_node(this, m_state->clicking_node, widget_hash);
                        }
                        m_state->clicking_node = nullptr;
                        m_state->dragging = false;
                    }
                    // Since we have handled this event manually in `docknode_handle_mouse_event`, we should prevent this event from 
                    // broadcasting to child widgets again.
                    handled = true;
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Dockspace::update(IContext* ctx)
        {
            lutry
            {
                RingDeque<DockNodeBase*> nodes;
                if(m_state->root)
                {
                    nodes.push_back(m_state->root.get());
                }
                while(!nodes.empty())
                {
                    DockNodeBase* node = nodes.front();
                    nodes.pop_front();
                    switch(node->type)
                    {
                        case DockNodeType::horizontal:
                        case DockNodeType::vertical:
                        {
                            BinaryDockNode* bnode = (BinaryDockNode*)node;
                            nodes.push_back(bnode->first_child.get());
                            nodes.push_back(bnode->second_child.get());
                        }
                        break;
                        case DockNodeType::widget:
                        {
                            WidgetDockNode* wnode = (WidgetDockNode*)node;
                            Widget* widget = find_widget_by_hash(this, wnode->widgets[wnode->current_tab].hash);
                            luassert(widget);
                            luexp(widget->update(ctx));
                        }
                        break;
                        default: lupanic();
                    }
                }
            }
            lucatchret;
            return ok;
        }
        static RV draw_docknode(Dockspace* dockspace, DockNodeBase* node, IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            lutry
            {
                switch(node->type)
                {
                    case DockNodeType::widget:
                    {
                        WidgetDockNode* wnode = (WidgetDockNode*)node;
                        f32 title_size = dockspace->get_sattr(SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                        // Draw title background.
                        draw_rectangle_filled(ctx, draw_list, 
                            node->layout_rect.left, wnode->title_rect.top, node->layout_rect.right, wnode->title_rect.bottom, Float4(0.8f, 0.8f, 0.8f, 1.0f));
                        auto& current_node = wnode->widgets[wnode->current_tab];
                        draw_rectangle_filled(ctx, draw_list,
                            wnode->title_rect.left + current_node.tab_rect_left, wnode->title_rect.top, wnode->title_rect.left + current_node.tab_rect_right, wnode->title_rect.bottom, Float4(1.0f, 1.0f, 1.0f, 1.0f));
                        // Draw title.
                        Font::IFontFile* font = query_interface<Font::IFontFile>(dockspace->get_oattr(OATTR_FONT, true, Font::get_default_font()));
                        u32 font_index = dockspace->get_sattr(SATTR_FONT_INDEX, true, 0);
                        for(auto& w : wnode->widgets)
                        {
                            Widget* widget = find_widget_by_hash(dockspace, w.hash);
                            Name title = widget->get_tattr(TATTR_TITLE, false, "Untitled");
                            draw_text(ctx, draw_list, title.c_str(), title.size(), Float4U(0, 0, 0, 1), title_size, 
                                wnode->title_rect.left + w.tab_rect_left + 5.0f, node->layout_rect.top + 5.0f, 
                                wnode->title_rect.left + w.tab_rect_right - 5.0f, node->layout_rect.top + title_size + 6.0f, 
                                font, font_index);
                        }
                        // Draw content.
                        Widget* widget = find_widget_by_hash(dockspace, current_node.hash);
                        luexp(widget->draw(ctx, draw_list, overlay_draw_list));
                    }
                    break;
                    case DockNodeType::horizontal:
                    {
                        BinaryDockNode* bnode = (BinaryDockNode*)node;
                        luexp(draw_docknode(dockspace, bnode->first_child.get(), ctx, draw_list, overlay_draw_list));
                        luexp(draw_docknode(dockspace, bnode->second_child.get(), ctx, draw_list, overlay_draw_list));
                        // Draw sep line.
                        f32 offset = (node->layout_rect.right - node->layout_rect.left) * bnode->second_offset;
                        draw_rectangle_filled(ctx, draw_list, 
                            node->layout_rect.left + offset - DOCKNODE_SEP_LINE_WIDTH, node->layout_rect.top,
                            node->layout_rect.left + offset + DOCKNODE_SEP_LINE_WIDTH, node->layout_rect.bottom, Float4(0, 0, 0, 1));
                    }
                    break;
                    case DockNodeType::vertical:
                    {
                        BinaryDockNode* bnode = (BinaryDockNode*)node;
                        luexp(draw_docknode(dockspace, bnode->first_child.get(), ctx, draw_list, overlay_draw_list));
                        luexp(draw_docknode(dockspace, bnode->second_child.get(), ctx, draw_list, overlay_draw_list));
                        // Draw sep line.
                        f32 offset = (node->layout_rect.bottom - node->layout_rect.top) * bnode->second_offset;
                        draw_rectangle_filled(ctx, draw_list, 
                            node->layout_rect.left, node->layout_rect.top + offset - DOCKNODE_SEP_LINE_WIDTH,
                            node->layout_rect.right, node->layout_rect.top + offset + DOCKNODE_SEP_LINE_WIDTH, Float4(0, 0, 0, 1));
                    }
                    break;
                    default: lupanic();
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Dockspace::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            lutry
            {
                // Draw root.
                if(m_state->root)
                {
                    DockNodeBase* node = m_state->root.get();
                    draw_rectangle_filled(ctx, draw_list, node->layout_rect.left, node->layout_rect.top, node->layout_rect.right, node->layout_rect.bottom, Float4(1, 1, 1, 1));
                    luexp(draw_docknode(this, m_state->root.get(), ctx, draw_list, overlay_draw_list));
                }
                // Draw overlay.
                if(m_state->dragging)
                {
                    // Draw overlay color
                    if(m_state->dragging_dock_target)
                    {
                        WidgetDockNode* wnode = m_state->dragging_dock_target;
                        f32 width = wnode->widget_rect.right - wnode->widget_rect.left;
                        f32 height = wnode->widget_rect.bottom - wnode->widget_rect.top;
                        f32 left = wnode->widget_rect.left;
                        f32 top = wnode->widget_rect.top;
                        f32 right = wnode->widget_rect.right;
                        f32 bottom = wnode->widget_rect.bottom;
                        switch(m_state->dragging_dock_side)
                        {
                            case 0: right = left + width * 0.5f; break; // Left
                            case 1: left = left + width * 0.5f; break; // Right
                            case 2: bottom = top + height * 0.5f; break; // Top
                            case 3: top = bottom - height * 0.5f; break; // Bottom
                            default: break;
                        }
                        draw_rectangle_filled(ctx, overlay_draw_list, left, top, right, bottom, Float4U(0.5, 0.5, 1.0, 0.5));
                    }
                    // Draw title rect.
                    {
                        OffsetRectF rect(
                            m_state->clicking_node_rect.left - m_state->clicking_pos.x + m_state->dragging_mouse_pos.x,
                            m_state->clicking_node_rect.top - m_state->clicking_pos.y + m_state->dragging_mouse_pos.y,
                            m_state->clicking_node_rect.right - m_state->clicking_pos.x + m_state->dragging_mouse_pos.x,
                            m_state->clicking_node_rect.bottom - m_state->clicking_pos.y + m_state->dragging_mouse_pos.y
                        );
                        draw_rectangle_filled(ctx, overlay_draw_list, rect.left, rect.top, rect.right, rect.bottom, Float4(1.0f, 1.0f, 1.0f, 1.0f));
                        Widget* widget = find_widget_by_hash(this, m_state->clicking_node->widgets[m_state->clicking_widget_index].hash);
                        Name title = widget->get_tattr(TATTR_TITLE, false, "Untitled");
                        f32 title_size = get_sattr(SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                        Font::IFontFile* font = query_interface<Font::IFontFile>(get_oattr(OATTR_FONT, true, Font::get_default_font()));
                        u32 font_index = get_sattr(SATTR_FONT_INDEX, true, 0);
                        draw_text(ctx, overlay_draw_list, title.c_str(), title.size(), Float4U(0, 0, 0, 1), title_size, 
                                    rect.left + 5.0f, rect.top + 5.0f, rect.right - 5.0f, rect.top + title_size + 6.0f, 
                                    font, font_index);
                    }
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API Dockspace* begin_dockspace(IWidgetBuilder* builder, const Name& id)
        {
            Dockspace* widget = builder->begin_widget<Dockspace>();
            widget->set_id(id);
            return widget;
        }
        LUNA_GUI_API void end_dockspace(IWidgetBuilder* builder)
        {
            builder->end_widget();
        }
    }
}