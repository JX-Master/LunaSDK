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
#include "TextureEditor.hpp"

namespace Luna
{
    void TextureEditor::on_render(GUI::IContext* context)
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
            GUI::begin_window(context, name, &m_open, GUI::Size::fixed(max((f32)desc.width + 16.0f, 220.0f), max((f32)desc.height + 46.0f, 120.0f)));
            GUI::image(context, tex.get(), GUI::Size::fixed((f32)desc.width, (f32)desc.height));
            GUI::end_window(context);
        }
        lucatch
        {
            GUI::begin_window(context, name, &m_open, GUI::Size::fixed(260.0f, 120.0f));
            GUI::text(context, "Texture Unavailable.");
            GUI::end_window(context);
        }
    }
    static void on_draw_tex_tile(GUI::IContext* context, object_t userdata, Asset::asset_t asset, const RectF& draw_rect)
    {
        if (Asset::get_asset_state(asset) == Asset::AssetState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset);
            if (tex)
            {
                GUI::draw_image(context, tex.get(), draw_rect);
            }
        }
        else
        {
            GUI::draw_text(context, draw_rect, "Texture", Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
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
        AssetEditorDesc desc;
        desc.on_draw_tile = on_draw_tex_tile;
        desc.new_editor = new_tex_editor;
        g_env->register_asset_editor_type(get_static_texture_asset_type(), desc);
    }
}
