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
#include <Luna/GUI/GUI.hpp>

namespace Luna
{
    namespace
    {
        GUICore::LayoutConfig texture_size_layout(RHI::ITexture* texture)
        {
            GUICore::LayoutConfig layout;
            if(texture)
            {
                auto desc = texture->get_desc();
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = (f32)desc.width;
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = (f32)desc.height;
            }
            return layout;
        }

        GUICore::LayoutConfig fixed_height_layout(f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }
    }

    void TextureEditor::on_render(GUICore::IContext* context, const GUICore::LayoutConfig& layout)
    {
        luassert(context);
        if(!m_open)
        {
            return;
        }
        context->push_data_scope(context->make_id((GUICore::id_t)(usize)this));
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("texture_editor"), "Texture Editor", layout);
        Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(m_tex);
        if(!tex)
        {
            GUI::text(context, context->make_id("unavailable"), "Texture Unavailable.", fixed_height_layout(28.0f));
        }
        else
        {
            GUI::image(context, context->make_id("texture"), tex.get(), texture_size_layout(tex.get()));
        }
        GUI::end_v_layout(context, root, GUICore::FlexLayoutDesc());
        context->pop_data_scope();
    }

    static void on_draw_tex_tile_core(GUICore::IContext* context, object_t userdata, Asset::asset_t asset, const RectF& draw_rect)
    {
        if(Asset::get_asset_state(asset) == Asset::AssetState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset);
            if(tex)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::image;
                command.rect = draw_rect;
                command.texture = tex.get();
                context->draw(command);
                return;
            }
        }
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::text;
        command.rect = draw_rect;
        command.color = Float4U(1.0f);
        command.font_size = 16.0f;
        command.horizontal_alignment = VG::TextAlignment::center;
        command.vertical_alignment = VG::TextAlignment::center;
        command.text = "Texture";
        context->draw(command);
    }
    static void on_draw_tex_tile_preview_core(GUICore::IContext* context, object_t userdata, Asset::asset_t asset,
        const RectF& relative_rect)
    {
        if(Asset::get_asset_state(asset) == Asset::AssetState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset);
            if(tex)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::image;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = relative_rect;
                command.color = Float4U(1.0f);
                command.texture = tex.get();
                context->draw(command);
                return;
            }
        }
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::text;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = relative_rect;
        command.color = Float4U(1.0f);
        command.font_size = 16.0f;
        command.horizontal_alignment = VG::TextAlignment::center;
        command.vertical_alignment = VG::TextAlignment::center;
        command.text = "Texture";
        context->draw(command);
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
        desc.on_draw_tile_core = on_draw_tex_tile_core;
        desc.on_draw_tile_preview_core = on_draw_tex_tile_preview_core;
        desc.new_editor = new_tex_editor;
        g_env->register_asset_editor_type(get_static_texture_asset_type(), desc);
    }
}
