/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AssetBrowser.cpp
* @author JXMaster
* @date 2020/4/29
*/
#include "AssetBrowser.hpp"
#include "MainEditor.hpp"
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/JobSystem/JobSystem.hpp>

namespace Luna
{
    struct AssetThumbnail
    {
        Name m_filename;    // Without extension.
        bool m_is_dir;
    };

    R<Vector<AssetThumbnail>> get_assets_in_folder(const Path& folder_path)
    {
        Vector<AssetThumbnail> assets;
        lutry
        {
            lulet(iter, VFS::open_dir(folder_path));
            while (iter->is_valid())
            {
                const c8* name = iter->get_filename();
                if(strcmp(name, ".") && strcmp(name, ".."))
                {
                    if ((iter->get_attributes() & FileAttributeFlag::directory) != FileAttributeFlag::none)
                    {
                        AssetThumbnail t;
                        t.m_filename = Name(iter->get_filename());
                        t.m_is_dir = true;
                        assets.push_back(t);
                    }
                    else
                    {
                        // Ends with ".meta.la" or ".meta.lb"
                        const char* name = iter->get_filename();
                        usize name_len = strlen(name);
                        if (name_len > 5)
                        {
                            if (!strcmp(name + name_len - 5, ".meta"))
                            {
                                AssetThumbnail t;
                                t.m_filename = Name(name, name_len - 5);
                                t.m_is_dir = false;
                                assets.push_back(t);
                            }
                        }
                    }
                }
                iter->move_next();
            }
        }
        lucatchret;
        return assets;
    }

    void AssetBrowser::change_path(const Path& path)
    {
        if (m_current_location_in_histroy_path != m_histroy_paths.size() - 1)
        {
            // Clear forwards.
            m_histroy_paths.resize(m_current_location_in_histroy_path + 1);
        }
        m_path.assign(path);
        m_histroy_paths.push_back(m_path);
        ++m_current_location_in_histroy_path;
    }

    inline Path get_new_asset_path(const Path& dir_path)
    {
        c8 buf[32];
        memcpy(buf, "Untitled", 9);
        Path path = dir_path;
        path.push_back(buf);
        auto asset = Asset::get_asset_by_path(path);
        if(succeeded(asset))
        {
            u32 index = 1;
            while(succeeded(asset))
            {
                path.pop_back();
                snprintf(buf, 32, "Untitled%u", index);
                path.push_back(buf);
                asset = Asset::get_asset_by_path(path);
            }
        }
        return path;
    }

    static bool gui_nav_button(const c8* label, bool disabled)
    {
        constexpr f32 button_size = 24.0f;
        Float2 pos = ImGui::GetCursorScreenPos();
        RectF rect(pos.x, pos.y, button_size, button_size);
        GUI::GUIItemHandle handle;
        if(disabled)
        {
            GUI::DrawRect(rect, Float4U(0.12f, 0.14f, 0.17f, 0.65f), 4.0f);
            GUI::DrawText(rect, label, Float4U(0.65f, 0.68f, 0.72f, 0.65f), 15.0f,
                GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
        }
        else
        {
            handle = GUI::Button(label, rect);
        }
        ImGui::Dummy(Float2(button_size, button_size));
        return !disabled && GUI::IsItemClicked(handle);
    }

    inline Path get_new_folder_path(const Path& dir_path)
    {
        c8 buf[64];
        memcpy(buf, "Untitled Folder", 9);
        Path path = dir_path;
        path.push_back(buf);
        auto attr = VFS::get_file_attribute(path);
        if(succeeded(attr))
        {
            u32 index = 1;
            while(succeeded(attr))
            {
                path.pop_back();
                snprintf(buf, 64, "Untitled Folder%u", index);
                path.push_back(buf);
                attr = VFS::get_file_attribute(path);
            }
        }
        return path;
    }

    namespace
    {
        void create_new_asset_in_folder(const Path& folder, const Name& type, Name& editing_asset_name, String& editing_buf)
        {
            Path new_asset_path = get_new_asset_path(folder);
            auto asset = Asset::new_asset(new_asset_path, type);
            if(succeeded(asset))
            {
                if(succeeded(Asset::load_asset_default_data(asset.get())))
                {
                    auto _ = Asset::save_asset(asset.get());
                }
                editing_buf = new_asset_path.back().c_str();
                editing_asset_name = new_asset_path.back();
            }
        }
    }

    void AssetBrowser::render()
    {
        for (auto& asset : m_deleting_assets)
        {
            auto r = Asset::delete_asset(asset);
            if (failed(r))
            {
                auto _ = Window::message_box(explain(r.errcode()), "Delete asset failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
            }
        }
        m_deleting_assets.clear();

        char title[64];
        snprintf(title, 64, "Asset Browser##%llu", (u64)this);

        ImGui::SetNextWindowSize({ 1000.0f, 500.0f }, ImGuiCond_FirstUseEver);
        ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse);
        m_host_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        Float2 host_min = ImGui::GetCursorScreenPos();
        Float2 host_size = ImGui::GetContentRegionAvail();
        RectF host_rect(host_min.x, host_min.y, max(host_size.x, 1.0f), max(host_size.y, 1.0f));
        GUI::PushClipRect(host_rect);
        GUI::DrawRect(host_rect, Float4U(0.06f, 0.07f, 0.08f, 1.0f), 0.0f);

        Float2 menu_pos = ImGui::GetCursorScreenPos();
        GUI::BeginMenuBar("Asset Browser Menu Bar", RectF(menu_pos.x, menu_pos.y, 146.0f, 30.0f));
        GUI::BeginMenu("New");
        GUI::GUIItemHandle folder_item = GUI::MenuItem("Folder");
        Vector<Pair<Name, GUI::GUIItemHandle>> asset_items;
        asset_items.reserve(g_env->new_asset_types.size());
        for(auto& i : g_env->new_asset_types)
        {
            asset_items.push_back(make_pair(i, GUI::MenuItem(i.c_str())));
        }
        GUI::EndMenu();
        GUI::BeginMenu("Import");
        Vector<Pair<Name, GUI::GUIItemHandle>> import_items;
        import_items.reserve(g_env->importer_types.size());
        for(auto& i : g_env->importer_types)
        {
            import_items.push_back(make_pair(i.first, GUI::MenuItem(i.first.c_str())));
        }
        GUI::EndMenu();
        GUI::EndMenuBar();
        ImGui::Dummy(Float2(146.0f, 30.0f));

        if(GUI::IsItemClicked(folder_item))
        {
            Path new_folder_path = get_new_folder_path(m_path);
            auto r = VFS::create_dir(new_folder_path);
            if(succeeded(r))
            {
                m_asset_name_editing_buf = new_folder_path.back().c_str();
                m_editing_asset_name = new_folder_path.back();
            }
        }
        for(auto& item : asset_items)
        {
            if(GUI::IsItemClicked(item.second))
            {
                create_new_asset_in_folder(m_path, item.first, m_editing_asset_name, m_asset_name_editing_buf);
            }
        }
        for(auto& item : import_items)
        {
            if(GUI::IsItemClicked(item.second))
            {
                auto iter = g_env->importer_types.find(item.first);
                if(iter != g_env->importer_types.end())
                {
                    auto editor = iter->second.new_importer(m_path);
                    m_editor->m_editors.push_back(editor);
                }
            }
        }

        navbar();

        tile_context();

        GUI::PopClipRect();
        ImGui::End();
    }
    void AssetBrowser::navbar()
    {
        // Draw back/forward/pop arrow.
        bool back_disabled = (m_current_location_in_histroy_path == 0);
        if (gui_nav_button("<", back_disabled))
        {
            --m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }
        ImGui::SameLine();
        bool forward_disabled = (m_current_location_in_histroy_path == m_histroy_paths.size() - 1);
        if (gui_nav_button(">", forward_disabled))
        {
            ++m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }
        ImGui::SameLine();
        bool pop_disabled = m_path.empty();
        if (gui_nav_button("^", pop_disabled))
        {
            auto path = m_path;
            path.pop_back();
            change_path(path);
        }
        ImGui::SameLine();
        // Draw path.
        {
            Float2 pos = ImGui::GetCursorScreenPos();
            Float2 frame_padding = ImGui::GetStyle().FramePadding;

            Float2 region_min = pos;
            Float2 region_max = pos + frame_padding * 2 + Float2(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x,
                ImGui::GetTextLineHeight());
            if (!m_is_navbar_text_editing)
            {
                RectF navbar_rect(region_min.x, region_min.y, region_max.x - region_min.x, region_max.y - region_min.y);
                Float4U navbar_border(0.25f, 0.28f, 0.32f, 1.0f);
                GUI::GUIItemHandle path_region = GUI::HitBox("Asset Browser Path Region", navbar_rect);
                GUI::DrawRect(navbar_rect, Float4U(0.125f, 0.125f, 0.125f, 1.0f), 0.0f);
                GUI::DrawRect(RectF(navbar_rect.offset_x, navbar_rect.offset_y, navbar_rect.width, 1.0f), navbar_border);
                GUI::DrawRect(RectF(navbar_rect.offset_x, navbar_rect.offset_y + navbar_rect.height - 1.0f, navbar_rect.width, 1.0f), navbar_border);
                GUI::DrawRect(RectF(navbar_rect.offset_x, navbar_rect.offset_y, 1.0f, navbar_rect.height), navbar_border);
                GUI::DrawRect(RectF(navbar_rect.offset_x + navbar_rect.width - 1.0f, navbar_rect.offset_y, 1.0f, navbar_rect.height), navbar_border);

                f32 cursor_x = region_min.x + frame_padding.x;
                f32 text_y = region_min.y;
                f32 text_h = region_max.y - region_min.y;
                const Float4U path_text_color(0.92f, 0.94f, 0.96f, 1.0f);
                if ((m_path.flags() & PathFlag::absolute) != PathFlag::none)
                {
                    GUI::DrawText(RectF(cursor_x, text_y, 10.0f, text_h), "/", path_text_color, 16.0f,
                        GUI::GUITextAlignment::begin, GUI::GUITextAlignment::center);
                    cursor_x += 10.0f;
                }
                Path changed_path;
                for (u32 i = 0; i < m_path.size(); ++i)
                {
                    auto node = m_path[i];
                    Float2 text_size = ImGui::CalcTextSize(node.c_str());
                    f32 node_width = max(text_size.x + 12.0f, 20.0f);
                    RectF node_rect(cursor_x, region_min.y + 1.0f, node_width, max(text_h - 2.0f, 1.0f));
                    GUI::PushID(i);
                    GUI::GUIItemHandle node_hit = GUI::HitBox(node.c_str(), node_rect);
                    GUI::DrawText(RectF(node_rect.offset_x + 6.0f, node_rect.offset_y, max(node_rect.width - 12.0f, 1.0f), node_rect.height),
                        node.c_str(), path_text_color, 16.0f, GUI::GUITextAlignment::begin, GUI::GUITextAlignment::center);
                    GUI::PopID();
                    if (GUI::IsItemClicked(node_hit) && i != (m_path.size() - 1))
                    {
                        changed_path = m_path;
                        for (u32 j = i; j < m_path.size() - 1; ++j)
                        {
                            changed_path.pop_back();
                        }
                    }
                    cursor_x += node_width;
                    GUI::DrawText(RectF(cursor_x, text_y, 10.0f, text_h), "/", path_text_color, 16.0f,
                        GUI::GUITextAlignment::begin, GUI::GUITextAlignment::center);
                    cursor_x += 10.0f;
                }
                if (!changed_path.empty())
                {
                    change_path(changed_path);
                }

                if (GUI::IsItemClicked(path_region))
                {
                    // Switch to text mode.
                    m_is_navbar_text_editing = true;
                    m_path_edit_text = m_path.encode(PathSeparator::slash, true);
                }
                ImGui::Dummy(region_max - region_min);
            }
            else
            {
                GUI::GUILayoutDesc edit_row;
                edit_row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::stretch;
                GUI::BeginHLayout("Path Text Editing",
                    RectF(region_min.x, region_min.y, region_max.x - region_min.x, region_max.y - region_min.y), edit_row);
                GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
                GUI::InputText("PathTextEditing", m_path_edit_text);
                GUI::EndHLayout();
                ImGui::Dummy(region_max - region_min);
                auto mouse_pos = ImGui::GetIO().MousePos;
                if (!in_bounds(mouse_pos, region_min, region_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    // Switch to normal mode.
                    m_is_navbar_text_editing = false;
                    auto new_p = Path(m_path_edit_text.c_str());
                    auto attr = VFS::get_file_attribute(new_p);
                    if (succeeded(attr) && ((attr.get().attributes & FileAttributeFlag::directory) != FileAttributeFlag::none))
                    {
                        m_path.assign(new_p);
                    }
                }
            }
        }

    }
    static RV remove_assets_in_folder(const Path& dir)
    {
        lutry
        {
            lulet(assets, get_assets_in_folder(dir));
            Path subpath = dir;
            for(auto& asset : assets)
            {
                subpath.push_back(asset.m_filename);
                if(asset.m_is_dir)
                {
                    luexp(remove_assets_in_folder(subpath));
                }
                else
                {
                    auto a = Asset::get_asset_by_path(subpath);
                    if(succeeded(a))
                    {
                        luexp(Asset::delete_asset(a.get()));
                    }
                }
                subpath.pop_back();
            }
        }
        lucatchret;
        return ok;
    }
    static RV remove_dir(Path& dir)
    {
        lutry
        {
            // Remove all files in directory.
            Vector<Name> files;
            lulet(iter, VFS::open_dir(dir));
            for(;iter->is_valid(); iter->move_next())
            {
                const c8* filename = iter->get_filename();
                if(!strcmp(filename, ".") || !strcmp(filename, "..")) continue;
                files.push_back(filename);
            }
            iter.reset();
            for(auto& f : files)
            {
                dir.push_back(f);
                lulet(attr, VFS::get_file_attribute(dir));
                if(test_flags(attr.attributes, FileAttributeFlag::directory))
                {
                    luexp(remove_dir(dir));
                }
                else
                {
                    luexp(VFS::delete_file(dir));
                }
                dir.pop_back();
            }
            // Delete empty directory.
            luexp(VFS::delete_file(dir));
        }
        lucatchret;
        return ok;
    }
    void AssetBrowser::tile_context()
    {
        // Draw content.
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("ctx", Float2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
        Float2 child_min = ImGui::GetWindowPos();
        Float2 child_size = ImGui::GetWindowSize();
        GUI::PushClipRect(RectF(child_min.x, child_min.y, max(child_size.x, 1.0f), max(child_size.y, 1.0f)));
        auto assets = get_assets_in_folder(m_path);
        bool tile_context_focused = ImGui::IsWindowFocused();
        if (succeeded(assets))
        {
            if (assets.get().empty())
            {
                auto region = ImGui::GetContentRegionAvail();
                auto origin = ImGui::GetCursorScreenPos();
                const char* text = "Empty Directory";
                GUI::DrawText(RectF(origin.x, origin.y, region.x, region.y), text, Float4U(1.0f), 16.0f,
                    GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
            }
            else
            {
                // Draw asset tiles.

                usize num_assets = assets.get().size();

                constexpr u32 padding = 5;

                u32 tile_width = (u32)(m_tile_size + padding * 2);
                u32 tile_height = (u32)(m_tile_size + padding * 2 + ImGui::GetTextLineHeight());
                auto window_pos = ImGui::GetWindowPos();

                f32 woff = 0;
                f32 hoff = 0;
                auto origin_pos = ImGui::GetCursorPos();

                for (usize i = 0; i < num_assets; ++i)
                {
                    // Set cursor pos for next tile.
                    ImGui::SetCursorPos(origin_pos + Float2(woff, hoff));

                    auto tile_min = ImGui::GetCursorScreenPos() + padding;
                    auto tile_max = tile_min + Float2((f32)tile_width, (f32)tile_height);
                    RectF tile_rect(tile_min.x - padding, tile_min.y - padding, tile_max.x - tile_min.x, tile_max.y - tile_min.y);

                    GUI::PushID(assets.get()[i].m_filename.c_str());
                    GUI::GUIItemHandle tile_hit = GUI::HitBox("Asset Tile", tile_rect);
                    GUI::PopID();
                    bool tile_clicked = tile_context_focused && GUI::IsItemClicked(tile_hit);
                    bool tile_right_clicked = tile_context_focused && GUI::IsItemRightClicked(tile_hit);
                    bool tile_double_clicked = tile_context_focused && GUI::IsItemDoubleClicked(tile_hit);
                    if(tile_clicked || tile_right_clicked)
                    {
                        m_selections.clear();
                        m_selections.insert(assets.get()[i].m_filename);
                    }
                    if(tile_right_clicked)
                    {
                        m_popup_asset = assets.get()[i].m_filename;
                        m_asset_popup_open = true;
                        m_asset_popup_position = GUI::GetPointerPosition();
                    }

                    auto siter = m_selections.find(assets.get()[i].m_filename);
                    if (siter != m_selections.end())
                    {
                        GUI::DrawRect(tile_rect, Float4U(0.18f, 0.28f, 0.45f, 0.90f), 5.0f);
                    }

                    if (assets.get()[i].m_is_dir)
                    {
                        auto folder_icon_begin_pos = ImGui::GetCursorScreenPos() + Float2(padding, padding);
                        Float4U folder_color(0.78f, 0.78f, 0.78f, 1.0f);
                        GUI::DrawRect(RectF(
                            folder_icon_begin_pos.x + m_tile_size * 0.18f,
                            folder_icon_begin_pos.y + m_tile_size * 0.18f,
                            m_tile_size * 0.44f,
                            m_tile_size * 0.18f),
                            folder_color, 5.0f);
                        GUI::DrawRect(RectF(
                            folder_icon_begin_pos.x + m_tile_size * 0.08f,
                            folder_icon_begin_pos.y + m_tile_size * 0.30f,
                            m_tile_size * 0.84f,
                            m_tile_size * 0.56f),
                            folder_color, 7.0f);

                        if (tile_double_clicked)
                        {
                            // Change path.
                            auto path = m_path;
                            path.push_back(assets.get()[i].m_filename);
                            change_path(path);
                        }
                    }
                    else
                    {
                        auto meta_path = m_path;
                        meta_path.push_back(assets.get()[i].m_filename);
                        auto asset = Asset::get_asset_by_path(meta_path);
                        if (succeeded(asset))
                        {
                            auto draw_rect = RectF(tile_min.x, tile_min.y, m_tile_size, m_tile_size);

                            ImGui::SetCursorScreenPos({ draw_rect.offset_x, draw_rect.offset_y });
                            ImGui::PushID(asset.get().handle);
                            ImGui::Button("", { draw_rect.width, draw_rect.height });
                            ImGui::PopID();

                            if (ImGui::BeginDragDropSource())
                            {
                                Asset::asset_t payload = asset.get();
                                ImGui::SetDragDropPayload("Asset Ref", &payload, sizeof(payload));
                                ImGui::Text("%s", meta_path.encode().c_str());
                                ImGui::EndDragDropSource();
                            }

                            // Editor logic.
                            auto asset_type = Asset::get_asset_type(asset.get());
                            auto iter = g_env->editor_types.find(asset_type);

                            if (iter != g_env->editor_types.end())
                            {
                                if (iter->second.on_draw_tile)
                                {
                                    iter->second.on_draw_tile(iter->second.userdata.get(), asset.get(), draw_rect);
                                }
                                else
                                {
                                    GUI::DrawText(draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
                                }

                                if (tile_double_clicked)
                                {
                                    // Open Editor.
                                    auto edit = iter->second.new_editor(iter->second.userdata.get(), asset.get());
                                    m_editor->m_editors.push_back(edit);
                                }
                            }
                            else
                            {
                                GUI::DrawText(draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
                            }

                            // Load the data if not loaded.
                            if (Asset::get_asset_state(asset.get()) == Asset::AssetState::unloaded)
                            {
                                async_load_asset(asset.get());
                            }

                            // Draw status circle.
                            if (Asset::get_asset_state(asset.get()) == Asset::AssetState::loaded)
                            {
                                GUI::DrawCircle(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::green());
                            }
                            else
                            {
                                GUI::DrawCircle(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::yellow());
                            }
                        }
                        else
                        {
                            RectF draw_rect(tile_min.x, tile_min.y, m_tile_size, m_tile_size);
                            GUI::DrawText(draw_rect, "Unknown", Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
                            GUI::DrawCircle(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::red());
                        }
                    }

                    // Draw asset name.
                    ImGui::SetCursorScreenPos(Float2(tile_min.x, tile_min.y + m_tile_size));
                    if(assets.get()[i].m_filename == m_editing_asset_name)
                    {
                        RectF edit_rect(tile_min.x, tile_min.y + m_tile_size, m_tile_size, max(ImGui::GetTextLineHeight() + 6.0f, 20.0f));
                        GUI::GUILayoutDesc edit_row;
                        edit_row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::stretch;
                        GUI::BeginHLayout("AssetNameEdit", edit_rect, edit_row);
                        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
                        GUI::InputText("AssetNameEdit", m_asset_name_editing_buf);
                        GUI::EndHLayout();
                        ImGui::Dummy(Float2(m_tile_size, ImGui::GetTextLineHeight()));

                        Float2 edit_min(edit_rect.offset_x, edit_rect.offset_y);
                        Float2 edit_max(edit_rect.offset_x + edit_rect.width, edit_rect.offset_y + edit_rect.height);
                        if (!in_bounds(ImGui::GetIO().MousePos, edit_min, edit_max) &&
                            (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyDown(ImGuiKey_Enter)))
                        {
                            bool valid_filename = true;
                            for(c8 ch : m_asset_name_editing_buf)
                            {
                                if(ch == '\\' || ch == '/' || ch == ':' || ch == '*' || ch == '?' || ch == '\"' || ch == '<' ||
                                    ch == '>' || ch == '|')
                                {
                                    auto _ = Window::message_box("File or directory name cannot contain the following characters: \\ / : * ? \" < > |", "Rename directory failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                                    valid_filename = false;
                                    break;
                                }
                            }
                            if(valid_filename && assets.get()[i].m_filename != m_asset_name_editing_buf)
                            {
                                Path from_path = m_path;
                                Path to_path = m_path;
                                from_path.push_back(assets.get()[i].m_filename);
                                to_path.push_back(m_asset_name_editing_buf);
                                if(assets.get()[i].m_is_dir)
                                {
                                    auto r = VFS::move_file(from_path, to_path);
                                    if(succeeded(r))
                                    {
                                        r = Asset::load_assets_meta(to_path);
                                    }
                                    if(failed(r))
                                    {
                                        auto _ = Window::message_box(explain(r.errcode()), "Rename directory failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                                    }
                                }
                                else
                                {
                                    auto asset = Asset::get_asset_by_path(from_path);
                                    if(succeeded(asset))
                                    {
                                        auto r = Asset::move_asset(asset.get(), to_path);
                                        if(failed(r))
                                        {
                                            auto _ = Window::message_box(explain(r.errcode()), "Rename asset failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                                        }
                                    }
                                }
                            }
                            m_editing_asset_name.reset();
                        }
                    }
                    else
                    {
                        auto& filename = assets.get()[i].m_filename;
                        constexpr usize CLAMP_LEN = 12;
                        const c8* display_name = filename.c_str();
                        Name clipped_name;
                        if(filename.size() > CLAMP_LEN)
                        {
                            constexpr usize DISPLAY_LEN = CLAMP_LEN - 1;
                            c8 buf[DISPLAY_LEN];
                            usize sz = 0;
                            const c8* cur = filename.c_str();
                            while(true)
                            {
                                usize next_char_sz = utf8_charlen(cur);
                                if(next_char_sz + sz >= DISPLAY_LEN)
                                {
                                    break;
                                }
                                sz += next_char_sz;
                                cur += next_char_sz;
                            }
                            clipped_name = Name(filename.c_str(), sz);
                            display_name = clipped_name.c_str();
                        }
                        String display_text;
                        if(filename.size() > CLAMP_LEN)
                        {
                            strprintf(display_text, "%s...", display_name);
                        }
                        else
                        {
                            display_text = display_name;
                        }
                        GUI::DrawText(RectF(tile_min.x, tile_min.y + m_tile_size, m_tile_size, ImGui::GetTextLineHeight()),
                            display_text.c_str(), Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
                        ImGui::Dummy(Float2(m_tile_size, ImGui::GetTextLineHeight()));
                    }

                    // Update woff and hoff.
                    woff += tile_width;
                    if (woff + tile_width > ImGui::GetWindowWidth())
                    {
                        woff = 0;
                        hoff += tile_height;
                    }
                }

                if(m_asset_popup_open)
                {
                    constexpr f32 popup_width = 160.0f;
                    constexpr f32 popup_height = 70.0f;
                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                        !in_bounds(ImGui::GetIO().MousePos, m_asset_popup_position, m_asset_popup_position + Float2U(popup_width, popup_height)))
                    {
                        m_asset_popup_open = false;
                    }
                    GUI::BeginPopup("Asset Popup", m_asset_popup_position, GUI::GUISize::fixed(popup_width, popup_height));
                    GUI::GUIItemHandle rename_item = GUI::Selectable("Rename");
                    GUI::GUIItemHandle delete_item = GUI::Selectable("Delete");
                    GUI::EndPopup();
                    if (GUI::IsItemClicked(rename_item))
                    {
                        m_editing_asset_name = m_popup_asset;
                        m_asset_name_editing_buf = m_popup_asset.c_str();
                        m_asset_popup_open = false;
                    }
                    if (GUI::IsItemClicked(delete_item))
                    {
                        Path path = m_path;
                        path.push_back(m_popup_asset);
                        auto attr = VFS::get_file_attribute(path);
                        if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
                        {
                            // Remove all assets in the folder.
                            auto r = remove_assets_in_folder(path);
                            if(failed(r))
                            {
                                auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                            }
                            r = remove_dir(path);
                            if(failed(r))
                            {
                                auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                            }
                        }
                        else
                        {
                            auto asset = Asset::get_asset_by_path(path);
                            if(failed(asset))
                            {
                                auto _ = Window::message_box(explain(asset.errcode()), "Delete asset failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                            }
                            else
                            {
                                m_deleting_assets.push_back(asset.get());
                            }
                        }
                        m_asset_popup_open = false;
                    }
                }
            }
        }
        else
        {
            auto region = ImGui::GetContentRegionAvail();
            auto origin = ImGui::GetCursorScreenPos();
            const char* text_fail = "Failed to display assets in this directory.";
            const char* text_reason = explain(assets.errcode());
            GUI::DrawText(RectF(origin.x, origin.y + region.y * 0.5f - 24.0f, region.x, 24.0f), text_fail,
                Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
            GUI::DrawText(RectF(origin.x, origin.y + region.y * 0.5f, region.x, 24.0f), text_reason,
                Float4U(1.0f), 16.0f, GUI::GUITextAlignment::center, GUI::GUITextAlignment::center);
        }

        GUI::PopClipRect();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
    struct AssetLoadTask
    {
        Asset::asset_t asset;
    };
    void async_load_asset_func(JobSystem::IJobScheduler* scheduler, void* params)
    {
        AssetLoadTask* task = (AssetLoadTask*)params;
        auto r = Asset::load_asset(task->asset);
        if(failed(r))
        {
            log_error("Studio", "Failed to load asset %s: %s", Asset::get_asset_path(task->asset).encode().c_str(), explain(r.errcode()));
        }
        memdelete(task);
    }
    void async_load_asset(Asset::asset_t asset)
    {
        AssetLoadTask* task = memnew<AssetLoadTask>();
        task->asset = asset;
        g_main_editor->m_job_scheduler->submit_job(async_load_asset_func, task);
    }
}
