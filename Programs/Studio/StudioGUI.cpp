/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StudioGUI.cpp
* @author JXMaster
* @date 2026/5/22
*/
#include "StudioGUI.hpp"
#include <Luna/GUI/GUI.hpp>
#include <Luna/Window/MessageBox.hpp>

namespace Luna
{
    namespace
    {
        GUICore::LayoutConfig fixed_size(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutConfig fill_width(f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        bool apply_asset_path_edit(const c8* label, Asset::asset_t& asset, String& path_text, const c8* failure_title,
            bool set_clicked, bool clear_clicked)
        {
            bool edited = false;
            if(set_clicked)
            {
                if(path_text.empty())
                {
                    asset.reset();
                    edited = true;
                }
                else
                {
                    Path path = path_text.c_str();
                    auto r = Asset::get_asset_by_path(path);
                    if(succeeded(r))
                    {
                        asset = r.get();
                        path_text = Asset::get_asset_path(asset).encode();
                        edited = true;
                    }
                    else
                    {
                        auto _ = Window::message_box(explain(r.errcode()), failure_title, Window::MessageBoxType::ok,
                            Window::MessageBoxIcon::error);
                    }
                }
            }
            if(clear_clicked)
            {
                asset.reset();
                path_text.clear();
                edited = true;
            }
            return edited;
        }
    }

    bool gui_edit_asset_path(GUICore::IContext* context, const c8* label, Asset::asset_t& asset, String& path_text, const c8* failure_title)
    {
        luassert(context && label);
        if(asset && path_text.empty())
        {
            path_text = Asset::get_asset_path(asset).encode();
        }

        GUICore::id_t scope = context->make_id(label);
        context->push_data_scope(scope);
        GUICore::LayoutConfig row_layout = fill_width(30.0f);
        GUICore::ElementHandle row = GUI::begin_h_layout(context, context->make_id("row"), label, row_layout);
        GUI::text(context, context->make_id("label"), label, fixed_size(112.0f, 30.0f));
        GUI::input_text(context, context->make_id("path"), path_text, fill_width(30.0f));
        GUICore::ElementHandle set_button = GUI::text_button(context, context->make_id("set"), "Set", fixed_size(52.0f, 30.0f));
        GUICore::ElementHandle clear_button = GUI::text_button(context, context->make_id("clear"), "Clear", fixed_size(64.0f, 30.0f));
        GUICore::FlexLayoutDesc row_desc;
        row_desc.axis = GUICore::LayoutAxis::x;
        row_desc.main_axis_gap = 8.0f;
        GUI::end_h_layout(context, row, row_desc);
        bool edited = apply_asset_path_edit(label, asset, path_text, failure_title,
            GUI::is_item_clicked(context, set_button), GUI::is_item_clicked(context, clear_button));
        context->pop_data_scope();
        return edited;
    }
}
