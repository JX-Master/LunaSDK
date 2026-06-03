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
#include <Luna/Window/MessageBox.hpp>

namespace Luna
{
    bool gui_edit_asset_path(GUI::IContext* context, const c8* label, Asset::asset_t& asset, String& path_text, const c8* failure_title)
    {
        if(asset && path_text.empty())
        {
            path_text = Asset::get_asset_path(asset).encode();
        }

        GUI::push_id(context, label);
        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;
        GUI::begin_h_layout(context, label, row);
        GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(112.0f));
        GUI::text(context, label);
        GUI::set_next_item_layout(context, GUI::LayoutStyle::fill_width());
        GUI::input_text(context, "Path", path_text);
        GUI::ItemHandle set_button = GUI::button(context, "Set");
        GUI::ItemHandle clear_button = GUI::button(context, "Clear");
        GUI::end_h_layout(context);

        bool edited = false;
        if(GUI::is_item_clicked(set_button))
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
                    auto _ = Window::message_box(explain(r.errcode()), failure_title, Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                }
            }
        }
        if(GUI::is_item_clicked(clear_button))
        {
            asset.reset();
            path_text.clear();
            edited = true;
        }
        GUI::pop_id(context);
        return edited;
    }
}
