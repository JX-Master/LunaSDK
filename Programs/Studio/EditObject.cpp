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
#include <Luna/GUI/Editor.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "MainEditor.hpp"

namespace Luna
{
    namespace
    {
        struct CoreGUIPropertyRow
        {
            GUICore::ElementHandle row;
        };

        GUICore::LayoutInput fixed_size(f32 width, f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::pixels;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::pixels;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutInput fixed_height(f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::pixels;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutInput fill()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }

        CoreGUIPropertyRow begin_core_gui_property_row(GUICore::IContext* context, const c8* name, f32 height = 30.0f)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, context->make_id("row"), name, fixed_height(height));
            GUI::text(context, context->make_id("label"), name, fixed_size(140.0f, height));
            return CoreGUIPropertyRow { row };
        }

        void end_core_gui_property_row(GUICore::IContext* context, const CoreGUIPropertyRow& row)
        {
            GUICore::LinearLayoutDesc desc;
            desc.axis = GUICore::LayoutAxis::x;
            desc.gap = 8.0f;
            lupanic_if_failed(GUI::end_h_layout(context, row.row, desc));
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

        bool float2_equal(const Float2& lhs, const Float2& rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        bool float3_equal(const Float3& lhs, const Float3& rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        bool float4_equal(const Float4& lhs, const Float4& rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
        }
    }

    bool edit_enum(GUICore::IContext* context, const c8* name, typeinfo_t type, void* obj)
    {
        auto descs = get_enum_options(type);
        if(descs.empty() || is_multienum_type(type))
        {
            return false;
        }
        i64 value = get_enum_instance_value(type, obj);
        i32 current_item = 0;
        Vector<const c8*> items;
        items.reserve(descs.size());
        for(usize i = 0; i < descs.size(); ++i)
        {
            auto& desc = descs[i];
            items.push_back(desc.name.c_str());
            if(value == desc.value)
            {
                current_item = (i32)i;
            }
        }
        i32 old_item = current_item;
        context->push_data_scope(context->make_id(name));
        CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
        GUI::combo(context, context->make_id("value"), name, &current_item, Span<const c8*>(items.data(), items.size()),
            fixed_height(30.0f));
        end_core_gui_property_row(context, row);
        context->pop_data_scope();
        if(current_item != old_item)
        {
            set_enum_instance_value(type, obj, descs[(usize)current_item].value);
            return true;
        }
        return false;
    }

    static bool edit_property(GUICore::IContext* context, const c8* name, typeinfo_t object_type, typeinfo_t type, void* obj)
    {
        auto hide = get_property_attribute(object_type, name, "hide");
        if(hide.boolean())
        {
            return false;
        }

        context->push_data_scope(context->make_id((GUICore::id_t)(usize)obj));
        bool edited = false;

        if(is_primitive_type(type))
        {
            if(type == f32_type())
            {
                f32* data = (f32*)obj;
                f32 old_value = *data;
                f32 v_min = 0.0f;
                f32 v_max = 0.0f;
                auto gui_min = get_property_attribute(object_type, name, "gui_min");
                if(gui_min.valid())
                {
                    v_min = gui_min.fnum();
                }
                auto gui_max = get_property_attribute(object_type, name, "gui_max");
                if(gui_max.valid())
                {
                    v_max = gui_max.fnum();
                }
                if(get_property_attribute(object_type, name, "radian") == true)
                {
                    f32 v_min_deg = rad_to_deg(v_min);
                    f32 v_max_deg = rad_to_deg(v_max);
                    f32& v_edit = get_edit_buffer(g_radian_edit_buffers, (usize)obj, rad_to_deg(*data));
                    f32 old_edit = v_edit;
                    f32 speed = (v_max_deg <= v_min_deg) ? 1.0f : (v_max_deg - v_min_deg) / 100.0f;
                    CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
                    GUICore::ElementHandle item = GUI::drag_float(context, context->make_id("value"), &v_edit,
                        speed, v_min_deg, v_max_deg, fixed_height(30.0f));
                    end_core_gui_property_row(context, row);
                    edited = (v_edit != old_edit);
                    if(edited)
                    {
                        *data = deg_to_rad(v_edit);
                    }
                    else if(!GUI::is_item_active(context, item) && !GUI::is_item_focused(context, item))
                    {
                        v_edit = rad_to_deg(*data);
                    }
                }
                else
                {
                    f32 speed = 1.0f;
                    if(v_max - v_min != 0.0f)
                    {
                        speed = (v_max - v_min) / 100.0f;
                    }
                    CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
                    GUI::drag_float(context, context->make_id("value"), data, speed, v_min, v_max, fixed_height(30.0f));
                    end_core_gui_property_row(context, row);
                    edited = (*data != old_value);
                }
            }
            else if(type == boolean_type())
            {
                bool* data = (bool*)obj;
                bool old_value = *data;
                CoreGUIPropertyRow row = begin_core_gui_property_row(context, name, 26.0f);
                GUI::checkbox(context, context->make_id("value"), "", data, fixed_height(26.0f));
                end_core_gui_property_row(context, row);
                edited = (*data != old_value);
            }
        }
        else if(is_enum_type(type))
        {
            edited = edit_enum(context, name, type, obj);
        }
        else if(type == typeof<Float2>())
        {
            Float2* data = (Float2*)obj;
            Float2 old_value = *data;
            CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
            GUI::drag_float2(context, context->make_id("value"), data->m, 0.01f, 0.0f, 0.0f, fixed_height(30.0f));
            end_core_gui_property_row(context, row);
            edited = !float2_equal(*data, old_value);
        }
        else if(type == typeof<Float3>())
        {
            Float3* data = (Float3*)obj;
            Float3 old_value = *data;
            CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
            if(get_property_attribute(object_type, name, "color_gui") == true)
            {
                GUI::color_edit3(context, context->make_id("value"), name, data->m, fixed_height(30.0f));
            }
            else
            {
                GUI::drag_float3(context, context->make_id("value"), data->m, 0.01f, 0.0f, 0.0f, fixed_height(30.0f));
            }
            end_core_gui_property_row(context, row);
            edited = !float3_equal(*data, old_value);
        }
        else if(type == typeof<Float4>())
        {
            Float4* data = (Float4*)obj;
            if(get_property_attribute(object_type, name, "quaternion") == true)
            {
                Float3& euler = get_edit_buffer(g_quaternion_edit_buffers, (usize)obj, quaternion_to_euler_degrees(*data));
                Float3 old_euler = euler;
                CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
                GUICore::ElementHandle item = GUI::drag_float3(context, context->make_id("value"), euler.m, 0.1f, 0.0f,
                    0.0f, fixed_height(30.0f));
                end_core_gui_property_row(context, row);
                edited = !float3_equal(euler, old_euler);
                if(edited)
                {
                    Float3 radians = euler * (PI / 180.0f);
                    *data = Quaternion::from_euler_angles(radians);
                }
                else if(!GUI::is_item_active(context, item) && !GUI::is_item_focused(context, item))
                {
                    euler = quaternion_to_euler_degrees(*data);
                }
            }
            else
            {
                Float4 old_value = *data;
                CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
                GUI::drag_float4(context, context->make_id("value"), data->m, 0.01f, 0.0f, 0.0f, fixed_height(30.0f));
                end_core_gui_property_row(context, row);
                edited = !float4_equal(*data, old_value);
            }
        }
        else if(type == typeof<Asset::asset_t>())
        {
            Asset::asset_t* asset = (Asset::asset_t*)obj;
            edited = edit_asset(context, name, *asset);
        }
        else if(type == typeof<Name>())
        {
            Name* data = (Name*)obj;
            String& buf = get_edit_buffer(g_name_edit_buffers, (usize)obj, String(data->c_str()));
            String old_buf = buf;
            CoreGUIPropertyRow row = begin_core_gui_property_row(context, name);
            GUICore::ElementHandle item = GUI::input_text(context, context->make_id("value"), buf, fixed_height(30.0f));
            end_core_gui_property_row(context, row);
            edited = strcmp(buf.c_str(), old_buf.c_str()) != 0;
            if(edited)
            {
                *data = buf;
            }
            else if(!GUI::is_item_active(context, item) && !GUI::is_item_focused(context, item))
            {
                buf = data->c_str();
            }
        }

        context->pop_data_scope();
        return edited;
    }

    bool edit_object(GUICore::IContext* context, typeinfo_t type, void* data)
    {
        Vector<StructurePropertyDesc> properties;
        get_struct_properties(type, properties);
        bool edited = false;
        for(usize i = 0; i < properties.size(); ++i)
        {
            auto& desc = properties[i];
            edited = edit_property(context, desc.name.c_str(), type, desc.type, (void*)((usize)data + desc.offset)) || edited;
        }
        return edited;
    }

    static bool edit_scene_object_property(GUICore::IContext* context, World* world, const c8* name, typeinfo_t object_type,
        typeinfo_t type, void* obj)
    {
        if(type == typeof<ActorRef>())
        {
            context->push_data_scope(context->make_id((GUICore::id_t)(usize)obj));
            ActorRef* ref = (ActorRef*)obj;
            bool edited = edit_actor_ref(context, name, world, *ref);
            context->pop_data_scope();
            return edited;
        }
        return edit_property(context, name, object_type, type, obj);
    }

    bool edit_scene_object(GUICore::IContext* context, World* world, typeinfo_t type, void* data)
    {
        Vector<StructurePropertyDesc> properties;
        get_struct_properties(type, properties);
        bool edited = false;
        for(usize i = 0; i < properties.size(); ++i)
        {
            auto& desc = properties[i];
            edited = edit_scene_object_property(context, world, desc.name.c_str(), type, desc.type,
                (void*)((usize)data + desc.offset)) || edited;
        }
        return edited;
    }

    bool edit_asset(GUICore::IContext* context, const c8* name, Asset::asset_t& asset)
    {
        bool edited = false;
        context->push_data_scope(context->make_id((GUICore::id_t)(usize)&asset));

        CoreGUIPropertyRow row = begin_core_gui_property_row(context, name, 82.0f);
        GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"), "Asset Reference", fill());
        GUICore::ElementHandle target = GUI::text_button(context, context->make_id("target"), asset ? "" : "(None)",
            fixed_height(32.0f));

        Name asset_ref_payload_type("Asset Ref");
        if(GUI::begin_drag_drop_target(context, target, asset_ref_payload_type))
        {
            GUI::end_drag_drop_target(context);
        }
        if(const GUICore::DragDropPayload* payload = GUI::accept_drag_drop_payload(context, target, asset_ref_payload_type))
        {
            if(const Asset::asset_t* data = payload->data_as<Asset::asset_t>())
            {
                asset = *data;
                edited = true;
            }
        }

        GUICore::ElementHandle path_row = GUI::begin_h_layout(context, context->make_id("asset_path_row"), "Asset Path Row",
            fixed_height(32.0f));
        if(asset)
        {
            auto path = Asset::get_asset_path(asset);
            GUI::text(context, context->make_id("path"), path.encode().c_str(), fixed_height(32.0f));
            GUICore::ElementHandle clear_button = GUI::text_button(context, context->make_id("clear"), "Clear",
                fixed_size(72.0f, 32.0f));
            if(GUI::is_item_clicked(context, clear_button))
            {
                asset.reset();
                edited = true;
            }
        }
        else
        {
            GUI::text(context, context->make_id("path"), "(drop asset here)", fixed_height(32.0f));
        }
        GUICore::LinearLayoutDesc path_desc;
        path_desc.axis = GUICore::LayoutAxis::x;
        path_desc.gap = 8.0f;
        lupanic_if_failed(GUI::end_h_layout(context, path_row, path_desc));

        GUICore::LinearLayoutDesc content_desc;
        content_desc.axis = GUICore::LayoutAxis::y;
        content_desc.gap = 6.0f;
        lupanic_if_failed(GUI::end_v_layout(context, content, content_desc));
        end_core_gui_property_row(context, row);

        context->pop_data_scope();
        return edited;
    }

    bool edit_actor_ref(GUICore::IContext* context, const c8* name, World* world, ActorRef& ref)
    {
        bool edited = false;
        context->push_data_scope(context->make_id((GUICore::id_t)(usize)&ref));

        const c8* actor_name = "(None)";
        if(ref.guid != Guid(0, 0))
        {
            Actor* actor = world->get_actor(ref.guid);
            if(actor)
            {
                actor_name = actor->get_actor_info()->name.c_str();
            }
        }

        CoreGUIPropertyRow row = begin_core_gui_property_row(context, name, 30.0f);
        GUICore::ElementHandle target = GUI::text_button(context, context->make_id("target"), actor_name,
            fixed_size(160.0f, 30.0f));

        Name actor_ref_payload_type("Actor Ref");
        if(GUI::begin_drag_drop_target(context, target, actor_ref_payload_type))
        {
            GUI::end_drag_drop_target(context);
        }
        if(const GUICore::DragDropPayload* payload = GUI::accept_drag_drop_payload(context, target, actor_ref_payload_type))
        {
            if(const Guid* data = payload->data_as<Guid>())
            {
                ref.guid = *data;
                edited = true;
            }
        }

        end_core_gui_property_row(context, row);
        context->pop_data_scope();
        return edited;
    }
}
