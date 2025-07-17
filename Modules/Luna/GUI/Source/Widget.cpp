/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widget.cpp
* @author JXMaster
* @date 2024/5/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Widget.hpp"
#include "../Attributes.hpp"

namespace Luna
{
    namespace GUI
    {
        widget_id_t Widget::get_id()
        {
            return m_id;
        }

        void Widget::set_id(widget_id_t id)
        {
            m_id = id;
        }

        Widget* Widget::get_parent()
        {
            return m_parent;
        }

        static void remove_parent_if_any(Widget* child)
        {
            Widget* parent = child->get_parent();
            if(parent)
            {
                auto children = parent->get_children();
                for(usize i = 0; i < children.size(); ++i)
                {
                    Widget* ch = parent->get_child(i);
                    if(ch == child)
                    {
                        parent->remove_child(i);
                        break;
                    }
                }
            }
        }

        void Widget::add_child(Widget* child, isize pos)
        {
            lucheck_msg(child, "Widget::set_child: child is nullptr");
            Ref<Widget> ref = child;
            remove_parent_if_any(child);
            child->m_parent = this;
            usize index = pos >= 0 ? pos : m_children.size() + 1 + pos;
            lucheck_msg(index <= m_children.size(), "Widget::add_child: Invalid pos.");
            m_children.insert(m_children.begin() + index, child);
        }

        Ref<Widget> Widget::remove_child(usize index)
        {
            lucheck_msg(index < m_children.size(), "Widget::remove_child: invalid index");
            Ref<Widget> ret = move(m_children[index]);
            m_children.erase(m_children.begin() + index);
            ret->m_parent = nullptr;
            return ret;
        }

        Widget* Widget::get_child(usize index)
        {
            lucheck_msg(index < m_children.size(), "Widget::get_child: invalid index");
            return m_children[index];
        }

        Ref<Widget> Widget::set_child(usize index, Widget* new_widget)
        {
            lucheck_msg(index < m_children.size(), "Widget::set_child: invalid index");
            lucheck_msg(new_widget, "Widget::set_child: new_widget is nullptr");
            Ref<Widget> ret = move(m_children[index]);
            m_children[index] = new_widget;
            return ret;
        }

        Span<const Ref<Widget>> Widget::get_children()
        {
            return m_children.cspan();
        }

        void Widget::set_children(Widget** widgets, usize num_widgets)
        {
            for(auto& child : m_children)
            {
                child->m_parent = nullptr;
            }
            m_children.clear();
            for(usize i = 0; i < num_widgets; ++i)
            {
                m_children.push_back(widgets[i]);
            }
        }

        HashMap<u32, f32>& Widget::get_sattrs()
        {
            return m_sattrs;
        }

        HashMap<u32, Float4U>& Widget::get_vattrs()
        {
            return m_vattrs;
        }

        HashMap<u32, Name>& Widget::get_tattrs()
        {
            return m_tattrs;
        }

        HashMap<u32, ObjRef>& Widget::get_oattrs()
        {
            return m_oattrs;
        }

        f32 Widget::get_sattr(u32 key, bool recursive, f32 default_value, bool* found)
        {
            auto& sattrs = get_sattrs();
            auto iter = sattrs.find(key);
            if (iter != sattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = get_parent();
                while(cur)
                {
                    iter = cur->get_sattrs().find(key);
                    if (iter != cur->get_sattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }

        Float4U Widget::get_vattr(u32 key, bool recursive, const Float4U& default_value, bool* found)
        {
            auto& vattrs = get_vattrs();
            auto iter = vattrs.find(key);
            if (iter != vattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = get_parent();
                while(cur)
                {
                    iter = cur->get_vattrs().find(key);
                    if (iter != cur->get_vattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }

        Name Widget::get_tattr(u32 key, bool recursive, const Name& default_value, bool* found)
        {
            auto& tattrs = get_tattrs();
            auto iter = tattrs.find(key);
            if (iter != tattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = get_parent();
                while(cur)
                {
                    iter = cur->get_tattrs().find(key);
                    if (iter != cur->get_tattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }

        object_t Widget::get_oattr(u32 key, bool recursive, object_t default_value, bool* found)
        {
            auto& oattrs = get_oattrs();
            auto iter = oattrs.find(key);
            if (iter != oattrs.end())
            {
                if (found) *found = true;
                return iter->second.get();
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = get_parent();
                while(cur)
                {
                    iter = cur->get_oattrs().find(key);
                    if (iter != cur->get_oattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second.get();
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }

        f32 Widget::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            return get_desired_size_x_attr(this, type);
        }

        f32 Widget::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            return get_desired_size_y_attr(this, type);
        }

        RV Widget::begin_update(IContext* ctx)
        {
            return ok;
        }

        RV Widget::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            m_bounding_rect = layout_rect;
            return ok;
        }

        OffsetRectF Widget::get_bounding_rect()
        {
            return m_bounding_rect;
        }

        void Widget::set_bounding_rect(const OffsetRectF& bounding_rect)
        {
            m_bounding_rect = bounding_rect;
        }

        bool Widget::contains_point(f32 x, f32 y)
        {
            return in_bounds(Float2(x, y), Float2(m_bounding_rect.left, m_bounding_rect.top), Float2(m_bounding_rect.right, m_bounding_rect.bottom));
        }

        RV Widget::handle_event(IContext* ctx, object_t e, bool& handled)
        {
            return ok;
        }

        RV Widget::update(IContext* ctx)
        {
            return ok;
        }

        RV Widget::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            return ok;
        }
    }
}