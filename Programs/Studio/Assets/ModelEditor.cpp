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
#include "ModelEditor.hpp"
#include "../StudioHeader.hpp"
#include "../StudioGUI.hpp"
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/Window/MessageBox.hpp>
#include "../Mesh.hpp"
namespace Luna
{
    namespace
    {
        GUI::LayoutConfig fixed_size(f32 width, f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUI::LayoutConfig fixed_height(f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUI::FlexLayoutDesc vertical_editor_layout()
        {
            GUI::FlexLayoutDesc desc;
            desc.axis = GUI::LayoutAxis::y;
            desc.main_axis_gap = 8.0f;
            return desc;
        }

        GUI::FlexLayoutDesc material_slot_row_layout()
        {
            GUI::FlexLayoutDesc desc;
            desc.axis = GUI::LayoutAxis::x;
            desc.main_axis_gap = 8.0f;
            return desc;
        }
    }

    void ModelEditor::on_render(GUI::IContext* context, const GUI::LayoutConfig& layout)
    {
        if(!m_open) return;

        context->push_data_scope(context->make_id((GUI::id_t)(usize)this));
        GUI::ElementHandle root = EditorGUI::begin_v_layout(context, context->make_id("model_editor"), "Model Editor", layout);

        Ref<Model> model = get_asset_or_async_load_if_not_ready<Model>(m_model);
        if(!model || (Asset::get_asset_state(m_model) != Asset::AssetState::loaded))
        {
            EditorGUI::text(context, context->make_id("not_loaded"), "Model Asset is not loaded.", fixed_height(24.0f));
        }
        else
        {
            GUI::ElementHandle save_button = EditorGUI::text_button(context, context->make_id("save"), "Save", fixed_height(30.0f));
            if(EditorGUI::is_item_clicked(context, save_button))
            {
                lutry
                {
                    luexp(Asset::save_asset(m_model));
                }
                lucatch
                {
                    auto _ = Window::message_box(explain(luerr), "Failed to save asset", {"OK"},
                        Window::MessageBoxIcon::error);
                }
            }

            gui_edit_asset_path(context, "Mesh Asset", model->mesh, m_mesh_name, "Failed to set mesh asset reference");
            if(model->mesh)
            {
                Ref<Mesh> mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);
                if(mesh)
                {
                    char mesh_info[64];
                    snprintf(mesh_info, 64, "This mesh requires %u material(s).", (u32)mesh->pieces.size());
                    EditorGUI::text(context, context->make_id("mesh_info"), mesh_info, fixed_height(24.0f));
                }
            }

            u32 num_mats = (u32)model->materials.size();
            m_mat_names.resize(num_mats);
            i32 remove_index = -1;
            i32 add_index = -1;
            for(u32 i = 0; i < num_mats; ++i)
            {
                char mat_name[32];
                snprintf(mat_name, 32, "Material slot %u", i);
                context->push_data_scope(context->make_id((GUI::id_t)i));
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, context->make_id("material_slot_row"), mat_name,
                    fixed_height(30.0f));
                gui_edit_asset_path(context, mat_name, model->materials[i], m_mat_names[i], "Failed to set material asset reference");
                GUI::ElementHandle remove_button = EditorGUI::text_button(context, context->make_id("remove"), "Remove current slot",
                    fixed_size(152.0f, 30.0f));
                GUI::ElementHandle add_button = EditorGUI::text_button(context, context->make_id("add"), "Add before this",
                    fixed_size(128.0f, 30.0f));
                EditorGUI::end_h_layout(context, row, material_slot_row_layout());
                if(EditorGUI::is_item_clicked(context, remove_button))
                {
                    remove_index = i;
                }
                if(EditorGUI::is_item_clicked(context, add_button))
                {
                    add_index = i;
                }
                context->pop_data_scope();
            }
            if(remove_index >= 0)
            {
                model->materials.erase(model->materials.begin() + remove_index);
            }
            else if(add_index >= 0)
            {
                model->materials.insert(model->materials.begin() + add_index, Asset::asset_t());
            }
            if(EditorGUI::is_item_clicked(context, EditorGUI::text_button(context, context->make_id("add_material_slot"),
                "Add a new material slot", fixed_height(30.0f))))
            {
                model->materials.push_back(Asset::asset_t());
            }
        }

        EditorGUI::end_v_layout(context, root, vertical_editor_layout());
        context->pop_data_scope();
    }

    Ref<IAssetEditor> new_model_editor(object_t userdata, Asset::asset_t editing_asset)
    {
        auto edt = new_object<ModelEditor>();
        edt->m_model = editing_asset;
        return edt;
    }
    void register_model_editor()
    {
        AssetEditorDesc desc;
        desc.new_editor = new_model_editor;
        g_env->register_asset_editor_type(get_model_asset_type(), desc);
    }
}
