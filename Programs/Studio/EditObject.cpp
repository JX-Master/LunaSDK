/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EditObject.cpp
* @author JXMaster
* @date 2020/5/29
*/
#include "EditObject.hpp"
#include <Runtime/Reflection.hpp>
#include <ImGui/ImGui.hpp>
#include <Runtime/Math/Color.hpp>
#include <Runtime/Math/Transform.hpp>
#include "MainEditor.hpp"

namespace Luna
{
	/*static bool edit_primitive(const Name& name, typeinfo_t type, void* obj)
	{
		if (type == boolean_type()) return ImGui::Checkbox(name.c_str(), (bool*)obj);

	}*/

	void edit_enum(const c8* name, typeinfo_t type, void* obj)
	{
		usize num_options = count_enum_options(type);
		if (is_multienum_type(type))
		{
			// TODO.

		}
		else
		{
			const c8** options = (const c8**)alloca(sizeof(const c8*) * num_options);
			i64 value = get_enum_instance_value(type, obj);
			int current_item = -1;
			for (usize i = 0; i < num_options; ++i)
			{
				auto desc = get_enum_option(type, i);
				options[i] = desc.name.c_str();
				if (value == desc.value)
				{
					current_item = (int)i;
				}
			}
			ImGui::Combo(name, &current_item, options, (int)num_options);
			value = get_enum_option(type, current_item).value;
			set_enum_instance_value(type, obj, value);
		}
	}

	static void edit_property(const c8* name, typeinfo_t object_type, typeinfo_t type, void* obj)
	{
		auto hide = get_property_attribute(object_type, name, "hide");
		if (hide.boolean())
		{
			return;
		}

		ImGui::PushID((int)(usize)obj);

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
					}
				}
				else
				{
					f32 speed = 1.0f;
					if (v_max - v_min != 0.0f)
					{
						speed = (v_max - v_min) / 100.0f;
					}
					ImGui::DragFloat(name, data, speed, v_min, v_max);
				}
			}
			else if (type == boolean_type())
			{
				bool* data = (bool*)obj;
				ImGui::Checkbox(name, data);
			}
		}
		else if (is_enum_type(type))
		{
			edit_enum(name, type, obj);
		}
		// Only support common structure.
		else if (type == typeof<Float2>())
		{
			Float2* data = (Float2*)obj;
			ImGui::DragFloat2(name, data->m);
		}
		else if (type == typeof<Float3>())
		{
			auto color_gui = get_property_attribute(object_type, name, "color_gui");
			if (color_gui == true)
			{
				Float3* data = (Float3*)obj;
				ImGui::ColorEdit3(name, data->m);
			}
			else
			{
				Float3* data = (Float3*)obj;
				ImGui::DragFloat3(name, data->m);
			}
		}
		else if (type == typeof<Float4>())
		{
			Float4* data = (Float4*)obj;
			ImGui::DragFloat4(name, data->m);
		}
		else if (type == typeof<Color>())
		{
			Color* data = (Color*)obj;
			ImGui::ColorEdit4(name, data->m);
		}
		else if (type == typeof<Quaternion>())
		{
			Quaternion* data = (Quaternion*)obj;
			auto euler = AffineMatrix::make_rotation(*data).euler_angles();
			euler *= 180.0f / PI;
			if (euler.x > 89.0f || euler.x < -89.0f)
			{
				euler.z = 0.0f;
			}
			ImGui::DragFloat3(name, euler.m);
			if (ImGui::IsItemEdited())
			{
				euler *= PI / 180.0f;
				*data = Quaternion::from_euler_angles(euler);
			}
		}
		else if (type == typeof<Asset::asset_t>())
		{
			Asset::asset_t* asset = (Asset::asset_t*)obj;
			edit_asset(name, *asset);
		}
		else if (type == typeof<Name>())
		{
			Name* data = (Name*)obj;
			String buf = data->c_str();
			if (ImGui::InputText(name, buf))
			{
				*data = buf;
			}
		}

		ImGui::PopID();
	}

	void edit_object(object_t obj)
	{
		auto type = get_object_type(obj);
		usize num_properties = count_struct_properties(type);
		for (usize i = 0; i < num_properties; ++i)
		{
			auto desc = get_struct_property(type, i);
			edit_property(desc.name.c_str(), type, desc.type, (void*)((usize)obj + desc.offset));
		}
	}

	void edit_asset(const c8* name, Asset::asset_t& asset)
	{
		using namespace ImGui;

		auto begin_pos = ImGui::GetCursorScreenPos();

		String label = "##";
		if (asset)
		{
			label.append(Asset::get_asset_type(asset).c_str());
		}

		ImGui::Text(name);
		ImGui::SameLine();

		auto pos_before = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos({ pos_before.x, pos_before.y + 10 });
		ImGui::Button(label.c_str(), {100, 100});

		if (BeginDragDropTarget())
		{
			const ImGuiPayload* payload = AcceptDragDropPayload("Asset Ref");
			if (payload)
			{
				const Asset::asset_t* data = (const Asset::asset_t*)payload->Data;
				asset = *data;
			}
			EndDragDropTarget();
		}

		if (asset)
		{
			ImGui::SameLine();

			auto pos_after = ImGui::GetCursorScreenPos();

			RectF draw_rect = RectF(pos_before.x, pos_before.y + 10, 100, 100);
			draw_asset_tile(asset, draw_rect);

			ImGui::SetCursorScreenPos(pos_after);
			auto path = Asset::get_asset_path(asset);
			Text(path.encode().c_str());
			SameLine();
			PushID(name);
			if (Button("Clear"))
			{
				asset = nullptr;
			}
			PopID();
		}

		ImGui::SetCursorScreenPos({ begin_pos.x, begin_pos.y + 120 });
	}
}