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

    static GUI::ItemHandle gui_nav_button_at(GUI::IContext* context, const c8* label, const RectF& rect, bool disabled)
    {
        if(disabled)
        {
            GUI::draw_rect(context, rect, Float4U(0.12f, 0.14f, 0.17f, 0.65f), 4.0f);
            GUI::draw_text(context, rect, label, Float4U(0.65f, 0.68f, 0.72f, 0.65f), 15.0f,
                GUI::TextAlignment::center, GUI::TextAlignment::center);
            return GUI::ItemHandle();
        }
        return GUI::text_button(context, label, rect);
    }

    static f32 estimate_gui_text_width(const c8* text)
    {
        return text ? (f32)strlen(text) * 8.0f : 0.0f;
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

    void AssetBrowser::render(GUI::IContext* context, bool* open)
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

        if(open && !*open)
        {
            return;
        }

        GUI::push_id(context, this);
        GUI::DockPanelStyle panel_style;
        panel_style.floating_size = Float2U(1000.0f, 500.0f);
        panel_style.min_floating_size = Float2U(320.0f, 220.0f);
        GUI::LayoutDesc panel_layout;
        panel_layout.padding = GUI::EdgeInsets::all(0.0f);
        panel_layout.gap = 0.0f;
        GUI::ItemHandle panel = GUI::begin_dock_panel(context, "Asset Browser", open, panel_style, panel_layout);
        m_host_focused = GUI::is_item_focused(panel) || GUI::is_item_hovered(panel) || GUI::is_item_active(panel);

        RectF host_rect = GUI::get_item_state(panel, GUI::State::rect());
        if(host_rect.width <= 1.0f || host_rect.height <= 1.0f)
        {
            GUI::text(context, "Asset Browser");
            GUI::end_dock_panel(context);
            GUI::pop_id(context);
            return;
        }
        GUI::push_clip_rect(context, host_rect);
        GUI::draw_rect(context, host_rect, Float4U(0.06f, 0.07f, 0.08f, 1.0f), 0.0f);

        constexpr f32 menu_height = 30.0f;
        constexpr f32 nav_height = 30.0f;
        constexpr f32 gap = 6.0f;
        RectF menu_rect(host_rect.offset_x, host_rect.offset_y, min(146.0f, host_rect.width), menu_height);
        GUI::begin_menu_bar(context, "Asset Browser Menu Bar", menu_rect);
        GUI::ItemHandle folder_item;
        Vector<Pair<Name, GUI::ItemHandle>> asset_items;
        if(GUI::begin_menu(context, "New"))
        {
            folder_item = GUI::menu_item(context, "Folder");
            asset_items.reserve(g_env->new_asset_types.size());
            for(auto& i : g_env->new_asset_types)
            {
                asset_items.push_back(make_pair(i, GUI::menu_item(context, i.c_str())));
            }
            GUI::end_menu(context);
        }
        Vector<Pair<Name, GUI::ItemHandle>> import_items;
        if(GUI::begin_menu(context, "Import"))
        {
            import_items.reserve(g_env->importer_types.size());
            for(auto& i : g_env->importer_types)
            {
                import_items.push_back(make_pair(i.first, GUI::menu_item(context, i.first.c_str())));
            }
            GUI::end_menu(context);
        }
        GUI::end_menu_bar(context);

        if(GUI::is_item_clicked(folder_item))
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
            if(GUI::is_item_clicked(item.second))
            {
                create_new_asset_in_folder(m_path, item.first, m_editing_asset_name, m_asset_name_editing_buf);
            }
        }
        for(auto& item : import_items)
        {
            if(GUI::is_item_clicked(item.second))
            {
                auto iter = g_env->importer_types.find(item.first);
                if(iter != g_env->importer_types.end())
                {
                    auto editor = iter->second.new_importer(m_path);
                    m_editor->m_editors.push_back(editor);
                }
            }
        }

        RectF navbar_rect(host_rect.offset_x, host_rect.offset_y + menu_height + gap, host_rect.width, nav_height);
        navbar(context, navbar_rect);

        RectF tile_rect(host_rect.offset_x, navbar_rect.offset_y + navbar_rect.height + gap, host_rect.width,
            max(host_rect.height - menu_height - nav_height - gap * 2.0f, 1.0f));
        tile_context(context, tile_rect);

        GUI::pop_clip_rect(context);
        GUI::end_dock_panel(context);
        GUI::pop_id(context);
    }
    void AssetBrowser::navbar(GUI::IContext* context, const RectF& rect)
    {
        constexpr f32 button_size = 24.0f;
        constexpr f32 gap = 6.0f;
        constexpr f32 row_padding_y = 3.0f;
        f32 row_width = max(rect.width, button_size * 3.0f + gap * 3.0f + 64.0f);
        f32 cursor_x = rect.offset_x;
        f32 button_y = rect.offset_y + row_padding_y;

        // Draw back/forward/pop arrow.
        bool back_disabled = (m_current_location_in_histroy_path == 0);
        GUI::ItemHandle back_button = gui_nav_button_at(context, "<", RectF(cursor_x, button_y, button_size, button_size), back_disabled);
        cursor_x += button_size + gap;
        if (GUI::is_item_clicked(back_button))
        {
            --m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }
        bool forward_disabled = (m_current_location_in_histroy_path == m_histroy_paths.size() - 1);
        GUI::ItemHandle forward_button = gui_nav_button_at(context, ">", RectF(cursor_x, button_y, button_size, button_size), forward_disabled);
        cursor_x += button_size + gap;
        if (GUI::is_item_clicked(forward_button))
        {
            ++m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }
        bool pop_disabled = m_path.empty();
        GUI::ItemHandle pop_button = gui_nav_button_at(context, "^", RectF(cursor_x, button_y, button_size, button_size), pop_disabled);
        cursor_x += button_size + gap;
        if (GUI::is_item_clicked(pop_button))
        {
            auto path = m_path;
            path.pop_back();
            change_path(path);
        }

        // Draw path.
        {
            Float2U frame_padding(8.0f, 3.0f);
            Float2 region_min(cursor_x, rect.offset_y + row_padding_y);
            Float2 region_max(rect.offset_x + row_width, rect.offset_y + row_padding_y + button_size);
            if (!m_is_navbar_text_editing)
            {
                RectF navbar_rect(region_min.x, region_min.y, region_max.x - region_min.x, region_max.y - region_min.y);
                Float4U navbar_border(0.25f, 0.28f, 0.32f, 1.0f);
                GUI::ItemHandle path_region = GUI::hit_box(context, "Asset Browser Path Region", navbar_rect);
                GUI::draw_rect(context, navbar_rect, Float4U(0.125f, 0.125f, 0.125f, 1.0f), 0.0f);
                GUI::draw_rect(context, RectF(navbar_rect.offset_x, navbar_rect.offset_y, navbar_rect.width, 1.0f), navbar_border);
                GUI::draw_rect(context, RectF(navbar_rect.offset_x, navbar_rect.offset_y + navbar_rect.height - 1.0f, navbar_rect.width, 1.0f), navbar_border);
                GUI::draw_rect(context, RectF(navbar_rect.offset_x, navbar_rect.offset_y, 1.0f, navbar_rect.height), navbar_border);
                GUI::draw_rect(context, RectF(navbar_rect.offset_x + navbar_rect.width - 1.0f, navbar_rect.offset_y, 1.0f, navbar_rect.height), navbar_border);

                f32 cursor_x = region_min.x + frame_padding.x;
                f32 text_y = region_min.y;
                f32 text_h = region_max.y - region_min.y;
                const Float4U path_text_color(0.92f, 0.94f, 0.96f, 1.0f);
                if ((m_path.flags() & PathFlag::absolute) != PathFlag::none)
                {
                    GUI::draw_text(context, RectF(cursor_x, text_y, 10.0f, text_h), "/", path_text_color, 16.0f,
                        GUI::TextAlignment::begin, GUI::TextAlignment::center);
                    cursor_x += 10.0f;
                }
                Path changed_path;
                for (u32 i = 0; i < m_path.size(); ++i)
                {
                    auto node = m_path[i];
                    f32 node_width = max(estimate_gui_text_width(node.c_str()) + 12.0f, 20.0f);
                    RectF node_rect(cursor_x, region_min.y + 1.0f, node_width, max(text_h - 2.0f, 1.0f));
                    GUI::push_id(context, i);
                    GUI::ItemHandle node_hit = GUI::hit_box(context, node.c_str(), node_rect);
                    GUI::draw_text(context, RectF(node_rect.offset_x + 6.0f, node_rect.offset_y, max(node_rect.width - 12.0f, 1.0f), node_rect.height),
                        node.c_str(), path_text_color, 16.0f, GUI::TextAlignment::begin, GUI::TextAlignment::center);
                    GUI::pop_id(context);
                    if (GUI::is_item_clicked(node_hit) && i != (m_path.size() - 1))
                    {
                        changed_path = m_path;
                        for (u32 j = i; j < m_path.size() - 1; ++j)
                        {
                            changed_path.pop_back();
                        }
                    }
                    cursor_x += node_width;
                    GUI::draw_text(context, RectF(cursor_x, text_y, 10.0f, text_h), "/", path_text_color, 16.0f,
                        GUI::TextAlignment::begin, GUI::TextAlignment::center);
                    cursor_x += 10.0f;
                }
                if (!changed_path.empty())
                {
                    change_path(changed_path);
                }

                if (GUI::is_item_clicked(path_region))
                {
                    // Switch to text mode.
                    m_is_navbar_text_editing = true;
                    m_path_edit_text = m_path.encode(PathSeparator::slash, true);
                }
            }
            else
            {
                GUI::LayoutDesc edit_row;
                edit_row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
                GUI::begin_h_layout(context, "Path Text Editing",
                    RectF(region_min.x, region_min.y, region_max.x - region_min.x, region_max.y - region_min.y), edit_row);
                GUI::set_next_item_layout(context, GUI::LayoutStyle::fill_width());
                GUI::input_text(context, "PathTextEditing", m_path_edit_text);
                GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(48.0f));
                GUI::ItemHandle go_button = GUI::text_button(context, "Go");
                GUI::end_h_layout(context);
                if (GUI::is_item_clicked(go_button))
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
    void AssetBrowser::tile_context(GUI::IContext* context, const RectF& child_rect)
    {
        Float2 child_size(max(child_rect.width, 1.0f), max(child_rect.height, 1.0f));
        GUI::draw_rect(context, child_rect, Float4U(0.16f, 0.19f, 0.24f, 1.0f), 5.0f);
        GUI::draw_rect(context, RectF(child_rect.offset_x + 1.0f, child_rect.offset_y + 1.0f, max(child_rect.width - 2.0f, 1.0f), max(child_rect.height - 2.0f, 1.0f)),
            Float4U(0.04f, 0.05f, 0.06f, 1.0f), 4.0f);
        GUI::ItemHandle content_hit = GUI::hit_box(context, "Asset Tile Background", child_rect);
        if(m_asset_popup_open && GUI::is_item_clicked(content_hit))
        {
            m_asset_popup_open = false;
        }

        GUI::LayoutDesc host_layout;
        host_layout.padding = GUI::EdgeInsets::all(1.0f);
        host_layout.gap = 0.0f;
        GUI::begin_v_layout(context, "Asset Tile Host", child_rect, host_layout);
        GUI::ItemHandle scroll = GUI::begin_scroll_view(context, "Asset Tile Scroll", GUI::Size::fixed(max(child_size.x - 2.0f, 1.0f), max(child_size.y - 2.0f, 1.0f)));
        RectF scroll_rect = GUI::get_item_state(scroll, GUI::State::rect());
        bool pushed_scroll_clip = scroll_rect.width > 1.0f && scroll_rect.height > 1.0f;
        if(pushed_scroll_clip)
        {
            GUI::push_clip_rect(context, scroll_rect);
        }

        auto assets = get_assets_in_folder(m_path);
        if (succeeded(assets))
        {
            if (assets.get().empty())
            {
                const char* text = "Empty Directory";
                GUI::draw_text(context, child_rect, text, Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
            }
            else
            {
                auto commit_rename = [&](const AssetThumbnail& thumbnail)
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
                    if(valid_filename && thumbnail.m_filename != m_asset_name_editing_buf)
                    {
                        Path from_path = m_path;
                        Path to_path = m_path;
                        from_path.push_back(thumbnail.m_filename);
                        to_path.push_back(m_asset_name_editing_buf);
                        if(thumbnail.m_is_dir)
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
                };

                usize num_assets = assets.get().size();
                constexpr f32 padding = 5.0f;
                constexpr f32 label_height = 24.0f;
                f32 tile_width = m_tile_size + padding * 2.0f;
                f32 tile_height = m_tile_size + padding * 2.0f + label_height;
                GUI::GridLayoutDesc grid;
                grid.sizing_mode = GUI::GridSizingMode::fixed_cell_size;
                grid.cell_size = Float2U(tile_width, tile_height);
                grid.padding = GUI::EdgeInsets::all(10.0f);
                grid.gap = Float2U(8.0f, 8.0f);
                GUI::begin_grid_layout(context, "Asset Tile Grid", grid);

                for (usize i = 0; i < num_assets; ++i)
                {
                    AssetThumbnail& thumbnail = assets.get()[i];
                    bool selected = m_selections.find(thumbnail.m_filename) != m_selections.end();
                    GUI::push_id(context, thumbnail.m_filename.c_str());
                    GUI::ItemHandle tile_hit = GUI::selectable(context, "", selected);
                    RectF tile_rect = GUI::get_item_state(tile_hit, GUI::State::rect());
                    GUI::pop_id(context);
                    bool tile_clicked = GUI::is_item_clicked(tile_hit);
                    bool tile_right_clicked = GUI::is_item_right_clicked(tile_hit);
                    bool tile_double_clicked = GUI::is_item_double_clicked(tile_hit);
                    if(tile_clicked || tile_right_clicked)
                    {
                        m_selections.clear();
                        m_selections.insert(thumbnail.m_filename);
                    }
                    if(tile_right_clicked)
                    {
                        m_popup_asset = thumbnail.m_filename;
                        m_asset_popup_open = true;
                        m_asset_popup_position = GUI::get_pointer_position(context);
                        GUI::open_popup(context, m_asset_popup_handle);
                    }

                    if(tile_rect.width <= 1.0f || tile_rect.height <= 1.0f)
                    {
                        continue;
                    }
                    Float2U tile_min(tile_rect.offset_x + padding, tile_rect.offset_y + padding);
                    RectF icon_rect(tile_min.x, tile_min.y, m_tile_size, m_tile_size);

                    if(selected)
                    {
                        GUI::draw_rect(context, tile_rect, Float4U(0.18f, 0.28f, 0.45f, 0.90f), 5.0f);
                    }

                    if (thumbnail.m_is_dir)
                    {
                        Float4U folder_color(0.78f, 0.78f, 0.78f, 1.0f);
                        GUI::draw_rect(context, RectF(
                            icon_rect.offset_x + m_tile_size * 0.18f,
                            icon_rect.offset_y + m_tile_size * 0.18f,
                            m_tile_size * 0.44f,
                            m_tile_size * 0.18f),
                            folder_color, 5.0f);
                        GUI::draw_rect(context, RectF(
                            icon_rect.offset_x + m_tile_size * 0.08f,
                            icon_rect.offset_y + m_tile_size * 0.30f,
                            m_tile_size * 0.84f,
                            m_tile_size * 0.56f),
                            folder_color, 7.0f);

                        if (tile_double_clicked)
                        {
                            // Change path.
                            auto path = m_path;
                            path.push_back(thumbnail.m_filename);
                            change_path(path);
                        }
                    }
                    else
                    {
                        auto meta_path = m_path;
                        meta_path.push_back(thumbnail.m_filename);
                        auto asset = Asset::get_asset_by_path(meta_path);
                        if (succeeded(asset))
                        {
                            auto draw_rect = icon_rect;

                            GUI::push_id(context, thumbnail.m_filename.c_str());
                            Name asset_ref_payload_type("Asset Ref");
                            if(GUI::begin_drag_drop_source(context, tile_hit, asset_ref_payload_type))
                            {
                                Asset::asset_t payload = asset.get();
                                GUI::set_drag_drop_payload(context, &payload, sizeof(payload));
                                GUI::text(context, meta_path.encode().c_str());
                                GUI::end_drag_drop_source(context);
                            }
                            GUI::pop_id(context);

                            // Editor logic.
                            auto asset_type = Asset::get_asset_type(asset.get());
                            auto iter = g_env->editor_types.find(asset_type);

                            if (iter != g_env->editor_types.end())
                            {
                                if (iter->second.on_draw_tile)
                                {
                                    iter->second.on_draw_tile(context, iter->second.userdata.get(), asset.get(), draw_rect);
                                }
                                else
                                {
                                    GUI::draw_text(context, draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
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
                                GUI::draw_text(context, draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
                            }

                            // Load the data if not loaded.
                            if (Asset::get_asset_state(asset.get()) == Asset::AssetState::unloaded)
                            {
                                async_load_asset(asset.get());
                            }

                            // Draw status circle.
                            if (Asset::get_asset_state(asset.get()) == Asset::AssetState::loaded)
                            {
                                GUI::draw_circle(context, tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::green());
                            }
                            else
                            {
                                GUI::draw_circle(context, tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::yellow());
                            }
                        }
                        else
                        {
                            RectF draw_rect = icon_rect;
                            GUI::draw_text(context, draw_rect, "Unknown", Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
                            GUI::draw_circle(context, tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::red());
                        }
                    }

                    // Draw asset name.
                    if(thumbnail.m_filename == m_editing_asset_name)
                    {
                        RectF edit_rect(tile_min.x, tile_min.y + m_tile_size, m_tile_size, label_height);
                        GUI::LayoutDesc edit_row;
                        edit_row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
                        GUI::begin_h_layout(context, "AssetNameEdit", edit_rect, edit_row);
                        GUI::set_next_item_layout(context, GUI::LayoutStyle::fill_width());
                        GUI::input_text(context, "AssetNameEdit", m_asset_name_editing_buf);
                        GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(34.0f));
                        GUI::ItemHandle rename_ok = GUI::text_button(context, "OK");
                        GUI::end_h_layout(context);
                        if(GUI::is_item_clicked(rename_ok))
                        {
                            commit_rename(thumbnail);
                        }
                    }
                    else
                    {
                        auto& filename = thumbnail.m_filename;
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
                        GUI::draw_text(context, RectF(tile_min.x, tile_min.y + m_tile_size, m_tile_size, label_height),
                            display_text.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
                    }
                }
                GUI::end_grid_layout(context);
                if(pushed_scroll_clip)
                {
                    GUI::pop_clip_rect(context);
                    pushed_scroll_clip = false;
                }

                {
                    constexpr f32 popup_width = 160.0f;
                    constexpr f32 popup_height = 70.0f;
                    GUI::ItemHandle rename_item;
                    GUI::ItemHandle delete_item;
                    bool popup_open = GUI::begin_popup(context, "Asset Popup", m_asset_popup_position, GUI::Size::fixed(popup_width, popup_height), &m_asset_popup_handle);
                    if(popup_open)
                    {
                        m_asset_popup_open = true;
                        rename_item = GUI::selectable(context, "Rename");
                        delete_item = GUI::selectable(context, "Delete");
                        GUI::end_popup(context);
                    }
                    else if(m_asset_popup_open && !GUI::is_popup_open(context, m_asset_popup_handle))
                    {
                        m_asset_popup_open = false;
                    }
                    if (GUI::is_item_clicked(rename_item))
                    {
                        m_editing_asset_name = m_popup_asset;
                        m_asset_name_editing_buf = m_popup_asset.c_str();
                        m_asset_popup_open = false;
                        GUI::close_popup(context, m_asset_popup_handle);
                    }
                    if (GUI::is_item_clicked(delete_item))
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
                        GUI::close_popup(context, m_asset_popup_handle);
                    }
                }
            }
        }
        else
        {
            const char* text_fail = "Failed to display assets in this directory.";
            const char* text_reason = explain(assets.errcode());
            GUI::draw_text(context, RectF(child_rect.offset_x, child_rect.offset_y + child_rect.height * 0.5f - 24.0f, child_rect.width, 24.0f), text_fail,
                Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
            GUI::draw_text(context, RectF(child_rect.offset_x, child_rect.offset_y + child_rect.height * 0.5f, child_rect.width, 24.0f), text_reason,
                Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
        }

        if(pushed_scroll_clip)
        {
            GUI::pop_clip_rect(context);
        }
        GUI::end_scroll_view(context);
        GUI::end_v_layout(context);
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
