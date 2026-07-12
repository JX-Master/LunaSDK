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
#include <Luna/GUI/Editor.hpp>
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
        GUICore::LayoutConfig fixed_size(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutConfig fixed_height(f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutConfig fill_layout()
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::percent;
            layout.height.value = 1.0f;
            layout.flex_grow = 1.0f;
            return layout;
        }

        GUICore::FlexLayoutDesc linear_desc(GUICore::LayoutAxis axis, f32 gap = 0.0f)
        {
            GUICore::FlexLayoutDesc desc;
            desc.axis = axis;
            desc.main_axis_gap = gap;
            return desc;
        }

        void core_draw_relative_rect(GUICore::IContext* context, const RectF& rect, const Float4U& color, f32 radius = 0.0f,
            const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = radius > 0.0f ? GUICore::DrawCommandType::rounded_rect : GUICore::DrawCommandType::rect;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        void core_draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            VG::TextAlignment alignment = VG::TextAlignment::center)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        void core_draw_folder_icon(GUICore::IContext* context, f32 tile_size)
        {
            Float4U folder_color(0.78f, 0.78f, 0.78f, 1.0f);
            core_draw_relative_rect(context, RectF(
                5.0f + tile_size * 0.18f,
                5.0f + tile_size * 0.18f,
                tile_size * 0.44f,
                tile_size * 0.18f),
                folder_color, 5.0f);
            core_draw_relative_rect(context, RectF(
                5.0f + tile_size * 0.08f,
                5.0f + tile_size * 0.30f,
                tile_size * 0.84f,
                tile_size * 0.56f),
                folder_color, 7.0f);
        }

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

        void commit_asset_rename(const Path& folder, const Name& old_name, String& editing_buf, Name& editing_asset_name)
        {
            bool valid_filename = true;
            for(c8 ch : editing_buf)
            {
                if(ch == '\\' || ch == '/' || ch == ':' || ch == '*' || ch == '?' || ch == '\"' || ch == '<' ||
                    ch == '>' || ch == '|')
                {
                    auto _ = Window::message_box(
                        "File or directory name cannot contain the following characters: \\ / : * ? \" < > |",
                        "Rename asset failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                    valid_filename = false;
                    break;
                }
            }
            if(valid_filename && old_name != editing_buf)
            {
                Path from_path = folder;
                Path to_path = folder;
                from_path.push_back(old_name);
                to_path.push_back(editing_buf);
                auto attr = VFS::get_file_attribute(from_path);
                if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
                {
                    auto r = VFS::move_file(from_path, to_path);
                    if(succeeded(r))
                    {
                        r = Asset::load_assets_meta(to_path);
                    }
                    if(failed(r))
                    {
                        auto _ = Window::message_box(explain(r.errcode()), "Rename directory failed",
                            Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
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
                            auto _ = Window::message_box(explain(r.errcode()), "Rename asset failed",
                                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                        }
                    }
                }
            }
            editing_asset_name.reset();
        }
    }

    void AssetBrowser::render(GUICore::IContext* context, bool* open)
    {
        for(auto& asset : m_deleting_assets)
        {
            auto r = Asset::delete_asset(asset);
            if(failed(r))
            {
                auto _ = Window::message_box(explain(r.errcode()), "Delete asset failed", Window::MessageBoxType::ok,
                    Window::MessageBoxIcon::error);
            }
        }
        m_deleting_assets.clear();

        if(open && !*open)
        {
            return;
        }

        context->push_data_scope(context->make_id((GUICore::id_t)(usize)this));
        GUI::DockPanelStyle panel_style;
        panel_style.min_floating_size = Float2U(320.0f, 220.0f);
        GUICore::ElementHandle panel;
        if(!GUI::begin_dock_panel(context, context->make_id("asset_browser_panel"), "Asset Browser", open, panel_style,
            fill_layout(), &panel))
        {
            context->pop_data_scope();
            return;
        }
        m_host_focused = GUI::is_item_focused(context, panel) || GUI::is_item_hovered(context, panel) ||
            GUI::is_item_active(context, panel);

        GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, context->make_id("menu_bar"),
            "Asset Browser Menu Bar", fixed_height(30.0f));
        GUICore::ElementHandle folder_item;
        Vector<Pair<Name, GUICore::ElementHandle>> asset_items;
        if(GUI::begin_menu(context, context->make_id("new_menu"), "New"))
        {
            folder_item = GUI::menu_item(context, context->make_id("folder"), "Folder");
            asset_items.reserve(g_env->new_asset_types.size());
            for(auto& i : g_env->new_asset_types)
            {
                asset_items.push_back(make_pair(i, GUI::menu_item(context, context->make_id(i.c_str()), i.c_str())));
            }
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 220.0f)));
        }
        Vector<Pair<Name, GUICore::ElementHandle>> import_items;
        if(GUI::begin_menu(context, context->make_id("import_menu"), "Import"))
        {
            import_items.reserve(g_env->importer_types.size());
            for(auto& i : g_env->importer_types)
            {
                import_items.push_back(make_pair(i.first, GUI::menu_item(context, context->make_id(i.first.c_str()), i.first.c_str())));
            }
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 220.0f)));
        }
        lupanic_if_failed(GUI::end_menu_bar(context, menu_bar));

        if(GUI::is_item_clicked(context, folder_item))
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
            if(GUI::is_item_clicked(context, item.second))
            {
                create_new_asset_in_folder(m_path, item.first, m_editing_asset_name, m_asset_name_editing_buf);
            }
        }
        for(auto& item : import_items)
        {
            if(GUI::is_item_clicked(context, item.second))
            {
                auto iter = g_env->importer_types.find(item.first);
                if(iter != g_env->importer_types.end())
                {
                    auto editor = iter->second.new_importer(m_path);
                    m_editor->m_editors.push_back(editor);
                }
            }
        }

        navbar(context);
        tile_context(context);

        GUI::end_dock_panel(context);
        context->pop_data_scope();
    }

    void AssetBrowser::navbar(GUICore::IContext* context)
    {
        constexpr f32 button_size = 24.0f;
        constexpr f32 row_height = 30.0f;
        context->push_data_scope(context->make_id("navbar"));
        GUICore::ElementHandle row = GUI::begin_h_layout(context, context->make_id("row"), "Asset Browser Navbar",
            fixed_height(row_height));

        bool back_disabled = (m_current_location_in_histroy_path == 0);
        GUICore::ElementHandle back_button = GUI::text_button(context, context->make_id("back"), "<",
            fixed_size(button_size, button_size), !back_disabled);
        if(GUI::is_item_clicked(context, back_button))
        {
            --m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }

        bool forward_disabled = (m_current_location_in_histroy_path == m_histroy_paths.size() - 1);
        GUICore::ElementHandle forward_button = GUI::text_button(context, context->make_id("forward"), ">",
            fixed_size(button_size, button_size), !forward_disabled);
        if(GUI::is_item_clicked(context, forward_button))
        {
            ++m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }

        bool pop_disabled = m_path.empty();
        GUICore::ElementHandle pop_button = GUI::text_button(context, context->make_id("up"), "^",
            fixed_size(button_size, button_size), !pop_disabled);
        if(GUI::is_item_clicked(context, pop_button))
        {
            auto path = m_path;
            path.pop_back();
            change_path(path);
        }

        if(m_is_navbar_text_editing)
        {
            GUI::input_text(context, context->make_id("path_text"), m_path_edit_text, fixed_height(button_size));
            GUICore::ElementHandle go_button = GUI::text_button(context, context->make_id("go"), "Go",
                fixed_size(48.0f, button_size));
            if(GUI::is_item_clicked(context, go_button))
            {
                m_is_navbar_text_editing = false;
                auto new_path = Path(m_path_edit_text.c_str());
                auto attr = VFS::get_file_attribute(new_path);
                if(succeeded(attr) && ((attr.get().attributes & FileAttributeFlag::directory) != FileAttributeFlag::none))
                {
                    change_path(new_path);
                }
            }
        }
        else
        {
            String path_text = m_path.encode(PathSeparator::slash, true);
            GUICore::ElementHandle path_button = GUI::text_button(context, context->make_id("path"), path_text.c_str(),
                fixed_height(button_size));
            if(GUI::is_item_clicked(context, path_button))
            {
                m_is_navbar_text_editing = true;
                m_path_edit_text = path_text;
            }
        }

        lupanic_if_failed(GUI::end_h_layout(context, row, linear_desc(GUICore::LayoutAxis::x, 6.0f)));
        context->pop_data_scope();
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
    void AssetBrowser::tile_context(GUICore::IContext* context)
    {
        context->push_data_scope(context->make_id("tile_context"));
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"), "Asset Tile Scroll",
            fill_layout());
        auto assets = get_assets_in_folder(m_path);
        if(succeeded(assets))
        {
            if(assets.get().empty())
            {
                GUI::text(context, context->make_id("empty"), "Empty Directory", fixed_height(30.0f));
            }
            else
            {
                constexpr f32 padding = 5.0f;
                constexpr f32 label_height = 24.0f;
                f32 tile_width = m_tile_size + padding * 2.0f;
                f32 tile_height = m_tile_size + padding * 2.0f + label_height;
                GUICore::ElementHandle grid = GUI::begin_grid_layout(context, context->make_id("grid"), "Asset Tile Grid",
                    fill_layout());
                for(usize i = 0; i < assets.get().size(); ++i)
                {
                    AssetThumbnail& thumbnail = assets.get()[i];
                    bool selected = m_selections.find(thumbnail.m_filename) != m_selections.end();
                    context->push_data_scope(context->make_id(thumbnail.m_filename.c_str()));
                    GUICore::ElementHandle tile = GUI::begin_button(context, context->make_id("tile"), thumbnail.m_filename.c_str(),
                        fixed_size(tile_width, tile_height));
                    if(selected)
                    {
                        core_draw_relative_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                            Float4U(0.18f, 0.28f, 0.45f, 0.90f), 5.0f);
                    }

                    if(thumbnail.m_is_dir)
                    {
                        core_draw_folder_icon(context, m_tile_size);
                    }
                    else
                    {
                        auto meta_path = m_path;
                        meta_path.push_back(thumbnail.m_filename);
                        auto asset = Asset::get_asset_by_path(meta_path);
                        if(succeeded(asset))
                        {
                            auto asset_type = Asset::get_asset_type(asset.get());
                            draw_asset_tile_preview(context, asset.get(), RectF(padding, padding, m_tile_size, m_tile_size));

                            auto iter = g_env->editor_types.find(asset_type);
                            if(iter != g_env->editor_types.end() && GUI::is_item_double_clicked(context, tile))
                            {
                                auto edit = iter->second.new_editor(iter->second.userdata.get(), asset.get());
                                m_editor->m_editors.push_back(edit);
                            }

                            if(Asset::get_asset_state(asset.get()) == Asset::AssetState::unloaded)
                            {
                                async_load_asset(asset.get());
                            }
                            Float4U state_color = Asset::get_asset_state(asset.get()) == Asset::AssetState::loaded ?
                                Color::green() : Color::yellow();
                            core_draw_relative_rect(context, RectF(padding + m_tile_size - 16.0f, padding + m_tile_size - 16.0f,
                                12.0f, 12.0f), state_color, 6.0f);
                        }
                        else
                        {
                            core_draw_relative_text(context, RectF(padding, padding, m_tile_size, m_tile_size), "Unknown",
                                Float4U(1.0f), 16.0f);
                            core_draw_relative_rect(context, RectF(padding + m_tile_size - 16.0f, padding + m_tile_size - 16.0f,
                                12.0f, 12.0f), Color::red(), 6.0f);
                        }
                    }

                    const c8* display_name = thumbnail.m_filename.c_str();
                    String display_text;
                    constexpr usize CLAMP_LEN = 12;
                    if(thumbnail.m_filename.size() > CLAMP_LEN)
                    {
                        Name clipped_name(thumbnail.m_filename.c_str(), CLAMP_LEN - 1);
                        strprintf(display_text, "%s...", clipped_name.c_str());
                    }
                    else
                    {
                        display_text = display_name;
                    }
                    core_draw_relative_text(context, RectF(padding, padding + m_tile_size, m_tile_size, label_height),
                        display_text.c_str(), Float4U(1.0f), 16.0f);
                    GUI::end_button(context);

                    if(GUI::is_item_clicked(context, tile) || GUI::is_item_right_clicked(context, tile))
                    {
                        m_selections.clear();
                        m_selections.insert(thumbnail.m_filename);
                    }
                    if(GUI::is_item_right_clicked(context, tile))
                    {
                        m_popup_asset = thumbnail.m_filename;
                        m_asset_popup_open = true;
                        m_asset_popup_position = context->get_pointer_position();
                        GUI::open_popup(context, context->make_id("asset_popup"));
                    }
                    if(thumbnail.m_is_dir && GUI::is_item_double_clicked(context, tile))
                    {
                        auto path = m_path;
                        path.push_back(thumbnail.m_filename);
                        change_path(path);
                    }
                    context->pop_data_scope();
                }
                GUICore::GridLayoutDesc grid_desc;
                grid_desc.mode = GUICore::GridLayoutMode::fixed_cell_size;
                grid_desc.cell_size = Float2U(tile_width, tile_height);
                grid_desc.gap = Float2U(8.0f, 8.0f);
                lupanic_if_failed(GUI::end_grid_layout(context, grid, grid_desc));
            }
        }
        else
        {
            GUI::text(context, context->make_id("failed"), "Failed to display assets in this directory.", fixed_height(24.0f));
            GUI::text(context, context->make_id("reason"), explain(assets.errcode()), fixed_height(24.0f));
        }
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));

        GUICore::id_t popup_id = context->make_id("asset_popup");
        bool rename_mode = m_editing_asset_name == m_popup_asset && !m_popup_asset.empty();
        GUI::PopupDesc popup_desc;
        popup_desc.position = m_asset_popup_position;
        popup_desc.layout = fixed_size(rename_mode ? 260.0f : 170.0f, rename_mode ? 118.0f : 74.0f);
        GUICore::ElementHandle popup;
        if(GUI::begin_popup(context, popup_id, popup_desc, &popup))
        {
            m_asset_popup_open = true;
            if(rename_mode)
            {
                GUI::input_text(context, context->make_id("rename_text"), m_asset_name_editing_buf, fixed_height(30.0f));
                GUICore::ElementHandle buttons = GUI::begin_h_layout(context, context->make_id("rename_buttons"),
                    "Rename Buttons", fixed_height(30.0f));
                GUICore::ElementHandle ok_button = GUI::text_button(context, context->make_id("rename_ok"), "OK",
                    fixed_size(64.0f, 30.0f));
                GUICore::ElementHandle cancel_button = GUI::text_button(context, context->make_id("rename_cancel"),
                    "Cancel", fixed_size(78.0f, 30.0f));
                lupanic_if_failed(GUI::end_h_layout(context, buttons, linear_desc(GUICore::LayoutAxis::x, 6.0f)));
                if(GUI::is_item_clicked(context, ok_button))
                {
                    commit_asset_rename(m_path, m_popup_asset, m_asset_name_editing_buf, m_editing_asset_name);
                    m_asset_popup_open = false;
                    GUI::close_popup(context, popup_id);
                }
                if(GUI::is_item_clicked(context, cancel_button))
                {
                    m_editing_asset_name.reset();
                    m_asset_popup_open = false;
                    GUI::close_popup(context, popup_id);
                }
            }
            else
            {
                GUICore::ElementHandle rename_item = GUI::selectable(context, context->make_id("rename"), "Rename",
                    false, fixed_height(28.0f));
                GUICore::ElementHandle delete_item = GUI::selectable(context, context->make_id("delete"), "Delete",
                    false, fixed_height(28.0f));
                if(GUI::is_item_clicked(context, rename_item))
                {
                    m_editing_asset_name = m_popup_asset;
                    m_asset_name_editing_buf = m_popup_asset.c_str();
                }
                if(GUI::is_item_clicked(context, delete_item))
                {
                    Path path = m_path;
                    path.push_back(m_popup_asset);
                    auto attr = VFS::get_file_attribute(path);
                    if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
                    {
                        auto r = remove_assets_in_folder(path);
                        if(failed(r))
                        {
                            auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed",
                                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                        }
                        r = remove_dir(path);
                        if(failed(r))
                        {
                            auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed",
                                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                        }
                    }
                    else
                    {
                        auto asset = Asset::get_asset_by_path(path);
                        if(failed(asset))
                        {
                            auto _ = Window::message_box(explain(asset.errcode()), "Delete asset failed",
                                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                        }
                        else
                        {
                            m_deleting_assets.push_back(asset.get());
                        }
                    }
                    m_asset_popup_open = false;
                    GUI::close_popup(context, popup_id);
                }
            }
            lupanic_if_failed(GUI::end_popup(context, popup, RectF(0.0f, 0.0f, popup_desc.layout.width.value,
                popup_desc.layout.height.value)));
        }
        else if(m_asset_popup_open && !GUI::is_popup_open(context, popup_id))
        {
            m_asset_popup_open = false;
            m_editing_asset_name.reset();
        }
        context->pop_data_scope();
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
