/*!
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
#include <Luna/Window/MessageBox.hpp>
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
        snprintf(title, 32, "Model Editor###%d", (u32)(usize)this);
        ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

        Ref<Model> model = get_asset_or_async_load_if_not_ready<Model>(m_model);
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
                            auto _ = Window::message_box(explain(luerr), "Failed to save asset", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            edit_asset("Mesh Asset", model->mesh);
            if (model->mesh)
            {
                Ref<Mesh> mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);
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
                auto pos = ImGui::GetCursorPos();
                char mat_name[32];
                snprintf(mat_name, 32, "Material slot %u", i);
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
                pos.y += 110;
                ImGui::SetCursorPos(pos);
            }
            if (remove_index >= 0)
            {
                model->materials.erase(model->materials.begin() + remove_index);
            }
            else if (add_index >= 0)
            {
                model->materials.insert(model->materials.begin() + add_index, Asset::asset_t());
            }
            auto pos = ImGui::GetCursorPos();
            pos.y += 100;
            ImGui::SetCursorPos(pos);
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
    void register_model_editor()
    {
        register_boxed_type<ModelEditor>();
        impl_interface_for_type<ModelEditor, IAssetEditor>();
        AssetEditorDesc desc;
        desc.new_editor = new_model_editor;
        desc.on_draw_tile = nullptr;
        g_env->register_asset_editor_type(get_model_asset_type(), desc);
    }
}