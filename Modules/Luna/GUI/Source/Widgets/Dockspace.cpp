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
                if(dockspace->state->root.get() == node)
                {
                    ret = move(dockspace->state->root);
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
                if(dockspace->state->root.get() == parent)
                {
                    dockspace->state->root = move(other_node);
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
        static void refresh_widget_tree(Dockspace* dockspace)
        {
            // Collect existing widgets.
            HashSet<widget_id_t> widgets;
            for(auto& w : dockspace->children)
            {
                if(w->get_id() != 0)
                {
                    widgets.insert(w->get_id());
                }
            }
            // Remove all widgets that does not exist anymore.
            RingDeque<DockNodeBase*> nodes;
            if(dockspace->state->root)
            {
                nodes.push_back(dockspace->state->root.get());
            }
            Vector<WidgetDockNode*> widget_nodes;
            HashSet<widget_id_t> existing_widgets;
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
                            existing_widgets.insert(iter->id);
                            auto found = widgets.find(iter->id);
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
            for(widget_id_t id : widgets)
            {
                if(!existing_widgets.contains(id))
                {
                    if(!dockspace->state->root)
                    {
                        // Set this node as root.
                        WidgetDockNode* wnode = memnew<WidgetDockNode>();
                        WidgetDockNode::WidgetItem item;
                        item.id = id;
                        wnode->widgets.push_back(item);
                        dockspace->state->root.reset(wnode);
                    }
                    else
                    {
                        // Find first docknode in the dockspace.
                        RingDeque<DockNodeBase*> nodes;
                        nodes.push_back(dockspace->state->root.get());
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
                                    item.id = id;
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
            Ref<DockspaceState> s = cast_object<DockspaceState>(ctx->get_widget_state(id));
            if(!s)
            {
                s = new_object<DockspaceState>();
            }
            ctx->set_widget_state(id, s.get(), WidgetStateLifetime::persistent);
            state = s.get();
            if(state->clicking_node)
            {
                ctx->capture_event(this, typeof<MouseEvent>());
            }
            // Refresh widget tree.
            refresh_widget_tree(this);
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->begin_update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        constexpr f32 DOCKNODE_SEP_LINE_WIDTH = 2.0f;
        static IWidget* find_widget_by_id(Dockspace* dockspace, widget_id_t widget_id)
        {
            for(auto& c : dockspace->children)
            {
                if(c->get_id() == widget_id)
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
                    f32 title_bar_height = get_sattr(dockspace, SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE) + 10.0f;
                    cnode->title_rect.bottom = min(cnode->title_rect.top + title_bar_height, layout_rect.bottom);
                    cnode->widget_rect = layout_rect;
                    cnode->widget_rect.top = cnode->title_rect.bottom;
                    // Layout tab rect.
                    f32 tab_rect_offset = 0;
                    f32 title_size = get_sattr(dockspace, SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                    Font::IFontFile* font = query_interface<Font::IFontFile>(get_oattr(dockspace, OATTR_FONT, true, Font::get_default_font()));
                    u32 font_index = get_sattr(dockspace, SATTR_FONT_INDEX, true, 0);
                    for(auto& w : cnode->widgets)
                    {
                        IWidget* widget = find_widget_by_id(dockspace, cnode->widgets[cnode->current_tab].id);
                        Name title = get_tattr(widget, TATTR_TITLE, false, "Untitled");
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
                    IWidget* widget = find_widget_by_id(dockspace, cnode->widgets[cnode->current_tab].id);
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
                if(state->root)
                {
                    luexp(layout_docknode(this, state->root.get(), ctx, layout_rect));
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
                                if(in_bounds(Float2(e->x, e->y), Float2(w.tab_rect_left, wnode->title_rect.top), Float2(w.tab_rect_right, wnode->title_rect.bottom)))
                                {
                                    // We need to relayout the widget when we change to a new tab, since this tab is hidden when `layout` is called,
                                    // thus not get updated.
                                    IWidget* widget = find_widget_by_id(dockspace, w.id);
                                    if(wnode->current_tab != i)
                                    {
                                        wnode->current_tab = (u32)i;
                                        luexp(widget->layout(ctx, wnode->widget_rect));
                                    }
                                    dockspace->state->clicking_node = wnode;
                                    dockspace->state->clicking_widget_index = i;
                                    dockspace->state->clicking_pos = Float2U{e->x, e->y};
                                    dockspace->state->clicking_node_rect = OffsetRectF(w.tab_rect_left, wnode->title_rect.top, w.tab_rect_right, wnode->title_rect.bottom);
                                    break;
                                }
                            }
                        }
                        // Broadcast event to child widgets if we are not currently dragging nodes.
                        if(in_bounds(Float2(e->x, e->y), Float2(wnode->widget_rect.left, wnode->widget_rect.top), Float2(wnode->widget_rect.right, wnode->widget_rect.bottom)))
                        {
                            IWidget* widget = find_widget_by_id(dockspace, wnode->widgets[wnode->current_tab].id);
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
                    if(me && state->clicking_node && !state->dragging && distance_squared(state->clicking_pos, Float2(me->x, me->y)) > 25.0f)
                    {
                        // Start dragging.
                        state->dragging = true;
                    }
                    if(!state->dragging)
                    {
                        luexp(docknode_handle_mouse_event(ctx, this, state->root.get(), mouse_event, handled));
                    }
                    else
                    {
                        state->dragging_mouse_pos = Float2U(mouse_event->x, mouse_event->y);
                    }
                    MouseButtonEvent* be = cast_object<MouseButtonEvent>(e);
                    if(be && be->button == HID::MouseButton::left && be->pressed == false)
                    {
                        state->clicking_node = nullptr;
                        state->dragging = false;
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
                if(state->root)
                {
                    nodes.push_back(state->root.get());
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
                            IWidget* widget = find_widget_by_id(this, wnode->widgets[wnode->current_tab].id);
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
                        f32 title_size = get_sattr(dockspace, SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                        // Draw title background.
                        draw_rectangle_filled(ctx, draw_list, 
                            node->layout_rect.left, wnode->title_rect.top, node->layout_rect.right, wnode->title_rect.bottom, Float4(0.8f, 0.8f, 0.8f, 1.0f));
                        auto& current_node = wnode->widgets[wnode->current_tab];
                        draw_rectangle_filled(ctx, draw_list,
                            current_node.tab_rect_left, wnode->title_rect.top, current_node.tab_rect_right, wnode->title_rect.bottom, Float4(1.0f, 1.0f, 1.0f, 1.0f));
                        // Draw title.
                        Font::IFontFile* font = query_interface<Font::IFontFile>(get_oattr(dockspace, OATTR_FONT, true, Font::get_default_font()));
                        u32 font_index = get_sattr(dockspace, SATTR_FONT_INDEX, true, 0);
                        for(auto& w : wnode->widgets)
                        {
                            IWidget* widget = find_widget_by_id(dockspace, w.id);
                            Name title = get_tattr(widget, TATTR_TITLE, false, "Untitled");
                            draw_text(ctx, draw_list, title.c_str(), title.size(), Float4U(0, 0, 0, 1), title_size, 
                                w.tab_rect_left + 5.0f, node->layout_rect.top + 5.0f, w.tab_rect_right - 5.0f, node->layout_rect.top + title_size + 6.0f, 
                                font, font_index);
                        }
                        // Draw content.
                        IWidget* widget = find_widget_by_id(dockspace, current_node.id);
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
                if(state->root)
                {
                    DockNodeBase* node = state->root.get();
                    draw_rectangle_filled(ctx, draw_list, node->layout_rect.left, node->layout_rect.top, node->layout_rect.right, node->layout_rect.bottom, Float4(1, 1, 1, 1));
                    luexp(draw_docknode(this, state->root.get(), ctx, draw_list, overlay_draw_list));
                }
                // Draw overlay.
                if(state->dragging)
                {
                    OffsetRectF rect(
                        state->clicking_node_rect.left - state->clicking_pos.x + state->dragging_mouse_pos.x,
                        state->clicking_node_rect.top - state->clicking_pos.y + state->dragging_mouse_pos.y,
                        state->clicking_node_rect.right - state->clicking_pos.x + state->dragging_mouse_pos.x,
                        state->clicking_node_rect.bottom - state->clicking_pos.y + state->dragging_mouse_pos.y
                    );
                    draw_rectangle_filled(ctx, overlay_draw_list, rect.left, rect.top, rect.right, rect.bottom, Float4(1.0f, 1.0f, 1.0f, 1.0f));
                    IWidget* widget = find_widget_by_id(this, state->clicking_node->widgets[state->clicking_widget_index].id);
                    Name title = get_tattr(widget, TATTR_TITLE, false, "Untitled");
                    f32 title_size = get_sattr(this, SATTR_TITLE_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                    Font::IFontFile* font = query_interface<Font::IFontFile>(get_oattr(this, OATTR_FONT, true, Font::get_default_font()));
                    u32 font_index = get_sattr(this, SATTR_FONT_INDEX, true, 0);
                    draw_text(ctx, overlay_draw_list, title.c_str(), title.size(), Float4U(0, 0, 0, 1), title_size, 
                                rect.left + 5.0f, rect.top + 5.0f, rect.right - 5.0f, rect.top + title_size + 6.0f, 
                                font, font_index);
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API void Dockspace::add_child(IWidget* child)
        {
            children.push_back(child);
        }
        LUNA_GUI_API void Dockspace::get_children(Vector<IWidget*>& out_children)
        {
            out_children.insert(out_children.end(), children.begin(), children.end());
        }
        LUNA_GUI_API usize Dockspace::get_num_children()
        {
            return children.size();
        }
        LUNA_GUI_API Dockspace* begin_dockspace(IWidgetBuilder* builder)
        {
            Ref<Dockspace> widget = new_object<Dockspace>();
            builder->add_widget(widget);
            builder->push_widget(widget);
            return widget;
        }
        LUNA_GUI_API void end_dockspace(IWidgetBuilder* builder)
        {
            builder->pop_widget();
        }
    }
}