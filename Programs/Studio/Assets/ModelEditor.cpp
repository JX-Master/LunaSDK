/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ModelEditor.cpp
* @author JXMaster
* @date 2020/5/27
*/
#include "Model.hpp"
#include "../StudioHeader.hpp"
#include "../StudioGUI.hpp"
#include <Luna/Window/MessageBox.hpp>
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
        char title[256];
        auto path = Asset::get_asset_path(m_model);
        if(!path.empty())
        {
            snprintf(title, 256, "Model Editor - %s###%d", path.encode().c_str(), (u32)(usize)this);
        }
        else
        {
            snprintf(title, 256, "Model Editor###%d", (u32)(usize)this);
        }
        if(!m_open) return;
        GUI::BeginWindow(title, &m_open, GUI::GUISize::fixed(760.0f, 620.0f));

        Ref<Model> model = get_asset_or_async_load_if_not_ready<Model>(m_model);
        if (!model || (Asset::get_asset_state(m_model) != Asset::AssetState::loaded))
        {
            GUI::Text("Model Asset is not loaded.");
        }
        else
        {
            if (GUI::IsItemClicked(GUI::Button("Save")))
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

            gui_edit_asset_path("Mesh Asset", model->mesh, m_mesh_name, "Failed to set mesh asset reference");
            if (model->mesh)
            {
                Ref<Mesh> mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);
                if (mesh)
                {
                    char mesh_info[64];
                    snprintf(mesh_info, 64, "This mesh requires %u material(s).", (u32)mesh->pieces.size());
                    GUI::Text(mesh_info);
                }
            }

            u32 num_mats = (u32)model->materials.size();
            m_mat_names.resize(num_mats);
            i32 remove_index = -1;
            i32 add_index = -1;
            for (u32 i = 0; i < num_mats; ++i)
            {
                char mat_name[32];
                snprintf(mat_name, 32, "Material slot %u", i);
                GUI::PushID(i);
                GUI::GUILayoutDesc row;
                row.gap = 8.0f;
                row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;
                GUI::BeginHLayout("Material Slot Row", row);
                GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
                gui_edit_asset_path(mat_name, model->materials[i], m_mat_names[i], "Failed to set material asset reference");
                GUI::GUIItemHandle remove_button = GUI::Button("Remove current slot");
                GUI::GUIItemHandle add_button = GUI::Button("Add before this");
                GUI::EndHLayout();
                if (GUI::IsItemClicked(remove_button))
                {
                    remove_index = i;
                }
                if (GUI::IsItemClicked(add_button))
                {
                    add_index = i;
                }
                GUI::PopID();
            }
            if (remove_index >= 0)
            {
                model->materials.erase(model->materials.begin() + remove_index);
            }
            else if (add_index >= 0)
            {
                model->materials.insert(model->materials.begin() + add_index, Asset::asset_t());
            }
            if (GUI::IsItemClicked(GUI::Button("Add a new material slot")))
            {
                model->materials.push_back(Asset::asset_t());
            }
        }

        GUI::EndWindow();
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
