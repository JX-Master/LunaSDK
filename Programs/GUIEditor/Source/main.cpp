/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file main.cpp
* @author JXMaster
* @date 2026/6/10
*/
#include "EditorService.hpp"
#include "PaletteIcons.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/HID/HID.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Luna;

namespace Luna
{
    namespace GUIEditor
    {
        struct NodeHandle
        {
            Guid node;
            GUI::ItemHandle handle;
        };

        struct PalettePayload
        {
            u32 type_index = 0;
        };

        struct TreeNodePayload
        {
            Guid node;
        };

        struct TypeItemHandle
        {
            u32 type_index = 0;
            GUI::ItemHandle handle;
        };

        struct FrameHandles
        {
            GUI::ItemHandle new_document;
            GUI::ItemHandle open_document;
            GUI::ItemHandle save_document;
            GUI::ItemHandle undo;
            GUI::ItemHandle redo;
            GUI::ItemHandle new_node;
            GUI::ItemHandle new_node_popup;
            GUI::ItemHandle remove_node_menu;
            GUI::ItemHandle remove_node;
            GUI::ItemHandle move_up;
            GUI::ItemHandle move_down;
            GUI::ItemHandle apply_common;
            GUI::ItemHandle set_property;
            GUI::ItemHandle erase_property;
            GUI::ItemHandle preview_drop_target;
            Vector<NodeHandle> tree_nodes;
            Vector<TypeItemHandle> new_node_items;
        };

        struct App
        {
            Ref<Window::IWindow> window;
            Ref<RHI::ISwapChain> swap_chain;
            Ref<RHI::ICommandBuffer> cmdbuf;
            Ref<GUI::IContext> gui;
            EditorService service;
            Vector<Name> node_types;
            PaletteIcons palette_icons;
            u32 queue = U32_MAX;
            u32 width = 0;
            u32 height = 0;
            bool show_preview = true;
            bool show_properties = true;
            bool dockspace_layout_initialized = false;
            Guid inspector_node = Guid(0, 0);
            String open_path = "/sample.guiasset";
            String save_path = "/sample.guiasset";
            String edit_label;
            String edit_style;
            bool edit_enabled = true;
            String property_key = "value";
            String property_value = "";
            i32 property_type = 0;
            bool shortcut_new_down = false;
            bool shortcut_open_down = false;
            bool shortcut_save_down = false;
            bool shortcut_undo_down = false;
            bool shortcut_redo_down = false;
            bool shortcut_delete_down = false;
            bool shortcut_move_up_down = false;
            bool shortcut_move_down_down = false;
        };

        enum class TreeDropPlacement : u8
        {
            child,
            before,
            after
        };

        static Name palette_payload_type()
        {
            return Name("gui_editor.palette_node");
        }

        static Name tree_node_payload_type()
        {
            return Name("gui_editor.tree_node");
        }

        static void set_current_dir_to_process_path()
        {
            const c8* path = get_process_path();
            Path p = path;
            release_process_path(path);
            p.pop_back();
            luassert_always(succeeded(set_current_dir(p.encode().c_str())));
        }

        static RV mount_current_dir()
        {
            const c8* dir = get_current_dir();
            Path path = dir;
            release_current_dir(dir);
            return VFS::mount(VFS::get_platform_filesystem_driver(), path.encode(PathSeparator::system_preferred).c_str(), "/");
        }

        static String variant_to_text(const Variant& value)
        {
            switch(value.type())
            {
            case VariantType::null:
                return "null";
            case VariantType::string:
                return value.c_str();
            case VariantType::boolean:
                return value.boolean(false) ? "true" : "false";
            case VariantType::number:
            {
                c8 buf[64];
                if(value.number_type() == VariantNumberType::number_f64)
                {
                    snprintf(buf, sizeof(buf), "%.4f", value.fnum());
                }
                else
                {
                    snprintf(buf, sizeof(buf), "%lld", (long long)value.inum());
                }
                return buf;
            }
            default:
                return VariantUtils::write_json(value);
            }
        }

        static bool selected_node_order(EditorDocument* document, usize& index, usize& count)
        {
            index = USIZE_MAX;
            count = 0;
            if(!document || !document->asset || document->selected_node == Guid(0, 0) ||
                document->selected_node == GA::get_root(document->asset.get()))
            {
                return false;
            }
            Ref<GA::Node> selected = GA::find_node(document->asset.get(), document->selected_node);
            if(!selected)
            {
                return false;
            }
            Ref<GA::Node> parent = GA::find_node(document->asset.get(), GA::get_parent(selected.get()));
            if(!parent)
            {
                return false;
            }
            Span<const Guid> children = GA::get_children(parent.get());
            count = children.size();
            for(usize i = 0; i < children.size(); ++i)
            {
                if(children[i] == document->selected_node)
                {
                    index = i;
                    return true;
                }
            }
            return false;
        }

        static bool node_order(EditorDocument* document, const Guid& node_id, Guid& parent, usize& index, usize& count)
        {
            parent = Guid(0, 0);
            index = USIZE_MAX;
            count = 0;
            if(!document || !document->asset || node_id == Guid(0, 0))
            {
                return false;
            }
            Ref<GA::Node> node = GA::find_node(document->asset.get(), node_id);
            if(!node)
            {
                return false;
            }
            parent = GA::get_parent(node.get());
            if(parent == Guid(0, 0))
            {
                return false;
            }
            Ref<GA::Node> parent_node = GA::find_node(document->asset.get(), parent);
            if(!parent_node)
            {
                return false;
            }
            Span<const Guid> children = GA::get_children(parent_node.get());
            count = children.size();
            for(usize i = 0; i < children.size(); ++i)
            {
                if(children[i] == node_id)
                {
                    index = i;
                    return true;
                }
            }
            return false;
        }

        static TreeDropPlacement tree_drop_placement(App& app, EditorDocument& document, const NodeHandle& target)
        {
            if(target.node == GA::get_root(document.asset.get()))
            {
                return TreeDropPlacement::child;
            }
            RectF rect = GUI::get_item_state(target.handle, GUI::State::rect());
            if(rect.height <= 1.0f)
            {
                return TreeDropPlacement::child;
            }
            Float2U pointer = GUI::get_pointer_position(app.gui);
            f32 y = pointer.y - rect.offset_y;
            if(y < rect.height * 0.25f)
            {
                return TreeDropPlacement::before;
            }
            if(y > rect.height * 0.75f)
            {
                return TreeDropPlacement::after;
            }
            return TreeDropPlacement::child;
        }

        static RV create_node_at(EditorService& service, EditorDocument& document, const Name& type, const c8* label, const Guid& parent, usize index)
        {
            return service.create_node(document.id, parent, type, label, index);
        }

        static RV move_node_at(EditorService& service, EditorDocument& document, const Guid& node, const Guid& parent, usize index)
        {
            if(node == Guid(0, 0) || node == GA::get_root(document.asset.get()))
            {
                return BasicError::bad_arguments();
            }
            return service.move_node(document.id, node, parent, index);
        }

        static void set_drop_status(EditorService& service, const RV& result)
        {
            if(failed(result))
            {
                service.last_status = explain(result.errcode());
            }
        }

        static bool can_remove_selected(EditorDocument* document)
        {
            return document && document->asset && document->selected_node != Guid(0, 0) &&
                document->selected_node != GA::get_root(document->asset.get());
        }

        static bool can_move_selected(EditorDocument* document, bool down)
        {
            usize index = 0;
            usize count = 0;
            if(!selected_node_order(document, index, count))
            {
                return false;
            }
            return down ? index + 1 < count : index > 0;
        }

        static bool key_edge(bool down, bool& previous)
        {
            bool pressed = down && !previous;
            previous = down;
            return pressed;
        }

        static bool shortcut_pressed(App& app, KeyCode key, bool& previous, bool shortcut_modifier, bool shift_modifier = false)
        {
            if(app.gui->get_text_input_state().active)
            {
                return key_edge(false, previous);
            }
            GUI::KeyModifierFlag modifiers = GUI::get_key_modifiers(app.gui);
            bool shortcut_down = test_flags(modifiers, GUI::KeyModifierFlag::ctrl) || test_flags(modifiers, GUI::KeyModifierFlag::system);
            bool shift_down = test_flags(modifiers, GUI::KeyModifierFlag::shift);
            bool down = GUI::is_key_down(app.gui, key);
            if(shortcut_modifier)
            {
                down = down && shortcut_down;
            }
            if(shift_modifier)
            {
                down = down && shift_down;
            }
            return key_edge(down, previous);
        }

        static String guid_to_text(const Guid& id)
        {
            auto data = serialize(id);
            if(succeeded(data))
            {
                return VariantUtils::write_json(data.get());
            }
            return "<invalid guid>";
        }

        static Variant make_property_value(i32 type, const String& text)
        {
            switch(type)
            {
            case 1:
                return (f64)std::strtod(text.c_str(), nullptr);
            case 2:
                return (i64)std::strtoll(text.c_str(), nullptr, 10);
            case 3:
                return strcmp(text.c_str(), "true") == 0 || strcmp(text.c_str(), "1") == 0;
            default:
                return text.c_str();
            }
        }

        static void refresh_node_types(App& app)
        {
            app.node_types.clear();
            GA::get_node_types(app.node_types);
        }

        static void sync_inspector(App& app)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                return;
            }
            if(app.inspector_node == document->selected_node)
            {
                return;
            }
            app.inspector_node = document->selected_node;
            Ref<GA::Node> node = GA::find_node(document->asset.get(), app.inspector_node);
            if(node)
            {
                app.edit_label = node->label;
                app.edit_style = node->style.c_str();
                app.edit_enabled = node->enabled;
            }
        }

        static void register_tree_drop_target(App& app, GUI::ItemHandle handle)
        {
            if(GUI::begin_drag_drop_target(app.gui, handle, palette_payload_type()))
            {
                (void)GUI::accept_drag_drop_payload(app.gui, palette_payload_type());
                GUI::end_drag_drop_target(app.gui);
            }
            if(GUI::begin_drag_drop_target(app.gui, handle, tree_node_payload_type()))
            {
                (void)GUI::accept_drag_drop_payload(app.gui, tree_node_payload_type());
                GUI::end_drag_drop_target(app.gui);
            }
        }

        static void register_tree_drag_source(App& app, EditorDocument& document, const Guid& id, GUI::ItemHandle handle)
        {
            if(id == GA::get_root(document.asset.get()))
            {
                return;
            }
            TreeNodePayload payload;
            payload.node = id;
            if(GUI::begin_drag_drop_source(app.gui, handle, tree_node_payload_type()))
            {
                GUI::set_drag_drop_payload(app.gui, &payload, sizeof(payload));
                Ref<GA::Node> node = GA::find_node(document.asset.get(), id);
                GUI::text(app.gui, node ? node->label.c_str() : "Move node");
                GUI::end_drag_drop_source(app.gui);
            }
        }

        static void register_palette_drag_source(App& app, usize type_index, GUI::ItemHandle handle)
        {
            PalettePayload payload;
            payload.type_index = (u32)type_index;
            if(GUI::begin_drag_drop_source(app.gui, handle, palette_payload_type()))
            {
                GUI::set_drag_drop_payload(app.gui, &payload, sizeof(payload));
                if(type_index < app.node_types.size())
                {
                    c8 label[160];
                    snprintf(label, sizeof(label), "Create %s", app.node_types[type_index].c_str());
                    GUI::text(app.gui, label);
                }
                else
                {
                    GUI::text(app.gui, "Create node");
                }
                GUI::end_drag_drop_source(app.gui);
            }
        }

        static void draw_node_tree(App& app, FrameHandles& handles, EditorDocument& document, const Guid& id)
        {
            Ref<GA::Node> node = GA::find_node(document.asset.get(), id);
            if(!node)
            {
                return;
            }
            c8 label[256];
            snprintf(label, sizeof(label), "%s  [%s]", node->label.empty() ? "(unnamed)" : node->label.c_str(), node->type.c_str());
            GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::default_open;
            if(GA::get_child_count(node.get()) == 0)
            {
                flags |= GUI::TreeNodeFlag::leaf;
            }
            if(id == document.selected_node)
            {
                flags |= GUI::TreeNodeFlag::selected;
            }
            GUI::push_id(app.gui, id.high);
            GUI::push_id(app.gui, id.low);
            GUI::ItemHandle handle = GUI::tree_node(app.gui, label, flags);
            handles.tree_nodes.push_back({id, handle});
            register_tree_drag_source(app, document, id, handle);
            register_tree_drop_target(app, handle);
            bool open = GUI::get_item_state(handle, GUI::State::open());
            if(open && GA::get_child_count(node.get()) > 0)
            {
                GUI::tree_push(app.gui, handle);
                for(const Guid& child : GA::get_children(node.get()))
                {
                    draw_node_tree(app, handles, document, child);
                }
                GUI::tree_pop(app.gui);
            }
            GUI::pop_id(app.gui);
            GUI::pop_id(app.gui);
        }

        static bool is_core_palette_type(const Name& type)
        {
            const c8* name = type.c_str();
            return strcmp(name, "h_layout") == 0 ||
                strcmp(name, "v_layout") == 0 ||
                strcmp(name, "scroll_view") == 0 ||
                strcmp(name, "grid_layout") == 0 ||
                strcmp(name, "canvas_layout") == 0 ||
                strcmp(name, "text") == 0 ||
                strcmp(name, "button") == 0 ||
                strcmp(name, "input_text") == 0 ||
                strcmp(name, "image") == 0 ||
                strcmp(name, "checkbox") == 0;
        }

        static void draw_palette_panel(App& app, FrameHandles& handles)
        {
            (void)handles;
            GUI::text(app.gui, "Palette");
            GUI::GridLayoutDesc palette_grid;
            palette_grid.sizing_mode = GUI::GridSizingMode::fixed_columns;
            palette_grid.columns = 2;
            palette_grid.cell_size = Float2U(42.0f, 38.0f);
            palette_grid.padding = GUI::EdgeInsets::all(0.0f);
            palette_grid.gap = Float2U(6.0f, 6.0f);
            palette_grid.cell_cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;
            GUI::begin_grid_layout(app.gui, "Palette Grid", palette_grid);
            for(usize i = 0; i < app.node_types.size(); ++i)
            {
                if(!is_core_palette_type(app.node_types[i]))
                {
                    continue;
                }
                GUI::push_id(app.gui, i);
                GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill());
                GUI::ShapeDesc& icon = palette_icon(app.palette_icons, app.node_types[i]);
                GUI::ItemHandle h = GUI::shape_button(app.gui, app.node_types[i].c_str(), icon, GUI::Size::fixed(18.0f, 18.0f));
                c8 tooltip[192];
                snprintf(tooltip, sizeof(tooltip), "Drag %s into the tree or preview.", app.node_types[i].c_str());
                GUI::set_item_tooltip(app.gui, h, tooltip);
                register_palette_drag_source(app, i, h);
                GUI::pop_id(app.gui);
            }
            GUI::end_grid_layout(app.gui);
        }

        static void draw_tree_panel(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.gui, "No document.");
                return;
            }
            c8 info[256];
            snprintf(info, sizeof(info), "Document %llu%s", (unsigned long long)document->id, document->dirty ? " *" : "");
            GUI::text(app.gui, info);
            GUI::LayoutDesc action_layout;
            action_layout.gap = 4.0f;
            GUI::begin_h_layout(app.gui, "Tree Actions", action_layout);
            GUI::set_next_item_enabled(app.gui, document && document->asset && !app.node_types.empty());
            handles.new_node = GUI::text_button(app.gui, "New");
            GUI::set_next_item_enabled(app.gui, can_move_selected(document, false));
            handles.move_up = GUI::text_button(app.gui, "Move Up");
            GUI::set_next_item_enabled(app.gui, can_move_selected(document, true));
            handles.move_down = GUI::text_button(app.gui, "Move Down");
            GUI::set_next_item_enabled(app.gui, can_remove_selected(document));
            handles.remove_node = GUI::text_button(app.gui, "Delete");
            GUI::end_h_layout(app.gui);

            RectF new_rect = GUI::get_item_state(handles.new_node, GUI::State::rect());
            GUI::PopupDesc popup_desc;
            popup_desc.position = Float2U(new_rect.offset_x, new_rect.offset_y + new_rect.height + 4.0f);
            popup_desc.size = GUI::Size::fixed(220.0f, 0.0f);
            if(GUI::begin_popup(app.gui, "New Node Popup", popup_desc, &handles.new_node_popup))
            {
                for(usize i = 0; i < app.node_types.size(); ++i)
                {
                    GUI::push_id(app.gui, i);
                    GUI::ItemHandle item = GUI::menu_item(app.gui, app.node_types[i].c_str());
                    handles.new_node_items.push_back({(u32)i, item});
                    GUI::pop_id(app.gui);
                }
                GUI::end_popup(app.gui);
            }
            draw_node_tree(app, handles, *document, GA::get_root(document->asset.get()));
        }

        static void draw_properties_panel(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.gui, "No document.");
                return;
            }
            Ref<GA::Node> node = GA::find_node(document->asset.get(), document->selected_node);
            if(!node)
            {
                GUI::text(app.gui, "No node selected.");
                return;
            }
            GUI::text(app.gui, "Selected Node");
            GUI::text(app.gui, guid_to_text(node->id).c_str());
            c8 type_text[128];
            snprintf(type_text, sizeof(type_text), "Type: %s", node->type.c_str());
            GUI::text(app.gui, type_text);
            GUI::input_text(app.gui, "Label", app.edit_label);
            GUI::input_text(app.gui, "Style", app.edit_style);
            GUI::checkbox(app.gui, "Enabled", &app.edit_enabled);
            handles.apply_common = GUI::text_button(app.gui, "Apply Common Fields");

            GUI::text(app.gui, "Properties");
            if(node->properties.type() == VariantType::object)
            {
                GUI::TableDesc table;
                table.column_sizes.push_back(GUI::TableTrackSize::fixed(120.0f));
                table.column_sizes.push_back(GUI::TableTrackSize::fixed(220.0f));
                table.style.row_separators = true;
                table.style.column_separators = true;
                table.style.background_mode = GUI::TableBackgroundMode::alternate_rows;
                GUI::begin_table_layout(app.gui, "Property Table", table);
                for(const auto& kv : node->properties.key_values())
                {
                    if(GUI::begin_table_row(app.gui))
                    {
                        GUI::text(app.gui, kv.first.c_str());
                        GUI::text(app.gui, variant_to_text(kv.second).c_str());
                        GUI::end_table_row(app.gui);
                    }
                }
                GUI::end_table_layout(app.gui);
            }
            GUI::input_text(app.gui, "Property Key", app.property_key);
            const c8* type_items[] = {"String", "Number", "Integer", "Boolean"};
            GUI::button_group(app.gui, "Property Type", &app.property_type, Span<const c8*>(type_items, 4));
            GUI::input_text(app.gui, "Property Value", app.property_value);
            handles.set_property = GUI::text_button(app.gui, "Set Property");
            handles.erase_property = GUI::text_button(app.gui, "Erase Property");
        }

        static void draw_preview_panel(App& app)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.gui, "No document.");
                return;
            }
            GUI::text(app.gui, "Live Preview");
            GUI::text(app.gui, "Drag palette or tree nodes into this panel to append to root.");
            GUI::push_id(app.gui, "preview");
            GUI::push_enabled(app.gui, false);
            RV r = GA::generate(app.gui, document->asset.get());
            GUI::pop_enabled(app.gui);
            GUI::pop_id(app.gui);
            if(failed(r))
            {
                GUI::text(app.gui, explain(r.errcode()));
            }
        }

        static void draw_history_panel(App& app)
        {
            EditorDocument* document = app.service.active_document();
            if(!document)
            {
                GUI::text(app.gui, "No document.");
                return;
            }
            c8 status[256];
            snprintf(status, sizeof(status), "Undo %llu | Redo %llu", (unsigned long long)document->undo_stack.size(), (unsigned long long)document->redo_stack.size());
            GUI::text(app.gui, status);
            if(!app.service.last_status.empty())
            {
                GUI::text(app.gui, app.service.last_status.c_str());
            }
            for(isize i = (isize)document->undo_stack.size() - 1; i >= 0 && i >= (isize)document->undo_stack.size() - 16; --i)
            {
                GUI::text(app.gui, document->undo_stack[(usize)i]->label.c_str());
            }
        }

        static GUI::DockSpaceLayoutNodeDesc dock_leaf(GUI::id_t panel)
        {
            GUI::DockSpaceLayoutNodeDesc node;
            node.tabs.push_back(panel);
            node.selected_tab = panel;
            return node;
        }

        static GUI::DockSpaceLayoutNodeDesc dock_split(GUI::DockSplitAxis axis, f32 ratio, u32 child0, u32 child1)
        {
            GUI::DockSpaceLayoutNodeDesc node;
            node.split = true;
            node.split_axis = axis;
            node.split_ratio = ratio;
            node.child0 = child0;
            node.child1 = child1;
            return node;
        }

        static void set_default_dockspace_layout(
            App& app,
            GUI::ItemHandle dock_space,
            GUI::ItemHandle palette,
            GUI::ItemHandle preview,
            GUI::ItemHandle inspector,
            GUI::ItemHandle tree,
            GUI::ItemHandle history)
        {
            if(app.dockspace_layout_initialized || !dock_space.id || !palette.id || !preview.id || !inspector.id || !tree.id || !history.id)
            {
                return;
            }
            GUI::DockSpaceLayoutDesc layout;
            layout.root_node = 0;
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::x, 0.10f, 1, 2));
            layout.nodes.push_back(dock_leaf(palette.id));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::x, 0.72f, 3, 4));
            layout.nodes.push_back(dock_leaf(preview.id));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::y, 0.42f, 5, 6));
            layout.nodes.push_back(dock_leaf(tree.id));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::y, 0.62f, 7, 8));
            layout.nodes.push_back(dock_leaf(inspector.id));
            layout.nodes.push_back(dock_leaf(history.id));
            GUI::set_dockspace_layout(app.gui, dock_space, layout);
            app.dockspace_layout_initialized = true;
        }

        static GUI::ItemHandle begin_panel_scroll(App& app, const c8* label)
        {
            GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill());
            return GUI::begin_scroll_view(app.gui, label, GUI::Size());
        }

        static void end_panel_scroll(App& app)
        {
            GUI::end_scroll_view(app.gui);
        }

        static void draw_editor(App& app, FrameHandles& handles, const Float2U& surface_size)
        {
            sync_inspector(app);
            EditorDocument* document = app.service.active_document();
            bool can_save = document && document->asset;
            bool can_undo = document && !document->undo_stack.empty();
            bool can_redo = document && !document->redo_stack.empty();
            bool can_delete = can_remove_selected(document);
            GUI::LayoutDesc root_layout;
            root_layout.padding = GUI::EdgeInsets::all(0.0f);
            root_layout.gap = 0.0f;
            GUI::begin_v_layout(app.gui, "GUIEditor Root", RectF(0.0f, 0.0f, surface_size.x, surface_size.y), root_layout);
            GUI::begin_menu_bar(app.gui, "Main Menu");
            if(GUI::begin_menu(app.gui, "File"))
            {
                handles.new_document = GUI::menu_item(app.gui, "New", "Ctrl+N");
                GUI::menu_separator(app.gui);
                GUI::input_text(app.gui, "Open Path", app.open_path);
                handles.open_document = GUI::menu_item(app.gui, "Open", nullptr);
                GUI::input_text(app.gui, "Save Path", app.save_path);
                handles.save_document = GUI::menu_item(app.gui, "Save", "Ctrl+S", false, can_save);
                GUI::end_menu(app.gui);
            }
            if(GUI::begin_menu(app.gui, "Edit"))
            {
                handles.undo = GUI::menu_item(app.gui, "Undo", "Ctrl+Z", false, can_undo);
                handles.redo = GUI::menu_item(app.gui, "Redo", "Ctrl+Y", false, can_redo);
                GUI::menu_separator(app.gui);
                handles.remove_node_menu = GUI::menu_item(app.gui, "Delete Node", "Del", false, can_delete);
                GUI::end_menu(app.gui);
            }
            if(GUI::begin_menu(app.gui, "View"))
            {
                GUI::menu_item(app.gui, "Preview", nullptr, &app.show_preview);
                GUI::menu_item(app.gui, "Properties", nullptr, &app.show_properties);
                GUI::end_menu(app.gui);
            }
            GUI::end_menu_bar(app.gui);

            GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill());
            GUI::ItemHandle dock_space = GUI::begin_dock_space(app.gui, "GUIEditor DockSpace");

            GUI::ItemHandle palette_panel = GUI::begin_dock_panel(app.gui, "Node Palette");
            (void)begin_panel_scroll(app, "Node Palette Scroll");
            draw_palette_panel(app, handles);
            end_panel_scroll(app);
            GUI::end_dock_panel(app.gui);

            GUI::ItemHandle preview_panel;
            if(app.show_preview)
            {
                preview_panel = GUI::begin_dock_panel(app.gui, "Preview", &app.show_preview);
                handles.preview_drop_target = begin_panel_scroll(app, "Preview Scroll");
                register_tree_drop_target(app, handles.preview_drop_target);
                draw_preview_panel(app);
                end_panel_scroll(app);
                GUI::end_dock_panel(app.gui);
            }

            GUI::ItemHandle inspector_panel;
            if(app.show_properties)
            {
                inspector_panel = GUI::begin_dock_panel(app.gui, "Inspector", &app.show_properties);
                (void)begin_panel_scroll(app, "Inspector Scroll");
                draw_properties_panel(app, handles);
                end_panel_scroll(app);
                GUI::end_dock_panel(app.gui);
            }

            GUI::ItemHandle tree_panel;
            {
                tree_panel = GUI::begin_dock_panel(app.gui, "Widget Tree");
                (void)begin_panel_scroll(app, "Widget Tree Scroll");
                draw_tree_panel(app, handles);
                end_panel_scroll(app);
                GUI::end_dock_panel(app.gui);
            }

            GUI::ItemHandle history_panel = GUI::begin_dock_panel(app.gui, "History");
            (void)begin_panel_scroll(app, "History Scroll");
            draw_history_panel(app);
            end_panel_scroll(app);
            GUI::end_dock_panel(app.gui);

            set_default_dockspace_layout(app, dock_space, palette_panel, preview_panel, inspector_panel, tree_panel, history_panel);
            GUI::end_dock_space(app.gui);
            GUI::end_v_layout(app.gui);
        }

        static void select_node(App& app, const Guid& node)
        {
            EditorDocument* document = app.service.active_document();
            if(document && succeeded(app.service.set_selection(document->id, node)))
            {
                app.inspector_node = Guid(0, 0);
                sync_inspector(app);
            }
        }

        static bool resolve_drop_destination(EditorDocument& document, const NodeHandle& target, TreeDropPlacement placement, Guid& parent, usize& index)
        {
            if(placement == TreeDropPlacement::child || target.node == GA::get_root(document.asset.get()))
            {
                parent = target.node;
                index = USIZE_MAX;
                return true;
            }
            usize target_index = 0;
            usize count = 0;
            if(!node_order(&document, target.node, parent, target_index, count))
            {
                parent = target.node;
                index = USIZE_MAX;
                return true;
            }
            index = placement == TreeDropPlacement::after ? target_index + 1 : target_index;
            return true;
        }

        static usize adjust_move_index(EditorDocument& document, const Guid& node, const Guid& parent, usize index)
        {
            if(index == USIZE_MAX)
            {
                return index;
            }
            Guid old_parent;
            usize old_index = 0;
            usize old_count = 0;
            if(node_order(&document, node, old_parent, old_index, old_count) && old_parent == parent && index > old_index)
            {
                --index;
            }
            return index;
        }

        static bool process_palette_drop(App& app, EditorDocument& document, GUI::ItemHandle target, const Guid& parent, usize index)
        {
            const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, target, palette_payload_type());
            if(!payload)
            {
                return false;
            }
            const PalettePayload* data = payload->data_as<PalettePayload>();
            if(!data || (usize)data->type_index >= app.node_types.size())
            {
                app.service.last_status = "Invalid palette drag payload.";
                return true;
            }
            RV r = create_node_at(app.service, document, app.node_types[data->type_index], app.node_types[data->type_index].c_str(), parent, index);
            set_drop_status(app.service, r);
            app.inspector_node = Guid(0, 0);
            return true;
        }

        static bool process_tree_node_drop(App& app, EditorDocument& document, GUI::ItemHandle target, const Guid& parent, usize index)
        {
            const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, target, tree_node_payload_type());
            if(!payload)
            {
                return false;
            }
            const TreeNodePayload* data = payload->data_as<TreeNodePayload>();
            if(!data || data->node == Guid(0, 0))
            {
                app.service.last_status = "Invalid tree node drag payload.";
                return true;
            }
            if(data->node == parent)
            {
                app.service.last_status = "Cannot move a node under itself.";
                return true;
            }
            usize move_index = adjust_move_index(document, data->node, parent, index);
            RV r = move_node_at(app.service, document, data->node, parent, move_index);
            set_drop_status(app.service, r);
            app.inspector_node = Guid(0, 0);
            return true;
        }

        static bool process_drop_actions(App& app, const FrameHandles& handles, EditorDocument& document)
        {
            for(const NodeHandle& target : handles.tree_nodes)
            {
                TreeDropPlacement placement = tree_drop_placement(app, document, target);
                Guid parent;
                usize index = USIZE_MAX;
                if(!resolve_drop_destination(document, target, placement, parent, index))
                {
                    continue;
                }
                if(process_palette_drop(app, document, target.handle, parent, index))
                {
                    return true;
                }
                if(process_tree_node_drop(app, document, target.handle, parent, index))
                {
                    return true;
                }
            }
            if(handles.preview_drop_target.context)
            {
                Guid root = GA::get_root(document.asset.get());
                if(process_palette_drop(app, document, handles.preview_drop_target, root, USIZE_MAX))
                {
                    return true;
                }
                if(process_tree_node_drop(app, document, handles.preview_drop_target, root, USIZE_MAX))
                {
                    return true;
                }
            }
            return false;
        }

        static void process_actions(App& app, const FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document)
            {
                return;
            }
            bool new_requested = GUI::is_item_clicked(handles.new_document) ||
                shortcut_pressed(app, KeyCode::n, app.shortcut_new_down, true);
            bool open_requested = GUI::is_item_clicked(handles.open_document) ||
                shortcut_pressed(app, KeyCode::o, app.shortcut_open_down, true);
            bool save_requested = GUI::is_item_clicked(handles.save_document) ||
                shortcut_pressed(app, KeyCode::s, app.shortcut_save_down, true);
            bool undo_requested = GUI::is_item_clicked(handles.undo) ||
                shortcut_pressed(app, KeyCode::z, app.shortcut_undo_down, true);
            bool redo_requested = GUI::is_item_clicked(handles.redo) ||
                shortcut_pressed(app, KeyCode::y, app.shortcut_redo_down, true);
            bool delete_requested = GUI::is_item_clicked(handles.remove_node_menu) ||
                GUI::is_item_clicked(handles.remove_node) ||
                shortcut_pressed(app, KeyCode::del, app.shortcut_delete_down, false);
            bool move_up_requested = GUI::is_item_clicked(handles.move_up) ||
                shortcut_pressed(app, KeyCode::up, app.shortcut_move_up_down, true);
            bool move_down_requested = GUI::is_item_clicked(handles.move_down) ||
                shortcut_pressed(app, KeyCode::down, app.shortcut_move_down_down, true);

            if(new_requested)
            {
                (void)app.service.new_document();
                app.inspector_node = Guid(0, 0);
                document = app.service.active_document();
            }
            if(open_requested)
            {
                auto r = app.service.open_document(Path(app.open_path.c_str()));
                if(failed(r))
                {
                    app.service.last_status = explain(r.errcode());
                }
                app.inspector_node = Guid(0, 0);
                document = app.service.active_document();
            }
            if(save_requested && document && document->asset)
            {
                Path path(app.save_path.c_str());
                RV r = app.service.save_document(document->id, app.save_path.empty() ? nullptr : &path);
                if(failed(r))
                {
                    app.service.last_status = explain(r.errcode());
                }
            }
            if(undo_requested && document && !document->undo_stack.empty())
            {
                (void)app.service.undo(document->id);
                app.inspector_node = Guid(0, 0);
                document = app.service.active_document();
            }
            if(redo_requested && document && !document->redo_stack.empty())
            {
                (void)app.service.redo(document->id);
                app.inspector_node = Guid(0, 0);
                document = app.service.active_document();
            }
            if(!document || !document->asset)
            {
                return;
            }
            if(GUI::is_item_clicked(handles.new_node))
            {
                GUI::open_popup(app.gui, handles.new_node_popup);
            }
            for(const TypeItemHandle& item : handles.new_node_items)
            {
                if(GUI::is_item_clicked(item.handle) && (usize)item.type_index < app.node_types.size())
                {
                    Guid parent = document->selected_node != Guid(0, 0) ? document->selected_node : GA::get_root(document->asset.get());
                    (void)app.service.create_node(document->id, parent, app.node_types[item.type_index], app.node_types[item.type_index].c_str());
                    if(handles.new_node_popup.context)
                    {
                        GUI::close_popup(app.gui, handles.new_node_popup);
                    }
                    app.inspector_node = Guid(0, 0);
                    return;
                }
            }
            if(process_drop_actions(app, handles, *document))
            {
                return;
            }
            for(const NodeHandle& h : handles.tree_nodes)
            {
                if(GUI::is_item_clicked(h.handle))
                {
                    select_node(app, h.node);
                }
            }
            if(move_up_requested && can_move_selected(document, false))
            {
                usize index = 0;
                usize count = 0;
                if(selected_node_order(document, index, count))
                {
                    (void)app.service.reorder_node(document->id, document->selected_node, index - 1);
                    app.inspector_node = Guid(0, 0);
                }
            }
            if(move_down_requested && can_move_selected(document, true))
            {
                usize index = 0;
                usize count = 0;
                if(selected_node_order(document, index, count))
                {
                    (void)app.service.reorder_node(document->id, document->selected_node, index + 1);
                    app.inspector_node = Guid(0, 0);
                }
            }
            if(delete_requested && can_remove_selected(document))
            {
                (void)app.service.remove_node(document->id, document->selected_node);
                app.inspector_node = Guid(0, 0);
            }
            if(GUI::is_item_clicked(handles.apply_common))
            {
                (void)app.service.set_node_common(document->id, document->selected_node, app.edit_label.c_str(), app.edit_enabled, Name(app.edit_style.c_str()));
                app.inspector_node = Guid(0, 0);
            }
            if(GUI::is_item_clicked(handles.set_property))
            {
                Variant value = make_property_value(app.property_type, app.property_value);
                (void)app.service.set_node_property(document->id, document->selected_node, Name(app.property_key.c_str()), move(value));
            }
            if(GUI::is_item_clicked(handles.erase_property))
            {
                (void)app.service.erase_node_property(document->id, document->selected_node, Name(app.property_key.c_str()));
            }
        }

        static RV run_gui_mode()
        {
            lutry
            {
                luexp(add_modules({
                    module_variant_utils(),
                    module_hid(),
                    module_window(),
                    module_rhi(),
                    module_font(),
                    module_vg(),
                    GUI::module_gui(),
                    GUIWindow::module_gui_window(),
                    module_asset(),
                    GA::module_gui_asset(),
                    Frontend::module_frontend(),
                    module_vfs()
                }));
                luexp(init_modules());
                (void)mount_current_dir();
                set_log_to_platform_enabled(true);
                set_log_to_platform_verbosity(LogVerbosity::warning);

                App app;
                luexp(app.service.init());
                refresh_node_types(app);
                init_palette_icons(app.palette_icons);
                luset(app.window, Window::new_window("Luna GUI Editor"));
                Ref<RHI::IDevice> dev = RHI::get_main_device();
                u32 num_queues = dev->get_num_command_queues();
                for(u32 i = 0; i < num_queues; ++i)
                {
                    auto desc = dev->get_command_queue_desc(i);
                    if(desc.type == RHI::CommandQueueType::graphics)
                    {
                        app.queue = i;
                        break;
                    }
                }
                luset(app.swap_chain, dev->new_swap_chain(app.queue, app.window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
                luset(app.cmdbuf, dev->new_command_buffer(app.queue));
                app.gui = GUI::new_context(dev);
                luexp(GUI::register_font(app.gui, Name("default"), Font::get_default_font()));

                GUIWindow::GUIWindowInputAdapter input_adapter;
                input_adapter.window = app.window;
                input_adapter.gui = app.gui;
                GUIWindow::install_window_event_handler(&input_adapter);

                while(true)
                {
                    Window::poll_events();
                    if(app.window->is_closed())
                    {
                        break;
                    }
                    if(app.window->is_minimized())
                    {
                        sleep(100);
                        continue;
                    }
                    UInt2U fb_sz = app.window->get_framebuffer_size();
                    if(fb_sz.x && fb_sz.y && (fb_sz.x != app.width || fb_sz.y != app.height))
                    {
                        luexp(app.swap_chain->reset({fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true}));
                        app.width = fb_sz.x;
                        app.height = fb_sz.y;
                    }
                    UInt2U logical_sz = app.window->get_size();
                    GUI::FrameDesc frame;
                    frame.surface_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                    frame.framebuffer_size = fb_sz;
                    frame.dpi_scale = app.window->get_dpi_scale_factor();
                    frame.delta_time = 1.0f / 60.0f;
                    app.gui->begin_frame(frame);

                    FrameHandles handles;
                    draw_editor(app, handles, frame.surface_size);

                    lulet(desc, app.gui->end_build());
                    luexp(app.gui->submit(desc));
                    luexp(GUIWindow::update_text_input(&input_adapter));
                    process_actions(app, handles);

                    EditorDocument* document = app.service.active_document();
                    c8 title[256];
                    snprintf(title, sizeof(title), "Luna GUI Editor%s", document && document->dirty ? " *" : "");
                    luexp(app.window->set_title(title));

                    lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                    RHI::RenderPassDesc render_pass;
                    render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U(0.02f, 0.025f, 0.03f, 1.0f));
                    app.cmdbuf->begin_render_pass(render_pass);
                    app.cmdbuf->end_render_pass();
                    luexp(app.gui->render(app.cmdbuf, back_buffer));
                    app.cmdbuf->resource_barrier({}, {
                        {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                    });
                    luexp(app.cmdbuf->submit({}, {}, true));
                    app.cmdbuf->wait();
                    luexp(app.cmdbuf->reset());
                    luexp(app.swap_chain->present());
                }
                GUIWindow::uninstall_window_event_handler(&input_adapter);
            }
            lucatchret;
            return ok;
        }

        static RV run_cli_mode(bool mcp_enabled)
        {
            lutry
            {
                luexp(add_modules({module_variant_utils(), GUI::module_gui(), module_asset(), GA::module_gui_asset(), Frontend::module_frontend(), module_vfs()}));
                luexp(init_modules());
                (void)mount_current_dir();
                EditorService service;
                luexp(service.init());
                if(mcp_enabled)
                {
                    log_warning("GUIEditor", "MCP/TCP host is reserved for a later Network-focused pass.");
                }
                log_info("GUIEditor", "GUIEditor service initialized in CLI mode.");
            }
            lucatchret;
            return ok;
        }
    }
}

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init())
    {
        return -1;
    }
    Luna::GUIEditor::set_current_dir_to_process_path();
    bool cli = false;
    bool mcp = false;
    for(int i = 1; i < argc; ++i)
    {
        if(!strcmp(argv[i], "--cli"))
        {
            cli = true;
        }
        else if(!strcmp(argv[i], "--mcp"))
        {
            mcp = true;
        }
    }
    auto r = cli ? Luna::GUIEditor::run_cli_mode(mcp) : Luna::GUIEditor::run_gui_mode();
    if(failed(r))
    {
        Luna::log_error("GUIEditor", "%s", Luna::explain(r.errcode()));
    }
    Luna::close();
    return failed(r) ? -1 : 0;
}
