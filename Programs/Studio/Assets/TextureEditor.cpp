/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureEditor.cpp
* @author JXMaster
* @date 2020/5/9
*/
#include "Texture.hpp"

namespace Luna
{
    struct TextureEditor : public IAssetEditor
    {
        lustruct("TextureEditor", "{E1F83CDB-D75C-4943-9428-AB1768C94677}");
        luiimpl();

        Asset::asset_t m_tex;

        bool m_open = true;

        TextureEditor() {}

        virtual void on_render() override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };

    void TextureEditor::on_render()
    {
        Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(m_tex);
        if (!tex)
        {
            m_open = false;
            return;
        }

        char name[32];
        snprintf(name, 32, "Texture###%d", (u32)(usize)this);

        if(!m_open) return;

        lutry
        {
            auto desc = tex->get_desc();
            GUI::BeginWindow(name, &m_open, GUI::GUISize::fixed(max((f32)desc.width + 16.0f, 220.0f), max((f32)desc.height + 46.0f, 120.0f)));
            GUI::Image(tex.get(), GUI::GUISize::fixed((f32)desc.width, (f32)desc.height));
            GUI::EndWindow();
        }
        lucatch
        {
            GUI::BeginWindow(name, &m_open, GUI::GUISize::fixed(260.0f, 120.0f));
            GUI::Text("Texture Unavailable.");
            GUI::EndWindow();
        }
    }
    static void on_draw_tex_tile(object_t userdata, Asset::asset_t asset, const RectF& draw_rect)
    {
        if (Asset::get_asset_state(asset) == Asset::AssetState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset);
            if (tex)
            {
                GUI::DrawImage(tex.get(), draw_rect);
            }
        }
        else
        {
            GUI::DrawText(draw_rect, "Texture", Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
        }
    }
    static Ref<IAssetEditor> new_tex_editor(object_t userdata, Asset::asset_t editing_asset)
    {
        auto edit = new_object<TextureEditor>();
        edit->m_tex = editing_asset;
        return edit;
    }
    void register_texture_editor()
    {
        register_boxed_type<TextureEditor>();
        impl_interface_for_type<TextureEditor, IAssetEditor>();
        AssetEditorDesc desc;
        desc.on_draw_tile = on_draw_tex_tile;
        desc.new_editor = new_tex_editor;
        g_env->register_asset_editor_type(get_static_texture_asset_type(), desc);
    }
}
