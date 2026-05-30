/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EditObject.cpp
* @author JXMaster
* @date 2020/5/29
*/
#include "EditObject.hpp"
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "MainEditor.hpp"

namespace Luna
{
    namespace
    {
        struct GUIPropertyRow
        {
            Float2 size;
        };

        u32 g_gui_property_flow_depth = 0;

        GUIPropertyRow begin_gui_property_row(const c8* name, f32 height = 30.0f)
        {
            (void)g_gui_property_flow_depth;
            GUI::LayoutStyle row_style = GUI::LayoutStyle::fill_width();
            row_style.height_policy = GUI::SizePolicy::fixed;
            row_style.fixed_height_value = height;
            GUI::set_next_item_layout(row_style);
            GUI::LayoutDesc row;
            row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
            GUI::begin_h_layout(name, row);
            GUI::set_next_item_layout(GUI::LayoutStyle::fill_width());
            return GUIPropertyRow { Float2(0.0f, height) };
        }

        bool end_gui_property_row(const GUIPropertyRow& row, GUI::ItemHandle item)
        {
            (void)row;
            GUI::end_h_layout();
            return GUI::get_item_state(item, GUI::State::value_changed());
        }

        void end_gui_property_row(const GUIPropertyRow& row)
        {
            (void)row;
            GUI::end_h_layout();
        }

        template <typename _Ty>
        _Ty& get_edit_buffer(HashMap<usize, _Ty>& buffers, usize key, const _Ty& default_value)
        {
            auto iter = buffers.find(key);
            if(iter == buffers.end())
            {
                iter = buffers.insert_or_assign(key, default_value).first;
            }
            return iter->second;
        }

        Float3 quaternion_to_euler_degrees(const Float4& quaternion)
        {
            Float3 euler = AffineMatrix::euler_angles(AffineMatrix::make_rotation(quaternion));
            euler *= 180.0f / PI;
            if (euler.x > 89.0f || euler.x < -89.0f)
            {
                euler.z = 0.0f;
            }
            return euler;
        }

        HashMap<usize, f32> g_radian_edit_buffers;
        HashMap<usize, Float3> g_quaternion_edit_buffers;
        HashMap<usize, String> g_name_edit_buffers;
    }

    void push_edit_object_gui_flow_layout()
    {
        ++g_gui_property_flow_depth;
    }

    void pop_edit_object_gui_flow_layout()
    {
        luassert(g_gui_property_flow_depth);
        --g_gui_property_flow_depth;
    }

    bool edit_enum(const c8* name, typeinfo_t type, void* obj)
    {
        auto descs = get_enum_options(type);
        bool edited = false;
        if (is_multienum_type(type))
        {
            // TODO.

        }
        else
        {
            if(descs.empty()) return false;
            i64 value = get_enum_instance_value(type, obj);
            usize current_item = 0;
            for (usize i = 0; i < descs.size(); ++i)
            {
                auto& desc = descs[i];
                if (value == desc.value)
                {
                    current_item = i;
                    break;
                }
            }
            String label;
            strprintf(label, "%s: %s", name, descs[current_item].name.c_str());
            GUIPropertyRow row = begin_gui_property_row(name);
            GUI::ItemHandle button = GUI::button(label.c_str());
            end_gui_property_row(row);
            if(GUI::is_item_clicked(button))
            {
                current_item = (current_item + 1) % descs.size();
                set_enum_instance_value(type, obj, descs[current_item].value);
                edited = true;
            }
        }
        return edited;
    }

    static bool edit_property(const c8* name, typeinfo_t object_type, typeinfo_t type, void* obj)
    {
        auto hide = get_property_attribute(object_type, name, "hide");
        if (hide.boolean())
        {
            return false;
        }

        GUI::push_id(obj);

        bool edited = false;

        // A very simple GUI implementation based on type reflection.
        if (is_primitive_type(type))
        {
            if (type == f32_type())
            {
                f32* data = (f32*)obj;
                f32 v_min = 0.0f;
                f32 v_max = 0.0f;
                auto gui_min = get_property_attribute(object_type, name, "gui_min");
                if (gui_min.valid())
                {
                    v_min = gui_min.fnum();
                }
                auto gui_max = get_property_attribute(object_type, name, "gui_max");
                if (gui_max.valid())
                {
                    v_max = gui_max.fnum();
                }
                if (get_property_attribute(object_type, name, "radian") == true)
                {
                    f32 v_min_deg = rad_to_deg(v_min);
                    f32 v_max_deg = rad_to_deg(v_max);
                    f32& v_edit = get_edit_buffer(g_radian_edit_buffers, (usize)obj, rad_to_deg(*data));
                    f32 speed = (v_max_deg <= v_min_deg) ? 1.0f : (v_max_deg - v_min_deg) / 100.0f;
                    GUIPropertyRow row = begin_gui_property_row(name);
                    GUI::ItemHandle item = GUI::drag_float(name, &v_edit, speed, v_min_deg, v_max_deg);
                    edited = end_gui_property_row(row, item);
                    if (edited)
                    {
                        *data = deg_to_rad(v_edit);
                    }
                    else if(!GUI::is_item_active(item) && !GUI::is_item_focused(item))
                    {
                        v_edit = rad_to_deg(*data);
                    }
                }
                else
                {
                    f32 speed = 1.0f;
                    if (v_max - v_min != 0.0f)
                    {
                        speed = (v_max - v_min) / 100.0f;
                    }
                    GUIPropertyRow row = begin_gui_property_row(name);
                    GUI::ItemHandle item = GUI::drag_float(name, data, speed, v_min, v_max);
                    edited = end_gui_property_row(row, item);
                }
            }
            else if (type == boolean_type())
            {
                bool* data = (bool*)obj;
                GUIPropertyRow row = begin_gui_property_row(name, 26.0f);
                GUI::ItemHandle item = GUI::checkbox(name, data);
                edited = end_gui_property_row(row, item);
            }
        }
        else if (is_enum_type(type))
        {
            edited = edit_enum(name, type, obj);
        }
        // Only support common structure.
        else if (type == typeof<Float2>())
        {
            Float2* data = (Float2*)obj;
            GUIPropertyRow row = begin_gui_property_row(name);
            GUI::ItemHandle item = GUI::drag_float2(name, data->m, 0.01f, 0.0f, 0.0f);
            edited = end_gui_property_row(row, item);
        }
        else if (type == typeof<Float3>())
        {
            auto color_gui = get_property_attribute(object_type, name, "color_gui");
            if (color_gui == true)
            {
                Float3* data = (Float3*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::ItemHandle item = GUI::color_edit3(name, data->m);
                edited = end_gui_property_row(row, item);
            }
            else
            {
                Float3* data = (Float3*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::ItemHandle item = GUI::drag_float3(name, data->m, 0.01f, 0.0f, 0.0f);
                edited = end_gui_property_row(row, item);
            }
        }
        else if (type == typeof<Float4>())
        {
            auto quat = get_property_attribute(object_type, name, "quaternion");
            if(quat == true)
            {
                Float4* data = (Float4*)obj;
                Float3& euler = get_edit_buffer(g_quaternion_edit_buffers, (usize)obj, quaternion_to_euler_degrees(*data));
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::ItemHandle item = GUI::drag_float3(name, euler.m, 0.1f, 0.0f, 0.0f);
                edited = end_gui_property_row(row, item);
                if (edited)
                {
                    Float3 radians = euler * (PI / 180.0f);
                    *data = Quaternion::from_euler_angles(radians);
                }
                else if(!GUI::is_item_active(item) && !GUI::is_item_focused(item))
                {
                    euler = quaternion_to_euler_degrees(*data);
                }
            }
            else
            {
                Float4* data = (Float4*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::ItemHandle item = GUI::drag_float4(name, data->m, 0.01f, 0.0f, 0.0f);
                edited = end_gui_property_row(row, item);
            }
        }
        else if (type == typeof<Asset::asset_t>())
        {
            Asset::asset_t* asset = (Asset::asset_t*)obj;
            edited = edit_asset(name, *asset);
        }
        else if (type == typeof<Name>())
        {
            Name* data = (Name*)obj;
            String& buf = get_edit_buffer(g_name_edit_buffers, (usize)obj, String(data->c_str()));
            GUIPropertyRow row = begin_gui_property_row(name);
            GUI::ItemHandle item = GUI::input_text(name, buf);
            edited = end_gui_property_row(row, item);
            if (edited)
            {
                *data = buf;
            }
            else if(!GUI::is_item_active(item) && !GUI::is_item_focused(item))
            {
                buf = data->c_str();
            }
        }

        GUI::pop_id();
        return edited;
    }

    bool edit_object(typeinfo_t type, void* data)
    {
        Vector<StructurePropertyDesc> properties;
        get_struct_properties(type, properties);
        bool edited = false;
        for (usize i = 0; i < properties.size(); ++i)
        {
            auto& desc = properties[i];
            edited = edited || edit_property(desc.name.c_str(), type, desc.type, (void*)((usize)data + desc.offset));
        }
        return edited;
    }

    static bool edit_scene_object_property(World* world, const c8* name, typeinfo_t object_type, typeinfo_t type, void* obj)
    {
        if (type == typeof<ActorRef>())
        {
            GUI::push_id(obj);
            ActorRef* ref = (ActorRef*)obj;
            bool edited = edit_actor_ref(name, world, *ref);
            GUI::pop_id();
            return edited;
        }
        else
        {
            return edit_property(name, object_type, type, obj);
        }
    }

    bool edit_scene_object(World* world, typeinfo_t type, void* data)
    {
        Vector<StructurePropertyDesc> properties;
        get_struct_properties(type, properties);
        bool edited = false;
        for (usize i = 0; i < properties.size(); ++i)
        {
            auto& desc = properties[i];
            edited = edited || edit_scene_object_property(world, desc.name.c_str(), type, desc.type, (void*)((usize)data + desc.offset));
        }
        return edited;
    }

    bool edit_asset(const c8* name, Asset::asset_t& asset)
    {
        bool edited = false;
        GUI::push_id(&asset);

        GUIPropertyRow row = begin_gui_property_row(name, 104.0f);
        GUI::set_next_item_layout(GUI::LayoutStyle::fixed_width(100.0f));
        GUI::ItemHandle target = GUI::button(asset ? "" : "(None)");

        Name asset_ref_payload_type("Asset Ref");
        if (GUI::begin_drag_drop_target(target, asset_ref_payload_type))
        {
            GUI::end_drag_drop_target();
        }
        if (const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(target, asset_ref_payload_type))
        {
            if (const Asset::asset_t* data = payload->data_as<Asset::asset_t>())
            {
                asset = *data;
                edited = true;
            }
        }

        if (asset)
        {
            RectF draw_rect = GUI::get_item_state(target, GUI::State::rect());
            if(draw_rect.width > 1.0f && draw_rect.height > 1.0f)
            {
                draw_asset_tile(asset, draw_rect);
            }
            auto path = Asset::get_asset_path(asset);
            GUI::set_next_item_layout(GUI::LayoutStyle::fill_width());
            GUI::text(path.encode().c_str());
            GUI::set_next_item_layout(GUI::LayoutStyle::fixed_width(72.0f));
            GUI::ItemHandle clear_button = GUI::button("Clear");
            if (GUI::is_item_clicked(clear_button))
            {
                asset.reset();
                edited = true;
            }
        }
        else
        {
            GUI::set_next_item_layout(GUI::LayoutStyle::fill_width());
            GUI::text("(drop asset here)");
        }

        GUI::set_next_item_layout(GUI::LayoutStyle::fixed_width(120.0f));
        GUI::text(name);
        end_gui_property_row(row);
        GUI::pop_id();

        return edited;
    }

    bool edit_actor_ref(const c8* name, World* world, ActorRef& ref)
    {
        bool edited = false;
        GUI::push_id(&ref);

        const c8* actor_name = "(None)";
        if(ref.guid != Guid(0, 0))
        {
            Actor* actor = world->get_actor(ref.guid);
            if(actor)
            {
                actor_name = actor->get_actor_info()->name.c_str();
            }
        }

        GUIPropertyRow row = begin_gui_property_row(name, 30.0f);
        GUI::set_next_item_layout(GUI::LayoutStyle::fixed_width(160.0f));
        GUI::ItemHandle target = GUI::button(actor_name);

        Name actor_ref_payload_type("Actor Ref");
        if (GUI::begin_drag_drop_target(target, actor_ref_payload_type))
        {
            GUI::end_drag_drop_target();
        }
        if (const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(target, actor_ref_payload_type))
        {
            if (const Guid* data = payload->data_as<Guid>())
            {
                ref.guid = *data;
                edited = true;
            }
        }

        GUI::set_next_item_layout(GUI::LayoutStyle::fill_width());
        GUI::text(name);
        end_gui_property_row(row);
        GUI::pop_id();
        return edited;
    }
}
