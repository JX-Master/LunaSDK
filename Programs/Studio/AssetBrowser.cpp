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
#include <Luna/EditorGUI/EditorGUI.hpp>
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
        GUI::LayoutConfig fixed_size(f32 width, f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUI::LayoutConfig fixed_height(f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUI::LayoutConfig fill_layout()
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::percent;
            layout.height.value = 1.0f;
            layout.flex_grow = 1.0f;
            return layout;
        }

        GUI::FlexLayoutDesc linear_desc(GUI::LayoutAxis axis, f32 gap = 0.0f)
        {
            GUI::FlexLayoutDesc desc;
            desc.axis = axis;
            desc.main_axis_gap = gap;
            return desc;
        }

        void gui_draw_relative_rect(GUI::IContext* context, const RectF& rect, const Float4U& color,
            GUI::paint_order_id_t paint_order_id, f32 radius = 0.0f,
            const Float4U& scale = Float4U(0.0f))
        {
            GUI::DrawCommand command;
            command.type = radius > 0.0f ? GUI::DrawCommandType::rounded_rect : GUI::DrawCommandType::rect;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command, paint_order_id);
        }

        void gui_draw_relative_text(GUI::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size,
            VG::TextAlignment alignment, GUI::paint_order_id_t paint_order_id)
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::text;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command, paint_order_id);
        }

        void gui_draw_folder_icon(GUI::IContext* context, f32 tile_size,
            GUI::paint_order_id_t paint_order_id)
        {
            Float4U folder_color(0.78f, 0.78f, 0.78f, 1.0f);
            gui_draw_relative_rect(context, RectF(
                5.0f + tile_size * 0.18f,
                5.0f + tile_size * 0.18f,
                tile_size * 0.44f,
                tile_size * 0.18f),
                folder_color, paint_order_id, 5.0f);
            gui_draw_relative_rect(context, RectF(
                5.0f + tile_size * 0.08f,
                5.0f + tile_size * 0.30f,
                tile_size * 0.84f,
                tile_size * 0.56f),
                folder_color, paint_order_id, 7.0f);
        }

        void create_new_asset_in_folder(const Path& folder, const Name& type, Name& editing_asset_name, String& editing_buf)
        {
            Path new_asset_path = get_new_asset_path(folder);
            auto asset = Asset::new_asset(new_asset_path, type);
            if(succeeded(asset))
            {
                if(succeeded(Asset::load_asset_data_unit_default_data(asset.get(), Name())))
                {
                    auto _ = Asset::save_asset_data_unit(asset.get(), Name());
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
                        "Rename asset failed", {"OK"}, Window::MessageBoxIcon::error);
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
                        auto _ = Window::message_box(explain(r.errcode()), "Rename directory failed", {"OK"},
                            Window::MessageBoxIcon::error);
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
                            auto _ = Window::message_box(explain(r.errcode()), "Rename asset failed", {"OK"},
                                Window::MessageBoxIcon::error);
                        }
                    }
                }
            }
            editing_asset_name.reset();
        }
    }

    R<GUI::paint_order_id_t> AssetBrowser::draw_tile(GUI::IContext* context,
        const GUI::ElementHandle& element, GUI::DrawPhase phase,
        GUI::paint_order_id_t paint_order_id, void* userdata)
    {
        TileDrawData& data = *(TileDrawData*)userdata;
        GUI::paint_order_id_t current_order = paint_order_id;
        if(data.selected)
        {
            gui_draw_relative_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                Float4U(0.18f, 0.28f, 0.45f, 0.90f), current_order, 5.0f,
                Float4U(0.0f, 0.0f, 1.0f, 1.0f));
        }
        ++current_order;
        constexpr f32 padding = 5.0f;
        constexpr f32 label_height = 24.0f;
        if(data.directory)
        {
            gui_draw_folder_icon(context, data.tile_size, current_order);
        }
        else if(data.asset)
        {
            R<GUI::paint_order_id_t> result = draw_asset_tile_preview(context, data.asset,
                RectF(padding, padding, data.tile_size, data.tile_size), current_order);
            if(failed(result)) return result.errcode();
            current_order = result.get();
        }
        else
        {
            gui_draw_relative_text(context, RectF(padding, padding, data.tile_size, data.tile_size),
                "Unknown", Float4U(1.0f), 16.0f, VG::TextAlignment::center, current_order);
        }
        ++current_order;
        if(!data.directory)
        {
            gui_draw_relative_rect(context,
                RectF(padding + data.tile_size - 16.0f, padding + data.tile_size - 16.0f,
                    12.0f, 12.0f), data.state_color, current_order, 6.0f);
        }
        ++current_order;
        gui_draw_relative_text(context,
            RectF(padding, padding + data.tile_size, data.tile_size, label_height),
            data.label.c_str(), Float4U(1.0f), 16.0f, VG::TextAlignment::center,
            current_order);
        return current_order;
    }

    void AssetBrowser::render(GUI::IContext* context, bool* open)
    {
        for(auto& asset : m_deleting_assets)
        {
            auto r = Asset::delete_asset(asset);
            if(failed(r))
            {
                auto _ = Window::message_box(explain(r.errcode()), "Delete asset failed", {"OK"},
                    Window::MessageBoxIcon::error);
            }
        }
        m_deleting_assets.clear();

        if(open && !*open)
        {
            return;
        }

        context->push_data_scope(context->make_id((GUI::id_t)(usize)this));
        EditorGUI::DockPanelDesc panel_desc;
        panel_desc.minimum_floating_size = Float2U(320.0f, 220.0f);
        if(!EditorGUI::begin_dock_panel(context, context->make_id("asset_browser_panel"), "Asset Browser", open, panel_desc))
        {
            context->pop_data_scope();
            return;
        }
        m_host_focused = true;

        GUI::ElementHandle menu_bar = EditorGUI::begin_menu_bar(context, context->make_id("menu_bar"),
            "Asset Browser Menu Bar", fixed_height(30.0f));
        GUI::ElementHandle folder_item;
        Vector<Pair<Name, GUI::ElementHandle>> asset_items;
        if(EditorGUI::begin_menu(context, context->make_id("new_menu"), "New"))
        {
            folder_item = EditorGUI::menu_item(context, context->make_id("folder"), "Folder");
            asset_items.reserve(g_env->new_asset_types.size());
            for(auto& i : g_env->new_asset_types)
            {
                asset_items.push_back(make_pair(i, EditorGUI::menu_item(context, context->make_id(i.c_str()), i.c_str())));
            }
            lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 220.0f)));
        }
        Vector<Pair<Name, GUI::ElementHandle>> import_items;
        if(EditorGUI::begin_menu(context, context->make_id("import_menu"), "Import"))
        {
            import_items.reserve(g_env->importer_types.size());
            for(auto& i : g_env->importer_types)
            {
                import_items.push_back(make_pair(i.first, EditorGUI::menu_item(context, context->make_id(i.first.c_str()), i.first.c_str())));
            }
            lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 220.0f)));
        }
        EditorGUI::end_menu_bar(context, menu_bar);

        if(EditorGUI::is_item_clicked(context, folder_item))
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
            if(EditorGUI::is_item_clicked(context, item.second))
            {
                create_new_asset_in_folder(m_path, item.first, m_editing_asset_name, m_asset_name_editing_buf);
            }
        }
        for(auto& item : import_items)
        {
            if(EditorGUI::is_item_clicked(context, item.second))
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

        EditorGUI::end_dock_panel(context);
        context->pop_data_scope();
    }

    void AssetBrowser::navbar(GUI::IContext* context)
    {
        constexpr f32 button_size = 24.0f;
        constexpr f32 row_height = 30.0f;
        context->push_data_scope(context->make_id("navbar"));
        GUI::ElementHandle row = EditorGUI::begin_h_layout(context, context->make_id("row"), "Asset Browser Navbar",
            fixed_height(row_height));

        bool back_disabled = (m_current_location_in_histroy_path == 0);
        EditorGUI::ButtonDesc back_desc;
        back_desc.enabled = !back_disabled;
        GUI::ElementHandle back_button = EditorGUI::text_button(context, context->make_id("back"), "<",
            fixed_size(button_size, button_size), back_desc);
        if(EditorGUI::is_item_clicked(context, back_button))
        {
            --m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }

        bool forward_disabled = (m_current_location_in_histroy_path == m_histroy_paths.size() - 1);
        EditorGUI::ButtonDesc forward_desc;
        forward_desc.enabled = !forward_disabled;
        GUI::ElementHandle forward_button = EditorGUI::text_button(context, context->make_id("forward"), ">",
            fixed_size(button_size, button_size), forward_desc);
        if(EditorGUI::is_item_clicked(context, forward_button))
        {
            ++m_current_location_in_histroy_path;
            m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
        }

        bool pop_disabled = m_path.empty();
        EditorGUI::ButtonDesc pop_desc;
        pop_desc.enabled = !pop_disabled;
        GUI::ElementHandle pop_button = EditorGUI::text_button(context, context->make_id("up"), "^",
            fixed_size(button_size, button_size), pop_desc);
        if(EditorGUI::is_item_clicked(context, pop_button))
        {
            auto path = m_path;
            path.pop_back();
            change_path(path);
        }

        if(m_is_navbar_text_editing)
        {
            EditorGUI::input_text(context, context->make_id("path_text"), m_path_edit_text, fixed_height(button_size));
            GUI::ElementHandle go_button = EditorGUI::text_button(context, context->make_id("go"), "Go",
                fixed_size(48.0f, button_size));
            if(EditorGUI::is_item_clicked(context, go_button))
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
            GUI::ElementHandle path_button = EditorGUI::text_button(context, context->make_id("path"), path_text.c_str(),
                fixed_height(button_size));
            if(EditorGUI::is_item_clicked(context, path_button))
            {
                m_is_navbar_text_editing = true;
                m_path_edit_text = path_text;
            }
        }

        EditorGUI::end_h_layout(context, row, linear_desc(GUI::LayoutAxis::x, 6.0f));
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
    void AssetBrowser::tile_context(GUI::IContext* context)
    {
        m_tile_draw_data.clear();
        context->push_data_scope(context->make_id("tile_context"));
        GUI::ElementHandle scroll = EditorGUI::begin_scroll_view(context, context->make_id("scroll"), "Asset Tile Scroll",
            fill_layout());
        auto assets = get_assets_in_folder(m_path);
        if(succeeded(assets))
        {
            if(assets.get().empty())
            {
                EditorGUI::text(context, context->make_id("empty"), "Empty Directory", fixed_height(30.0f));
            }
            else
            {
                m_tile_draw_data.reserve(assets.get().size());
                constexpr f32 padding = 5.0f;
                constexpr f32 label_height = 24.0f;
                f32 tile_width = m_tile_size + padding * 2.0f;
                f32 tile_height = m_tile_size + padding * 2.0f + label_height;
                GUI::ElementHandle grid = EditorGUI::begin_grid_layout(context, context->make_id("grid"), "Asset Tile Grid",
                    fill_layout());
                for(usize i = 0; i < assets.get().size(); ++i)
                {
                    AssetThumbnail& thumbnail = assets.get()[i];
                    bool selected = m_selections.find(thumbnail.m_filename) != m_selections.end();
                    context->push_data_scope(context->make_id(thumbnail.m_filename.c_str()));
                    GUI::ElementHandle tile = EditorGUI::begin_button(context, context->make_id("tile"), thumbnail.m_filename.c_str(),
                        fixed_size(tile_width, tile_height));
                    TileDrawData draw_data;
                    draw_data.selected = selected;
                    draw_data.directory = thumbnail.m_is_dir;
                    draw_data.tile_size = m_tile_size;
                    if(!thumbnail.m_is_dir)
                    {
                        auto meta_path = m_path;
                        meta_path.push_back(thumbnail.m_filename);
                        auto asset = Asset::get_asset_by_path(meta_path);
                        if(succeeded(asset))
                        {
                            auto asset_type = Asset::get_asset_type(asset.get());
                            draw_data.asset = asset.get();

                            auto iter = g_env->editor_types.find(asset_type);
                            if(iter != g_env->editor_types.end() && EditorGUI::is_item_double_clicked(context, tile))
                            {
                                auto edit = iter->second.new_editor(iter->second.userdata.get(), asset.get());
                                m_editor->m_editors.push_back(edit);
                            }

                            auto state = Asset::get_asset_data_unit_state(asset.get(), Name());
                            if(succeeded(state) && state.get() == Asset::AssetDataUnitState::unloaded)
                            {
                                async_load_asset(asset.get(), Name());
                            }
                            Float4U state_color = succeeded(state) && state.get() == Asset::AssetDataUnitState::loaded ?
                                Color::green() : Color::yellow();
                            draw_data.state_color = state_color;
                        }
                        else
                        {
                            draw_data.state_color = Color::red();
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
                    draw_data.label = move(display_text);
                    m_tile_draw_data.push_back(move(draw_data));
                    GUI::ElementHandle visual = context->begin_element(context->make_id("tile.visual"));
                    GUI::LayoutConfig visual_layout = fixed_size(tile_width, tile_height);
                    Name button_style = context->current_style();
                    if(button_style.empty()) button_style = Name(EditorGUI::DEFAULT_STYLE_NAME);
                    Float4U button_padding_value = context->get_style_value(button_style,
                        Name("gui.button.padding"), GUI::style_f32x2(Float2U(10.0f, 6.0f))).number;
                    visual_layout.margin = Float4U(-button_padding_value.x, -button_padding_value.y,
                        -button_padding_value.x, -button_padding_value.y);
                    visual_layout.flex_shrink = 0.0f;
                    context->set_layout_config(visual, visual_layout);
                    context->set_element_debug_name(visual, Name("Asset Tile Visual"));
                    GUI::DrawConfig visual_draw;
                    visual_draw.name = Name("studio.asset_browser.tile");
                    visual_draw.callback = draw_tile;
                    visual_draw.userdata = &m_tile_draw_data.back();
                    context->set_draw_config(visual, visual_draw);
                    context->end_element();
                    EditorGUI::end_button(context);

                    if(EditorGUI::is_item_clicked(context, tile) || EditorGUI::is_item_right_clicked(context, tile))
                    {
                        m_selections.clear();
                        m_selections.insert(thumbnail.m_filename);
                    }
                    if(EditorGUI::is_item_right_clicked(context, tile))
                    {
                        m_popup_asset = thumbnail.m_filename;
                        m_asset_popup_open = true;
                        m_asset_popup_position = context->get_pointer_position();
                        EditorGUI::open_popup(context, context->make_id("asset_popup"));
                    }
                    if(thumbnail.m_is_dir && EditorGUI::is_item_double_clicked(context, tile))
                    {
                        auto path = m_path;
                        path.push_back(thumbnail.m_filename);
                        change_path(path);
                    }
                    context->pop_data_scope();
                }
                GUI::GridLayoutDesc grid_desc;
                grid_desc.mode = GUI::GridLayoutMode::fixed_cell_size;
                grid_desc.cell_size = Float2U(tile_width, tile_height);
                grid_desc.gap = Float2U(8.0f, 8.0f);
                EditorGUI::end_grid_layout(context, grid, grid_desc);
            }
        }
        else
        {
            EditorGUI::text(context, context->make_id("failed"), "Failed to display assets in this directory.", fixed_height(24.0f));
            EditorGUI::text(context, context->make_id("reason"), explain(assets.errcode()), fixed_height(24.0f));
        }
        EditorGUI::end_scroll_view(context);

        GUI::id_t popup_id = context->make_id("asset_popup");
        bool rename_mode = m_editing_asset_name == m_popup_asset && !m_popup_asset.empty();
        EditorGUI::PopupDesc popup_desc;
        popup_desc.position = m_asset_popup_position;
        popup_desc.layout = fixed_size(rename_mode ? 260.0f : 170.0f, rename_mode ? 118.0f : 74.0f);
        GUI::ElementHandle popup;
        if(EditorGUI::begin_popup(context, popup_id, popup_desc, &popup))
        {
            m_asset_popup_open = true;
            if(rename_mode)
            {
                EditorGUI::input_text(context, context->make_id("rename_text"), m_asset_name_editing_buf, fixed_height(30.0f));
                GUI::ElementHandle buttons = EditorGUI::begin_h_layout(context, context->make_id("rename_buttons"),
                    "Rename Buttons", fixed_height(30.0f));
                GUI::ElementHandle ok_button = EditorGUI::text_button(context, context->make_id("rename_ok"), "OK",
                    fixed_size(64.0f, 30.0f));
                GUI::ElementHandle cancel_button = EditorGUI::text_button(context, context->make_id("rename_cancel"),
                    "Cancel", fixed_size(78.0f, 30.0f));
                EditorGUI::end_h_layout(context, buttons, linear_desc(GUI::LayoutAxis::x, 6.0f));
                if(EditorGUI::is_item_clicked(context, ok_button))
                {
                    commit_asset_rename(m_path, m_popup_asset, m_asset_name_editing_buf, m_editing_asset_name);
                    m_asset_popup_open = false;
                    EditorGUI::close_popup(context, popup_id);
                }
                if(EditorGUI::is_item_clicked(context, cancel_button))
                {
                    m_editing_asset_name.reset();
                    m_asset_popup_open = false;
                    EditorGUI::close_popup(context, popup_id);
                }
            }
            else
            {
                GUI::ElementHandle rename_item = EditorGUI::selectable(context, context->make_id("rename"), "Rename",
                    false, fixed_height(28.0f));
                GUI::ElementHandle delete_item = EditorGUI::selectable(context, context->make_id("delete"), "Delete",
                    false, fixed_height(28.0f));
                if(EditorGUI::is_item_clicked(context, rename_item))
                {
                    m_editing_asset_name = m_popup_asset;
                    m_asset_name_editing_buf = m_popup_asset.c_str();
                }
                if(EditorGUI::is_item_clicked(context, delete_item))
                {
                    Path path = m_path;
                    path.push_back(m_popup_asset);
                    auto attr = VFS::get_file_attribute(path);
                    if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
                    {
                        auto r = remove_assets_in_folder(path);
                        if(failed(r))
                        {
                            auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed", {"OK"},
                                Window::MessageBoxIcon::error);
                        }
                        r = remove_dir(path);
                        if(failed(r))
                        {
                            auto _ = Window::message_box(explain(r.errcode()), "Delete directory failed", {"OK"},
                                Window::MessageBoxIcon::error);
                        }
                    }
                    else
                    {
                        auto asset = Asset::get_asset_by_path(path);
                        if(failed(asset))
                        {
                            auto _ = Window::message_box(explain(asset.errcode()), "Delete asset failed", {"OK"},
                                Window::MessageBoxIcon::error);
                        }
                        else
                        {
                            m_deleting_assets.push_back(asset.get());
                        }
                    }
                    m_asset_popup_open = false;
                    EditorGUI::close_popup(context, popup_id);
                }
            }
            lupanic_if_failed(EditorGUI::end_popup(context, popup, RectF(0.0f, 0.0f, popup_desc.layout.width.value,
                popup_desc.layout.height.value)));
        }
        else if(m_asset_popup_open && !EditorGUI::is_popup_open(context, popup_id))
        {
            m_asset_popup_open = false;
            m_editing_asset_name.reset();
        }
        context->pop_data_scope();
    }

    struct AssetLoadTask
    {
        Asset::asset_t asset;
        Name data_unit;
    };
    void async_load_asset_func(JobSystem::IJobScheduler* scheduler, void* params)
    {
        AssetLoadTask* task = (AssetLoadTask*)params;
        auto r = Asset::load_asset_data_unit(task->asset, task->data_unit);
        if(failed(r))
        {
            log_error("Studio", "Failed to load asset %s: %s", Asset::get_asset_path(task->asset).encode().c_str(), explain(r.errcode()));
        }
        memdelete(task);
    }
    void async_load_asset(Asset::asset_t asset, const Name& data_unit)
    {
        AssetLoadTask* task = memnew<AssetLoadTask>();
        task->asset = asset;
        task->data_unit = data_unit;
        g_main_editor->m_job_scheduler->submit_job(async_load_asset_func, task);
    }
}
