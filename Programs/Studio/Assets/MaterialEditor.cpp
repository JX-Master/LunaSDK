/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MaterialEditor.cpp
* @author JXMaster
* @date 2020/5/29
*/
#include "Material.hpp"
#include "MaterialEditor.hpp"
#include "../StudioHeader.hpp"
#include "../StudioGUI.hpp"
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/Window/MessageBox.hpp>
namespace Luna
{
    namespace
    {
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
    }

    void MaterialEditor::on_render(GUI::IContext* context, const GUI::LayoutConfig& layout)
    {
        if(!m_open) return;

        context->push_data_scope(context->make_id((GUI::id_t)(usize)this));
        GUI::ElementHandle root = EditorGUI::begin_v_layout(context, context->make_id("material_editor"), "Material Editor", layout);

        Ref<Material> mat = get_asset_or_async_load_if_not_ready<Material>(m_material);
        if(!mat || (Asset::get_asset_state(m_material) != Asset::AssetState::loaded))
        {
            EditorGUI::text(context, context->make_id("not_loaded"), "Material Asset is not loaded.", fixed_height(24.0f));
        }
        else
        {
            GUI::ElementHandle save_button = EditorGUI::text_button(context, context->make_id("save"), "Save", fixed_height(30.0f));
            if(EditorGUI::is_item_clicked(context, save_button))
            {
                lutry
                {
                    luexp(Asset::save_asset(m_material));
                }
                lucatch
                {
                    auto _ = Window::message_box(explain(luerr), "Failed to save asset", {"OK"},
                        Window::MessageBoxIcon::error);
                }
            }

            i32 material_type = (i32)mat->material_type;
            const c8* material_types[] = {"lit", "unlit"};
            EditorGUI::combo(context, context->make_id("material_type"), "Material Type", &material_type,
                Span<const c8*>(material_types, 2), fixed_height(30.0f));
            mat->material_type = (MeterialType)material_type;
            if(mat->material_type == MeterialType::lit)
            {
                gui_edit_asset_path(context, "Base Color", mat->base_color, m_base_color_name);
                gui_edit_asset_path(context, "Roughness", mat->roughness, m_roughness_name);
                gui_edit_asset_path(context, "Normal", mat->normal, m_normal_name);
                gui_edit_asset_path(context, "Metallic", mat->metallic, m_metallic_name);
                gui_edit_asset_path(context, "Emissive", mat->emissive, m_emissive_name);
            }
            else
            {
                gui_edit_asset_path(context, "Emissive", mat->emissive, m_emissive_name);
            }
            EditorGUI::DragDesc drag_desc;
            drag_desc.speed = 0.01f;
            EditorGUI::drag_float(context, context->make_id("emissive_intensity"), &mat->emissive_intensity,
                0.0f, 20.0f, fixed_height(30.0f), drag_desc);
        }
        EditorGUI::end_v_layout(context, root, vertical_editor_layout());
        context->pop_data_scope();
    }

    static Ref<IAssetEditor> material_new_editor(object_t userdata, Asset::asset_t editing_asset)
    {
        auto edt = new_object<MaterialEditor>();
        edt->m_material = editing_asset;
        return edt;
    }
    void register_material_editor()
    {
        AssetEditorDesc desc;
        desc.new_editor = material_new_editor;
        g_env->register_asset_editor_type(get_material_asset_type(), desc);
    }
}
