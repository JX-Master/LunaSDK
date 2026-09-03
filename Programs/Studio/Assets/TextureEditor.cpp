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
#include <Luna/EditorGUI/EditorGUI.hpp>

namespace Luna
{
    namespace
    {
        GUI::LayoutConfig texture_size_layout(RHI::ITexture* texture)
        {
            GUI::LayoutConfig layout;
            if(texture)
            {
                auto desc = texture->get_desc();
                layout.width.kind = GUI::SizeKind::fixed;
                layout.width.value = (f32)desc.width;
                layout.height.kind = GUI::SizeKind::fixed;
                layout.height.value = (f32)desc.height;
            }
            return layout;
        }

        GUI::LayoutConfig fixed_height_layout(f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }
    }

    void TextureEditor::on_render(GUI::IContext* context, const GUI::LayoutConfig& layout)
    {
        luassert(context);
        if(!m_open)
        {
            return;
        }
        context->push_data_scope(context->make_id((GUI::id_t)(usize)this));
        GUI::ElementHandle root = EditorGUI::begin_v_layout(context, context->make_id("texture_editor"), "Texture Editor", layout);
        Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(m_tex, Name());
        if(!tex)
        {
            EditorGUI::text(context, context->make_id("unavailable"), "Texture Unavailable.", fixed_height_layout(28.0f));
        }
        else
        {
            EditorGUI::image(context, context->make_id("texture"), tex.get(), texture_size_layout(tex.get()));
        }
        EditorGUI::end_v_layout(context, root, GUI::FlexLayoutDesc());
        context->pop_data_scope();
    }

    static R<GUI::paint_order_id_t> on_draw_tex_tile_gui(GUI::IContext* context, object_t userdata,
        Asset::asset_t asset, const RectF& draw_rect, GUI::paint_order_id_t paint_order_id)
    {
        auto state = Asset::get_asset_data_unit_state(asset, Name());
        if(succeeded(state) && state.get() == Asset::AssetDataUnitState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset, Name());
            if(tex)
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::image;
                command.rect = draw_rect;
                command.texture = tex.get();
                context->draw(command, paint_order_id);
                return paint_order_id;
            }
        }
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::text;
        command.rect = draw_rect;
        command.color = Float4U(1.0f);
        command.font_size = 16.0f;
        command.horizontal_alignment = VG::TextAlignment::center;
        command.vertical_alignment = VG::TextAlignment::center;
        command.text = "Texture";
        context->draw(command, paint_order_id);
        return paint_order_id;
    }
    static R<GUI::paint_order_id_t> on_draw_tex_tile_preview_gui(GUI::IContext* context, object_t userdata,
        Asset::asset_t asset, const RectF& relative_rect, GUI::paint_order_id_t paint_order_id)
    {
        auto state = Asset::get_asset_data_unit_state(asset, Name());
        if(succeeded(state) && state.get() == Asset::AssetDataUnitState::loaded)
        {
            Ref<RHI::ITexture> tex = get_asset_or_async_load_if_not_ready<RHI::ITexture>(asset, Name());
            if(tex)
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::image;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = relative_rect;
                command.color = Float4U(1.0f);
                command.texture = tex.get();
                context->draw(command, paint_order_id);
                return paint_order_id;
            }
        }
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::text;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = relative_rect;
        command.color = Float4U(1.0f);
        command.font_size = 16.0f;
        command.horizontal_alignment = VG::TextAlignment::center;
        command.vertical_alignment = VG::TextAlignment::center;
        command.text = "Texture";
        context->draw(command, paint_order_id);
        return paint_order_id;
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
        desc.on_draw_tile_gui = on_draw_tex_tile_gui;
        desc.on_draw_tile_preview_gui = on_draw_tex_tile_preview_gui;
        desc.new_editor = new_tex_editor;
        g_env->register_asset_editor_type(get_static_texture_asset_type(), desc);
    }
}
