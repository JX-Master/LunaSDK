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
#include "../StudioHeader.hpp"
#include "../StudioGUI.hpp"
#include <Luna/Window/MessageBox.hpp>
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

        virtual void on_render(GUI::IContext* context) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };

    void MaterialEditor::on_render(GUI::IContext* context)
    {
        char title[256];
        auto path = Asset::get_asset_path(m_material);
        if (!path.empty())
        {
            snprintf(title, 256, "Material Editor - %s###%d", path.encode().c_str(), (u32)(usize)this);
        }
        else
        {
            snprintf(title, 256, "Material Editor###%d", (u32)(usize)this);
        }
        if(!m_open) return;
        GUI::begin_window(context, title, &m_open, GUI::Size::fixed(720.0f, 520.0f));

        Ref<Material> mat = get_asset_or_async_load_if_not_ready<Material>(m_material);
        if (!mat || (Asset::get_asset_state(m_material) != Asset::AssetState::loaded))
        {
            GUI::text(context, "Material Asset is not loaded.");
        }
        else
        {
            if (GUI::is_item_clicked(GUI::button(context, "Save")))
            {
                lutry
                {
                    luexp(Asset::save_asset(m_material));
                }
                lucatch
                {
                    auto _ = Window::message_box(explain(luerr), "Failed to save asset", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                }
            }
            i32 material_type = (i32)mat->material_type;
            const c8* material_types[] = {"lit", "unlit"};
            GUI::combo(context, "Material Type", &material_type, Span<const c8*>(material_types, 2));
            mat->material_type = (MeterialType)material_type;
            if (mat->material_type == MeterialType::lit)
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
            GUI::drag_float(context, "Emissive Intensity", &mat->emissive_intensity, 0.01f, 0.0f, 20.0f);
        }
        GUI::end_window(context);
    }
    static Ref<IAssetEditor> material_new_editor(object_t userdata, Asset::asset_t editing_asset)
    {
        auto edt = new_object<MaterialEditor>();
        edt->m_material = editing_asset;
        return edt;
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
}
