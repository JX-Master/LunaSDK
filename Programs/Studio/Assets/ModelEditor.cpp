/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ModelEditor.cpp
* @author JXMaster
* @date 2020/5/27
*/
#include "Model.hpp"
#include "../StudioHeader.hpp"
#include <Window/MessageBox.hpp>
#include "../EditObject.hpp"
#include "../Mesh.hpp"
namespace Luna
{
	class ModelEditor : public IAssetEditor
	{
	public:
		lustruct("ModelEditor", "{46d8b09d-1d7d-4deb-95b1-ac008c7998d4}");
		luiimpl();

		Asset::asset_t m_model;

		String m_mesh_name;

		Vector<String> m_mat_names;

		bool m_open = true;

		ModelEditor() {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	void ModelEditor::on_render()
	{
		char title[32];
		sprintf_s(title, "Model Editor###%d", (u32)(usize)this);
		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

		Ref<Model> model = Asset::get_asset_data(m_model);
		if (!model || (Asset::get_asset_state(m_model) != Asset::AssetState::loaded))
		{
			ImGui::Text("Model Asset is not loaded.");
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
							luexp(Asset::save_asset(m_model));
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

			edit_asset("Mesh Asset", model->mesh);
			if (model->mesh)
			{
				Ref<Mesh> mesh = Asset::get_asset_data(model->mesh);
				if (mesh)
				{
					ImGui::Text("This mesh requires %u material(s).", (u32)mesh->pieces.size());
				}
			}

			u32 num_mats = (u32)model->materials.size();
			m_mat_names.resize(num_mats);
			i32 remove_index = -1;
			i32 add_index = -1;
			for (u32 i = 0; i < num_mats; ++i)
			{
				char mat_name[32];
				sprintf_s(mat_name, "Material slot %u", i);
				edit_asset(mat_name, model->materials[i]);
				ImGui::SameLine();
				ImGui::PushID(i);
				if (ImGui::Button("Remove current slot"))
				{
					remove_index = i;
				}
				ImGui::SameLine();
				if (ImGui::Button("Add before this"))
				{
					add_index = i;
				}
				ImGui::PopID();
			}
			if (remove_index >= 0)
			{
				model->materials.erase(model->materials.begin() + remove_index);
			}
			else if (add_index >= 0)
			{
				model->materials.insert(model->materials.begin() + add_index, Asset::asset_t());
			}
			if (ImGui::Button("Add a new material slot"))
			{
				model->materials.push_back(Asset::asset_t());
			}
		}

		ImGui::End();
	}
	Ref<IAssetEditor> new_model_editor(object_t userdata, Asset::asset_t editing_asset)
	{
		auto edt = new_object<ModelEditor>();
		edt->m_model = editing_asset;
		return edt;
	}
	struct ModelCreator : public IAssetEditor
	{
		lustruct("ModelCreator", "{bd5b3a4d-d52c-4a6d-9e49-49a083096039}");
		luiimpl();

		Path m_create_dir;
		String m_asset_name;
		bool m_open;

		ModelCreator() :
			m_open(true) {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};
	void ModelCreator::on_render()
	{
		char title[32];
		sprintf_s(title, "Create Model###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		ImGui::InputText("Model Asset Name", m_asset_name);
		if (!m_asset_name.empty())
		{
			ImGui::Text("The Model will be created as: %s%s", m_create_dir.encode().c_str(), m_asset_name.c_str());
			if (ImGui::Button("Create"))
			{
				lutry
				{
					Path asset_path = m_create_dir;
					asset_path.push_back(m_asset_name);
					lulet(asset, Asset::new_asset(asset_path, get_model_asset_type()));
					Ref<Model> mat = new_object<Model>();
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
	void register_model_editor()
	{
		register_boxed_type<ModelEditor>();
		impl_interface_for_type<ModelEditor, IAssetEditor>();
		AssetEditorDesc desc;
		desc.new_editor = new_model_editor;
		desc.on_draw_tile = nullptr;
		g_env->register_asset_editor_type(get_model_asset_type(), desc);
	}
	Ref<IAssetEditor> new_model_importer(const Path& create_dir)
	{
		auto dialog = new_object<ModelCreator>();
		dialog->m_create_dir = create_dir;
		return dialog;
	}
	void register_model_importer()
	{
		register_boxed_type<ModelCreator>();
		impl_interface_for_type<ModelCreator, IAssetEditor>();
		AssetImporterDesc desc;
		desc.new_importer = new_model_importer;
		g_env->register_asset_importer_type(get_model_asset_type(), desc);
	}
}