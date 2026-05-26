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
#include <Luna/ImGui/ImGui.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include "MainEditor.hpp"

namespace Luna
{
    namespace
    {
        struct GUIPropertyRow
        {
            Float2 size;
        };

        GUIPropertyRow begin_gui_property_row(const c8* name, f32 height = 30.0f)
        {
            Float2 pos = ImGui::GetCursorScreenPos();
            Float2 avail = ImGui::GetContentRegionAvail();
            f32 width = max(avail.x, 240.0f);
            GUI::GUILayoutDesc row;
            row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::stretch;
            GUI::BeginHLayout(name, RectF(pos.x, pos.y, width, height), row);
            GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
            return GUIPropertyRow { Float2(width, height) };
        }

        bool end_gui_property_row(const GUIPropertyRow& row, GUI::GUIItemHandle item)
        {
            GUI::EndHLayout();
            ImGui::Dummy(row.size);
            return GUI::GetItemState(item, GUI::GUIState::value_changed());
        }
    }

    /*static bool edit_primitive(const Name& name, typeinfo_t type, void* obj)
    {
        if (type == boolean_type()) return ImGui::Checkbox(name.c_str(), (bool*)obj);

    }*/

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
            GUI::GUIItemHandle button = GUI::Button(label.c_str());
            GUI::EndHLayout();
            ImGui::Dummy(row.size);
            if(GUI::IsItemClicked(button))
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

        ImGui::PushID((int)(usize)obj);
        GUI::PushID(obj);

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
                    f32 v_edit = rad_to_deg(*data);
                    v_min = rad_to_deg(v_min);
                    v_max = rad_to_deg(v_max);
                    f32 speed = (v_max <= v_min) ? 1.0f : (v_max - v_min) / 100.0f;
                    ImGui::DragFloat(name, &v_edit, speed, v_min, v_max);
                    if (ImGui::IsItemEdited())
                    {
                        *data = deg_to_rad(v_edit);
                        edited = true;
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
                    GUI::GUIItemHandle item = GUI::DragFloat(name, data, speed, v_min, v_max);
                    edited = end_gui_property_row(row, item);
                }
            }
            else if (type == boolean_type())
            {
                bool* data = (bool*)obj;
                GUIPropertyRow row = begin_gui_property_row(name, 26.0f);
                GUI::GUIItemHandle item = GUI::Checkbox(name, data);
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
            GUI::GUIItemHandle item = GUI::DragFloat2(name, data->m, 0.01f, 0.0f, 0.0f);
            edited = end_gui_property_row(row, item);
        }
        else if (type == typeof<Float3>())
        {
            auto color_gui = get_property_attribute(object_type, name, "color_gui");
            if (color_gui == true)
            {
                Float3* data = (Float3*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::GUIItemHandle item = GUI::ColorEdit3(name, data->m);
                edited = end_gui_property_row(row, item);
            }
            else
            {
                Float3* data = (Float3*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::GUIItemHandle item = GUI::DragFloat3(name, data->m, 0.01f, 0.0f, 0.0f);
                edited = end_gui_property_row(row, item);
            }
        }
        else if (type == typeof<Float4>())
        {
            auto quat = get_property_attribute(object_type, name, "quaternion");
            if(quat == true)
            {
                Float4* data = (Float4*)obj;
                auto euler = AffineMatrix::euler_angles(AffineMatrix::make_rotation(*data));
                euler *= 180.0f / PI;
                if (euler.x > 89.0f || euler.x < -89.0f)
                {
                    euler.z = 0.0f;
                }
                ImGui::DragFloat3(name, euler.m);
                if (ImGui::IsItemEdited())
                {
                    edited = true;
                    euler *= PI / 180.0f;
                    *data = Quaternion::from_euler_angles(euler);
                }
            }
            else
            {
                Float4* data = (Float4*)obj;
                GUIPropertyRow row = begin_gui_property_row(name);
                GUI::GUIItemHandle item = GUI::DragFloat4(name, data->m, 0.01f, 0.0f, 0.0f);
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
            String buf = data->c_str();
            if (ImGui::InputText(name, buf))
            {
                *data = buf;
                edited = true;
            }
        }

        GUI::PopID();
        ImGui::PopID();
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
            ImGui::PushID((int)(usize)obj);
            ActorRef* ref = (ActorRef*)obj;
            bool edited = edit_actor_ref(name, world, *ref);
            ImGui::PopID();
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
        using namespace ImGui;

        bool edited = false;

        String label = "##";
        if (asset)
        {
            label.append(Asset::get_asset_type(asset).c_str());
        }

        ImGui::Button(label.c_str(), {100, 100});

        if (BeginDragDropTarget())
        {
            const ImGuiPayload* payload = AcceptDragDropPayload("Asset Ref");
            if (payload)
            {
                const Asset::asset_t* data = (const Asset::asset_t*)payload->Data;
                asset = *data;
                edited = true;
            }
            EndDragDropTarget();
        }

        if (asset)
        {
            ImGui::SameLine();

            auto pos_after = ImGui::GetCursorScreenPos();

            auto pos = ImGui::GetItemRectMin();
            auto size = ImGui::GetItemRectSize();
            RectF draw_rect = RectF(pos.x, pos.y, size.x, size.y);
            draw_asset_tile(asset, draw_rect);

            ImGui::SetCursorScreenPos(pos_after);
            auto path = Asset::get_asset_path(asset);
            Text("%s", path.encode().c_str());
            SameLine();
            PushID(name);
            if (Button("Clear"))
            {
                asset.reset();
                edited = true;
            }
            PopID();
        }

        ImGui::SameLine();
        ImGui::Text("%s", name);

        return edited;
    }

    bool edit_actor_ref(const c8* name, World* world, ActorRef& ref)
    {
        using namespace ImGui;

        bool edited = false;

        const c8* actor_name = "(None)";
        if(ref.guid != Guid(0, 0))
        {
            Actor* actor = world->get_actor(ref.guid);
            if(actor)
            {
                actor_name = actor->get_actor_info()->name.c_str();
            }
        }

        ImGui::Button(actor_name, {100.0f, ImGui::GetTextLineHeightWithSpacing()});

        if (BeginDragDropTarget())
        {
            const ImGuiPayload* payload = AcceptDragDropPayload("Actor Ref");
            if (payload)
            {
                const Guid* data = (const Guid*)payload->Data;
                ref.guid = *data;
                edited = true;
            }
            EndDragDropTarget();
        }

        ImGui::SameLine();
        ImGui::Text("%s", name);
        return edited;
    }
}
