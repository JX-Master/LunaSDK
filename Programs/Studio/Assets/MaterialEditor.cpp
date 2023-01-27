/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MaterialEditor.cpp
* @author JXMaster
* @date 2020/5/29
*/
#include "Material.hpp"
#include "../StudioHeader.hpp"
#include <Window/MessageBox.hpp>
#include "../EditObject.hpp"
namespace Luna
{
	struct MaterialEditor : public IAssetEditor
	{
		lustruct("MaterialEditor", "{705b8d2f-75ef-4784-a72e-f99dcf3f67aa}");
		luiimpl();

		Asset::asset_t m_material;

		String m_base_color_name;
		String m_roughness_name;
		String m_normal_name;
		String m_metallic_name;
		String m_emissive_name;

		bool m_open = true;

		MaterialEditor() {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	void MaterialEditor::on_render()
	{
		char title[32];
		sprintf_s(title, "Material Editor###%d", (u32)(usize)this);
		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

		Ref<Material> mat = Asset::get_asset_data(m_material);
		if (!mat || (Asset::get_asset_state(m_material) != Asset::AssetState::loaded))
		{
			ImGui::Text("Material Asset is not loaded.");
		}
		else
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save"))
					{
						lutry
						{
							luexp(Asset::save_asset(m_material));
						}
						lucatch
						{
							auto _ = Window::message_box(explain(lures), "Failed to save asset", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			edit_enum("Material Type", mat->material_type);
			if (mat->material_type == MeterialType::lit)
			{
				edit_asset("Base Color", mat->base_color);
				edit_asset("Roughness", mat->roughness);
				edit_asset("Normal", mat->normal);
				edit_asset("Metallic", mat->metallic);
				edit_asset("Emissive", mat->emissive);
			}
			else
			{
				edit_asset("Emissive", mat->emissive);
			}
		}
		ImGui::End();
	}
	static Ref<IAssetEditor> material_new_editor(object_t userdata, Asset::asset_t editing_asset)
	{
		auto edt = new_object<MaterialEditor>();
		edt->m_material = editing_asset;
		return edt;
	}
	struct MaterialCreator : public IAssetEditor
	{
	public:
		lustruct("MaterialCreator", "{d87a96b8-a7e8-4a1d-b5e6-1d31987b60a9}");
		luiimpl();

		Path m_create_dir;
		String m_asset_name;
		bool m_open;

		MaterialCreator() :
			m_open(true) {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};
	void MaterialCreator::on_render()
	{
		char title[32];
		sprintf_s(title, "Create Material###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		ImGui::InputText("Material Asset Name", m_asset_name);
		if (!m_asset_name.empty())
		{
			ImGui::Text("The Material will be created as: %s%s", m_create_dir.encode().c_str(), m_asset_name.c_str());
			if (ImGui::Button("Create"))
			{
				lutry
				{
					Path asset_path = m_create_dir;
					asset_path.push_back(m_asset_name);
					lulet(asset, Asset::new_asset(asset_path, get_material_asset_type()));
					Ref<Material> mat = new_object<Material>();
					luexp(Asset::set_asset_data(asset, mat.object()));
					luexp(Asset::save_asset(asset));
				}
				lucatch
				{
					auto _ = Window::message_box(explain(lures), "Failed to create asset",
									Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
				}
			}

		}
		ImGui::End();
	}
	void register_material_editor()
	{
		register_boxed_type<MaterialEditor>();
		impl_interface_for_type<MaterialEditor, IAssetEditor>();
		AssetEditorDesc desc;
		desc.new_editor = material_new_editor;
		desc.on_draw_tile = nullptr;
		g_env->register_asset_editor_type(get_material_asset_type(), desc);
	}
	Ref<IAssetEditor> new_material_importer(const Path& create_dir)
	{
		auto dialog = new_object<MaterialCreator>();
		dialog->m_create_dir = create_dir;
		return dialog;
	}
	void register_material_importer()
	{
		register_boxed_type<MaterialCreator>();
		impl_interface_for_type<MaterialCreator, IAssetEditor>();
		AssetImporterDesc desc;
		desc.new_importer = new_material_importer;
		g_env->register_asset_importer_type(get_material_asset_type(), desc);
	}
}