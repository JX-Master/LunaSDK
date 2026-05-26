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
    bool gui_edit_asset_path(const c8* label, Asset::asset_t& asset, String& path_text, const c8* failure_title)
    {
        if(asset && path_text.empty())
        {
            path_text = Asset::get_asset_path(asset).encode();
        }

        GUI::PushID(label);
        GUI::GUILayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;
        GUI::BeginHLayout(label, row);
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fixed_width(112.0f));
        GUI::Text(label);
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
        GUI::InputText("Path", path_text);
        GUI::GUIItemHandle set_button = GUI::Button("Set");
        GUI::GUIItemHandle clear_button = GUI::Button("Clear");
        GUI::EndHLayout();

        bool edited = false;
        if(GUI::IsItemClicked(set_button))
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
        if(GUI::IsItemClicked(clear_button))
        {
            asset.reset();
            path_text.clear();
            edited = true;
        }
        GUI::PopID();
        return edited;
    }
}
