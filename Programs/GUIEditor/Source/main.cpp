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
#include <Luna/GUICore/GUICore.hpp>
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
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
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
        struct CoreNodeHandle
        {
            Guid node;
            GUICore::ElementHandle handle;
        };

        struct CoreTypeItemHandle
        {
            u32 type_index = 0;
            GUICore::ElementHandle handle;
        };

        struct PropertyEditHandle
        {
            u64 document_id = 0;
            Guid node;
            Name key;
            GA::NodePropertyKind kind = GA::NodePropertyKind::string;
            GUICore::ElementHandle core_handle;
            GUICore::ElementHandle core_extra_handle;
            Variant original_value;
            String string_value;
            Vector<String> enum_items;
            bool bool_value = false;
            i32 int_value = 0;
            f32 values[5] = {};
        };

        struct FrameHandles
        {
            GUICore::ElementHandle new_document;
            GUICore::ElementHandle open_document;
            GUICore::ElementHandle save_document;
            GUICore::ElementHandle undo;
            GUICore::ElementHandle redo;
            GUICore::ElementHandle remove_node_menu;
            GUICore::ElementHandle preview_canvas;
            GUICore::ElementHandle core_new_node;
            GUICore::ElementHandle core_new_node_popup;
            GUICore::ElementHandle core_node_context_popup;
            GUICore::ElementHandle core_node_context_move_up;
            GUICore::ElementHandle core_node_context_move_down;
            GUICore::ElementHandle core_node_context_delete;
            GUICore::ElementHandle core_common_label;
            GUICore::ElementHandle core_common_style;
            GUICore::ElementHandle core_common_enabled;
            GUICore::ElementHandle core_set_property;
            GUICore::ElementHandle core_erase_property;
            Vector<CoreNodeHandle> core_tree_nodes;
            Vector<CoreTypeItemHandle> core_new_node_items;
            Vector<UniquePtr<PropertyEditHandle>> property_edits;
        };

        struct App
        {
            Ref<Window::IWindow> window;
            Ref<RHI::ISwapChain> swap_chain;
            Ref<RHI::ICommandBuffer> cmdbuf;
            Ref<GUICore::IContext> editor_core;
            Ref<GUICore::IRenderer> editor_renderer;
            Ref<GUICore::IContext> preview_core;
            Ref<GUICore::IRenderer> preview_renderer;
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
            bool edit_label_was_focused = false;
            bool edit_style_was_focused = false;
            Guid tree_context_node = Guid(0, 0);
            Float2U tree_context_position = Float2U(0.0f, 0.0f);
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
            String preview_error;
        };

        static u64 hash_bytes(const void* data, usize size, u64 h = 14695981039346656037ull)
        {
            const byte_t* bytes = (const byte_t*)data;
            for(usize i = 0; i < size; ++i)
            {
                h ^= (u64)bytes[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        static u64 hash_cstr(const c8* text, u64 h = 14695981039346656037ull)
        {
            if(!text)
            {
                return h;
            }
            while(*text)
            {
                h ^= (u64)(byte_t)*text;
                h *= 1099511628211ull;
                ++text;
            }
            return h;
        }

        static GUICore::id_t core_id(const c8* scope, u64 value)
        {
            u64 h = hash_cstr(scope);
            return hash_bytes(&value, sizeof(value), h);
        }

        static GUICore::id_t core_id(const c8* scope, const Guid& value)
        {
            u64 h = hash_cstr(scope);
            h = hash_bytes(&value.high, sizeof(value.high), h);
            return hash_bytes(&value.low, sizeof(value.low), h);
        }

        static GUICore::id_t core_id(const c8* scope, const Name& value)
        {
            return hash_cstr(value.c_str(), hash_cstr(scope));
        }

        static GUICore::id_t core_id(const c8* scope, const Guid& value, const Name& key)
        {
            u64 h = core_id(scope, value);
            return hash_cstr(key.c_str(), h);
        }

        static GUICore::id_t core_derived_id(GUICore::id_t id, const c8* salt)
        {
            return hash_cstr(salt, id);
        }

        static GUICore::LayoutConfig core_layout_pixels(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            if(width > 0.0f)
            {
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = width;
            }
            else
            {
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
            }
            if(height > 0.0f)
            {
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
            }
            return layout;
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
                return VariantUtils::write_json(value, false);
            }
        }

        static const Variant& node_property_or_default(const GA::Node& node, const GA::NodePropertyDesc& desc)
        {
            const Variant& value = node.properties[desc.key];
            return value.valid() ? value : desc.default_value;
        }

        static Variant make_size_variant(f32 width, f32 height)
        {
            Variant r(VariantType::object);
            r[Name("width")] = (f64)width;
            r[Name("height")] = (f64)height;
            return r;
        }

        static Variant make_edge_insets_variant(f32 left, f32 top, f32 right, f32 bottom)
        {
            Variant r(VariantType::object);
            r[Name("left")] = (f64)left;
            r[Name("top")] = (f64)top;
            r[Name("right")] = (f64)right;
            r[Name("bottom")] = (f64)bottom;
            return r;
        }

        static bool set_property_if_changed(App& app, EditorDocument& document, const Guid& node, const Name& key, Variant&& value)
        {
            RV r = app.service.set_node_property(document.id, node, key, move(value));
            if(failed(r))
            {
                app.service.last_status = explain(r.errcode());
                return false;
            }
            return true;
        }

        static void split_csv(const String& text, Vector<String>& out_items)
        {
            out_items.clear();
            usize start = 0;
            for(usize i = 0; i <= text.size(); ++i)
            {
                if(i == text.size() || text[i] == ',')
                {
                    usize end = i;
                    while(start < end && (text[start] == ' ' || text[start] == '\t'))
                    {
                        ++start;
                    }
                    while(end > start && (text[end - 1] == ' ' || text[end - 1] == '\t'))
                    {
                        --end;
                    }
                    out_items.push_back(String(text.data() + start, text.data() + end));
                    start = i + 1;
                }
            }
        }

        static String join_string_array(const Variant& value)
        {
            String r;
            bool first = true;
            for(const Variant& item : value.values())
            {
                if(!first)
                {
                    r.append(", ");
                }
                r.append(item.c_str(""));
                first = false;
            }
            return r;
        }

        static Variant parse_string_array(const String& text)
        {
            Vector<String> strings;
            split_csv(text, strings);
            Variant r(VariantType::array);
            for(const String& item : strings)
            {
                if(!item.empty())
                {
                    r.push_back(item.c_str());
                }
            }
            return r;
        }

        static String join_number_array(const Variant& value)
        {
            String r;
            bool first = true;
            c8 buf[64];
            for(const Variant& item : value.values())
            {
                if(!first)
                {
                    r.append(", ");
                }
                snprintf(buf, sizeof(buf), "%.3f", item.fnum(0.0));
                r.append(buf);
                first = false;
            }
            return r;
        }

        static Variant parse_number_array(const String& text)
        {
            Vector<String> parts;
            split_csv(text, parts);
            Variant r(VariantType::array);
            for(const String& part : parts)
            {
                if(part.empty())
                {
                    continue;
                }
                c8* end = nullptr;
                f64 value = strtod(part.c_str(), &end);
                if(end != part.c_str())
                {
                    r.push_back(value);
                }
            }
            return r;
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

        static bool can_remove_node(EditorDocument* document, const Guid& node)
        {
            return document && document->asset && node != Guid(0, 0) &&
                node != GA::get_root(document->asset.get()) &&
                GA::find_node(document->asset.get(), node);
        }

        static bool can_remove_selected(EditorDocument* document)
        {
            return document && can_remove_node(document, document->selected_node);
        }

        static bool can_move_node(EditorDocument* document, const Guid& node, bool down)
        {
            usize index = 0;
            usize count = 0;
            Guid parent;
            if(!node_order(document, node, parent, index, count))
            {
                return false;
            }
            return down ? index + 1 < count : index > 0;
        }

        static bool can_move_selected(EditorDocument* document, bool down)
        {
            return document && can_move_node(document, document->selected_node, down);
        }

        static bool key_edge(bool down, bool& previous)
        {
            bool pressed = down && !previous;
            previous = down;
            return pressed;
        }

        static bool shortcut_pressed(App& app, KeyCode key, bool& previous, bool shortcut_modifier, bool shift_modifier = false)
        {
            if((app.editor_core && app.editor_core->get_text_input_state().active) ||
                (app.preview_core && app.preview_core->get_text_input_state().active))
            {
                return key_edge(false, previous);
            }
            bool core_down = false;
            bool core_shortcut_down = false;
            bool core_shift_down = false;
            if(app.editor_core)
            {
                GUICore::KeyModifierFlag core_modifiers = app.editor_core->get_key_modifiers();
                core_shortcut_down = test_flags(core_modifiers, GUICore::KeyModifierFlag::ctrl) ||
                    test_flags(core_modifiers, GUICore::KeyModifierFlag::system);
                core_shift_down = test_flags(core_modifiers, GUICore::KeyModifierFlag::shift);
                core_down = app.editor_core->is_key_down(key);
            }
            bool shortcut_down = core_shortcut_down;
            bool shift_down = core_shift_down;
            bool down = core_down;
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
            app.edit_label_was_focused = false;
            app.edit_style_was_focused = false;
            Ref<GA::Node> node = GA::find_node(document->asset.get(), app.inspector_node);
            if(node)
            {
                app.edit_label = node->label;
                app.edit_style = node->style.c_str();
                app.edit_enabled = node->enabled;
            }
        }

        static void build_core_node_tree(App& app, FrameHandles& handles, EditorDocument& document,
            const Guid& id, u32 indent_depth)
        {
            Ref<GA::Node> node = GA::find_node(document.asset.get(), id);
            if(!node)
            {
                return;
            }
            c8 label[256];
            snprintf(label, sizeof(label), "%s  [%s]", node->label.empty() ? "(unnamed)" : node->label.c_str(), node->type.c_str());
            GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::none;
            if(GA::get_child_count(node.get()) == 0)
            {
                flags |= GUI::TreeNodeFlag::leaf;
            }
            if(id == document.selected_node)
            {
                flags |= GUI::TreeNodeFlag::selected;
            }
            GUICore::ElementHandle handle;
            GUI::DisclosureDesc disclosure_desc;
            disclosure_desc.default_open = true;
            bool open = GUI::tree_node(app.editor_core.get(), core_id("gui_editor.tree_node", id), label, flags,
                indent_depth, core_layout_pixels(0.0f, 26.0f), disclosure_desc, &handle);
            handles.core_tree_nodes.push_back({id, handle});
            if(open && GA::get_child_count(node.get()) > 0)
            {
                for(const Guid& child : GA::get_children(node.get()))
                {
                    build_core_node_tree(app, handles, document, child, indent_depth + 1);
                }
            }
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

        static PropertyEditHandle& add_property_edit(FrameHandles& handles, u64 document_id, const GA::Node& node, const GA::NodePropertyDesc& desc)
        {
            UniquePtr<PropertyEditHandle> edit(memnew<PropertyEditHandle>());
            edit->document_id = document_id;
            edit->node = node.id;
            edit->key = desc.key;
            edit->kind = desc.kind;
            handles.property_edits.push_back(move(edit));
            return *handles.property_edits.back().get();
        }

        static GUICore::LayoutConfig core_text_layout();

        static GUICore::LayoutConfig core_inspector_row_layout(f32 height = 30.0f)
        {
            return core_layout_pixels(0.0f, height);
        }

        static GUICore::LayoutConfig core_inspector_label_layout()
        {
            return core_layout_pixels(112.0f, 28.0f);
        }

        static GUICore::LayoutConfig core_inspector_field_layout(f32 height = 28.0f)
        {
            return core_layout_pixels(0.0f, height);
        }

        static void core_inspector_spacer(GUICore::IContext* context, GUICore::id_t id, f32 height = 8.0f)
        {
            GUI::text(context, id, "", core_layout_pixels(0.0f, height));
        }

        static void core_inspector_section(GUICore::IContext* context, GUICore::id_t id, const c8* title, bool first = false)
        {
            if(!first)
            {
                core_inspector_spacer(context, core_derived_id(id, "spacer"));
            }
            GUI::text(context, core_derived_id(id, "title"), title, core_layout_pixels(0.0f, 28.0f));
            GUI::menu_separator(context, core_derived_id(id, "separator"), core_layout_pixels(0.0f, 6.0f));
        }

        static void core_inspector_subsection(GUICore::IContext* context, GUICore::id_t id, const c8* title)
        {
            GUI::text(context, id, title, core_layout_pixels(0.0f, 24.0f));
        }

        static GUICore::ElementHandle core_labeled_input_text(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, String& value)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout());
            GUI::text(context, core_derived_id(id, "label"), label, core_inspector_label_layout());
            GUICore::ElementHandle input = GUI::input_text(context, core_derived_id(id, "input"), value,
                core_inspector_field_layout());
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return input;
        }

        static GUICore::ElementHandle core_labeled_checkbox(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, bool* value)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout());
            GUI::text(context, core_derived_id(id, "label"), label, core_inspector_label_layout());
            GUICore::ElementHandle checkbox = GUI::checkbox(context, core_derived_id(id, "checkbox"), "", value,
                core_inspector_field_layout());
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return checkbox;
        }

        static GUICore::ElementHandle core_labeled_drag_int(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout());
            GUI::text(context, core_derived_id(id, "label"), label, core_inspector_label_layout());
            GUI::DragDesc drag_desc;
            drag_desc.speed = speed;
            GUICore::ElementHandle drag = GUI::drag_int(context, core_derived_id(id, "drag"), value,
                min_value, max_value, core_inspector_field_layout(), drag_desc);
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return drag;
        }

        static GUICore::ElementHandle core_labeled_drag_float(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout());
            GUI::text(context, core_derived_id(id, "label"), label, core_inspector_label_layout());
            GUI::DragDesc drag_desc;
            drag_desc.speed = speed;
            GUICore::ElementHandle drag = GUI::drag_float(context, core_derived_id(id, "drag"), value,
                min_value, max_value, core_inspector_field_layout(), drag_desc);
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return drag;
        }

        static GUICore::ElementHandle core_labeled_drag_float_n(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, u32 count, f32 speed, f32 min_value, f32 max_value)
        {
            f32 height = max(28.0f, (f32)count * 26.0f);
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout(height + 2.0f));
            GUI::text(context, core_derived_id(id, "label"), label, core_layout_pixels(112.0f, height));
            GUICore::ElementHandle drag;
            GUI::DragDesc drag_desc;
            drag_desc.speed = speed;
            if(count == 2)
            {
                drag = GUI::drag_float2(context, core_derived_id(id, "drag"), value, min_value, max_value,
                    core_inspector_field_layout(height), drag_desc);
            }
            else if(count == 3)
            {
                drag = GUI::drag_float3(context, core_derived_id(id, "drag"), value, min_value, max_value,
                    core_inspector_field_layout(height), drag_desc);
            }
            else
            {
                drag = GUI::drag_float4(context, core_derived_id(id, "drag"), value, min_value, max_value,
                    core_inspector_field_layout(height), drag_desc);
            }
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return drag;
        }

        static GUICore::ElementHandle core_labeled_combo(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, Span<const c8*> items)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, core_inspector_row_layout());
            GUI::text(context, core_derived_id(id, "label"), label, core_inspector_label_layout());
            GUICore::ElementHandle combo = GUI::combo(context, core_derived_id(id, "combo"), label, value, items,
                core_inspector_field_layout());
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return combo;
        }

        static void build_core_schema_property(App& app, FrameHandles& handles, EditorDocument& document,
            const GA::Node& node, const GA::NodePropertyDesc& desc)
        {
            const c8* label = desc.display_name.empty() ? desc.key.c_str() : desc.display_name.c_str();
            const Variant& value = node_property_or_default(node, desc);
            PropertyEditHandle& edit = add_property_edit(handles, document.id, node, desc);
            edit.original_value = value;
            GUICore::id_t id = core_id("gui_editor.inspector.property", node.id, desc.key);
            switch(desc.kind)
            {
            case GA::NodePropertyKind::string:
            case GA::NodePropertyKind::asset:
                edit.string_value = value.c_str("");
                edit.core_handle = core_labeled_input_text(app.editor_core.get(), id, label, edit.string_value);
                break;
            case GA::NodePropertyKind::boolean:
                edit.bool_value = value.boolean(false);
                edit.core_handle = core_labeled_checkbox(app.editor_core.get(), id, label, &edit.bool_value);
                break;
            case GA::NodePropertyKind::integer:
                edit.int_value = (i32)value.inum((i64)desc.default_value.inum(0));
                edit.core_handle = core_labeled_drag_int(app.editor_core.get(), id, label, &edit.int_value,
                    desc.speed, (i32)desc.min_value, (i32)desc.max_value);
                break;
            case GA::NodePropertyKind::number:
                edit.values[0] = (f32)value.fnum(desc.default_value.fnum(0.0));
                edit.core_handle = core_labeled_drag_float(app.editor_core.get(), id, label, edit.values,
                    desc.speed, (f32)desc.min_value, (f32)desc.max_value);
                break;
            case GA::NodePropertyKind::enum_string:
            {
                Vector<const c8*> items;
                items.reserve(desc.enum_items.size());
                for(const String& item : desc.enum_items)
                {
                    edit.enum_items.push_back(item);
                    items.push_back(item.c_str());
                }
                if(items.empty())
                {
                    edit.string_value = value.c_str("");
                    edit.core_handle = core_labeled_input_text(app.editor_core.get(), id, label, edit.string_value);
                    break;
                }
                const c8* current_text = value.c_str(desc.default_value.c_str(""));
                edit.int_value = 0;
                for(usize i = 0; i < items.size(); ++i)
                {
                    if(!strcmp(current_text, items[i]))
                    {
                        edit.int_value = (i32)i;
                        break;
                    }
                }
                edit.core_handle = core_labeled_combo(app.editor_core.get(), id, label, &edit.int_value,
                    Span<const c8*>(items.data(), items.size()));
                break;
            }
            case GA::NodePropertyKind::size:
                edit.values[0] = (f32)value[Name("width")].fnum(desc.default_value[Name("width")].fnum(0.0));
                edit.values[1] = (f32)value[Name("height")].fnum(desc.default_value[Name("height")].fnum(0.0));
                edit.core_handle = core_labeled_drag_float_n(app.editor_core.get(), id, label, edit.values, 2,
                    1.0f, 0.0f, 8192.0f);
                break;
            case GA::NodePropertyKind::edge_insets:
                edit.values[0] = (f32)value[Name("left")].fnum(desc.default_value[Name("left")].fnum(0.0));
                edit.values[1] = (f32)value[Name("top")].fnum(desc.default_value[Name("top")].fnum(0.0));
                edit.values[2] = (f32)value[Name("right")].fnum(desc.default_value[Name("right")].fnum(0.0));
                edit.values[3] = (f32)value[Name("bottom")].fnum(desc.default_value[Name("bottom")].fnum(0.0));
                edit.core_handle = core_labeled_drag_float_n(app.editor_core.get(), id, label, edit.values, 4,
                    1.0f, 0.0f, 512.0f);
                break;
            case GA::NodePropertyKind::layout_desc:
                edit.values[0] = (f32)value[Name("padding")][Name("left")].fnum(desc.default_value[Name("padding")][Name("left")].fnum(0.0));
                edit.values[1] = (f32)value[Name("padding")][Name("top")].fnum(desc.default_value[Name("padding")][Name("top")].fnum(0.0));
                edit.values[2] = (f32)value[Name("padding")][Name("right")].fnum(desc.default_value[Name("padding")][Name("right")].fnum(0.0));
                edit.values[3] = (f32)value[Name("padding")][Name("bottom")].fnum(desc.default_value[Name("padding")][Name("bottom")].fnum(0.0));
                edit.values[4] = (f32)value[Name("gap")].fnum(desc.default_value[Name("gap")].fnum(0.0));
                edit.core_handle = core_labeled_drag_float_n(app.editor_core.get(), id, "Padding", edit.values, 4,
                    1.0f, 0.0f, 512.0f);
                edit.core_extra_handle = core_labeled_drag_float(app.editor_core.get(), core_derived_id(id, "gap"),
                    "Gap", edit.values + 4, 1.0f, 0.0f, 512.0f);
                break;
            case GA::NodePropertyKind::string_array:
                edit.string_value = join_string_array(value);
                edit.core_handle = core_labeled_input_text(app.editor_core.get(), id, label, edit.string_value);
                break;
            case GA::NodePropertyKind::number_array:
                edit.string_value = join_number_array(value);
                edit.core_handle = core_labeled_input_text(app.editor_core.get(), id, label, edit.string_value);
                break;
            }
        }

        static void build_core_schema_properties(App& app, FrameHandles& handles, EditorDocument& document,
            const GA::Node& node)
        {
            R<GA::NodeTypeDesc> desc_result = GA::get_node_type(node.type);
            if(failed(desc_result))
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.schema.error", node.id),
                    explain(desc_result.errcode()), core_text_layout());
                return;
            }
            const GA::NodeTypeDesc& desc = desc_result.get();
            if(desc.properties.empty())
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.schema.empty", node.id),
                    "This node type does not declare editable properties.", core_text_layout());
                return;
            }
            String current_category;
            for(const GA::NodePropertyDesc& property_desc : desc.properties)
            {
                const String& category = property_desc.category;
                if(strcmp(category.c_str(), current_category.c_str()))
                {
                    current_category = category;
                    core_inspector_subsection(app.editor_core.get(),
                        core_id("gui_editor.inspector.category", node.id, Name(current_category.c_str())),
                        current_category.empty() ? "Properties" : current_category.c_str());
                }
                build_core_schema_property(app, handles, document, node, property_desc);
            }
        }

        static void build_core_raw_properties(App& app, const GA::Node& node)
        {
            if(node.properties.type() != VariantType::object)
            {
                return;
            }
            u64 row_index = 0;
            for(const auto& kv : node.properties.key_values())
            {
                GUICore::id_t id = core_id("gui_editor.inspector.raw", row_index++);
                GUICore::ElementHandle row = GUI::begin_h_layout(app.editor_core.get(), id, kv.first.c_str(),
                    core_inspector_row_layout());
                GUI::text(app.editor_core.get(), core_derived_id(id, "key"), kv.first.c_str(), core_inspector_label_layout());
                String value_text = variant_to_text(kv.second);
                GUI::text(app.editor_core.get(), core_derived_id(id, "value"), value_text.c_str(), core_inspector_field_layout());
                GUICore::FlexLayoutDesc desc;
                desc.main_axis_gap = 8.0f;
                GUI::end_h_layout(app.editor_core.get(), row, desc);
            }
        }

        static void build_core_properties_panel(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.empty", 0), "No document.", core_text_layout());
            }
            else
            {
                Ref<GA::Node> node = GA::find_node(document->asset.get(), document->selected_node);
                if(!node)
                {
                    GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.no_selection", 0),
                        "No node selected.", core_text_layout());
                }
                else
                {
                    core_inspector_section(app.editor_core.get(), core_id("gui_editor.inspector.identity", 0), "Identity", true);
                    c8 id_text[128];
                    snprintf(id_text, sizeof(id_text), "ID: %s", guid_to_text(node->id).c_str());
                    GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.identity.id", node->id), id_text,
                        core_text_layout());
                    c8 type_text[160];
                    snprintf(type_text, sizeof(type_text), "Type: %s", node->type.c_str());
                    GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.identity.type", node->id), type_text,
                        core_text_layout());
                    handles.core_common_label = core_labeled_input_text(app.editor_core.get(),
                        core_id("gui_editor.inspector.common.label", node->id), "Label", app.edit_label);

                    core_inspector_section(app.editor_core.get(), core_id("gui_editor.inspector.common", 0), "Common");
                    handles.core_common_style = core_labeled_input_text(app.editor_core.get(),
                        core_id("gui_editor.inspector.common.style", node->id), "Style", app.edit_style);
                    handles.core_common_enabled = core_labeled_checkbox(app.editor_core.get(),
                        core_id("gui_editor.inspector.common.enabled", node->id), "Enabled", &app.edit_enabled);

                    core_inspector_section(app.editor_core.get(), core_id("gui_editor.inspector.widget_properties", 0),
                        "Widget Properties");
                    build_core_schema_properties(app, handles, *document, *node.get());

                    core_inspector_section(app.editor_core.get(), core_id("gui_editor.inspector.raw_properties", 0),
                        "Raw Properties");
                    build_core_raw_properties(app, *node.get());

                    core_inspector_subsection(app.editor_core.get(), core_id("gui_editor.inspector.manual_edit", 0),
                        "Manual Edit");
                    core_labeled_input_text(app.editor_core.get(), core_id("gui_editor.inspector.manual.key", 0),
                        "Property Key", app.property_key);
                    const c8* type_items[] = {"String", "Number", "Integer", "Boolean"};
                    GUICore::ElementHandle type_row = GUI::begin_h_layout(app.editor_core.get(),
                        core_id("gui_editor.inspector.manual.type_row", 0), "Property Type", core_inspector_row_layout());
                    GUI::text(app.editor_core.get(), core_id("gui_editor.inspector.manual.type_label", 0),
                        "Property Type", core_inspector_label_layout());
                    GUI::button_group(app.editor_core.get(), core_id("gui_editor.inspector.manual.type", 0),
                        Span<const c8*>(type_items, 4), &app.property_type, core_inspector_field_layout());
                    GUICore::FlexLayoutDesc type_desc;
                    type_desc.main_axis_gap = 8.0f;
                    GUI::end_h_layout(app.editor_core.get(), type_row, type_desc);
                    core_labeled_input_text(app.editor_core.get(), core_id("gui_editor.inspector.manual.value", 0),
                        "Property Value", app.property_value);
                    GUICore::ElementHandle actions = GUI::begin_h_layout(app.editor_core.get(),
                        core_id("gui_editor.inspector.manual.actions", 0), "Property Actions", core_inspector_row_layout());
                    handles.core_set_property = GUI::text_button(app.editor_core.get(),
                        core_id("gui_editor.inspector.manual.set", 0), "Set Property", core_layout_pixels(120.0f, 28.0f));
                    handles.core_erase_property = GUI::text_button(app.editor_core.get(),
                        core_id("gui_editor.inspector.manual.erase", 0), "Erase Property", core_layout_pixels(120.0f, 28.0f));
                    GUICore::FlexLayoutDesc action_desc;
                    action_desc.main_axis_gap = 8.0f;
                    GUI::end_h_layout(app.editor_core.get(), actions, action_desc);
                }
            }
        }

        static void draw_preview_panel(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.preview.empty", 0), "No document.", core_text_layout());
                return;
            }
            GUI::text(app.editor_core.get(), core_id("gui_editor.preview.title", 0), "Live Preview", core_text_layout());
            GUI::text(app.editor_core.get(), core_id("gui_editor.preview.help", 0),
                "Preview of the active GUI document.", core_text_layout());
            if(!app.preview_error.empty())
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.preview.error", 0), app.preview_error.c_str(), core_text_layout());
            }
            GUICore::LayoutConfig canvas_layout;
            canvas_layout.width.kind = GUICore::SizeKind::percent;
            canvas_layout.width.value = 1.0f;
            canvas_layout.height.kind = GUICore::SizeKind::percent;
            canvas_layout.height.value = 1.0f;
            canvas_layout.flex_grow = 1.0f;
            handles.preview_canvas = GUI::hit_box(app.editor_core.get(), core_id("gui_editor.preview.canvas", 0),
                canvas_layout);
        }

        static GUICore::LayoutConfig core_text_layout()
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = 24.0f;
            return layout;
        }

        static void build_core_palette_panel(App& app)
        {
            GUI::text(app.editor_core.get(), core_id("gui_editor.palette.title", 0), "Palette", core_text_layout());

            usize visible_count = 0;
            for(const Name& type : app.node_types)
            {
                if(is_core_palette_type(type))
                {
                    ++visible_count;
                }
            }
            f32 grid_height = max(38.0f, (f32)((visible_count + 1) / 2) * 44.0f);
            GUICore::ElementHandle grid = GUI::begin_grid_layout(app.editor_core.get(), core_id("gui_editor.palette.grid", 0),
                "Palette Grid", core_layout_pixels(0.0f, grid_height));
            for(usize i = 0; i < app.node_types.size(); ++i)
            {
                if(!is_core_palette_type(app.node_types[i]))
                {
                    continue;
                }
                GUICore::ShapeDesc& icon = palette_icon(app.palette_icons, app.node_types[i]);
                GUI::ShapeButtonDesc button_desc;
                button_desc.padding = 8.0f;
                GUI::shape_button(app.editor_core.get(),
                    core_id("gui_editor.palette.item", (u64)i), app.node_types[i].c_str(), icon,
                    core_layout_pixels(42.0f, 38.0f), button_desc);
            }
            GUICore::GridLayoutDesc grid_desc;
            grid_desc.mode = GUICore::GridLayoutMode::fixed_column_count;
            grid_desc.column_count = 2;
            grid_desc.cell_size = Float2U(42.0f, 38.0f);
            grid_desc.gap = Float2U(6.0f, 6.0f);
            GUI::end_grid_layout(app.editor_core.get(), grid, grid_desc);

        }

        static void build_core_tree_panel_base(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.tree.empty", 0), "No document.", core_text_layout());
            }
            else
            {
                c8 info[256];
                snprintf(info, sizeof(info), "Document %llu%s", (unsigned long long)document->id, document->dirty ? " *" : "");
                GUI::text(app.editor_core.get(), core_id("gui_editor.tree.info", 0), info, core_text_layout());
                GUICore::ElementHandle actions = GUI::begin_h_layout(app.editor_core.get(),
                    core_id("gui_editor.tree.actions", 0), "Tree Actions", core_layout_pixels(0.0f, 30.0f));
                GUI::ButtonDesc button_desc;
                button_desc.enabled = !app.node_types.empty();
                handles.core_new_node = GUI::text_button(app.editor_core.get(), core_id("gui_editor.tree.new", 0),
                    "New", core_layout_pixels(78.0f, 28.0f), button_desc);
                GUICore::FlexLayoutDesc action_desc;
                action_desc.main_axis_gap = 4.0f;
                GUI::end_h_layout(app.editor_core.get(), actions, action_desc);
                build_core_node_tree(app, handles, *document, GA::get_root(document->asset.get()), 0);
            }
        }

        static void build_core_tree_popups(App& app, FrameHandles& handles)
        {
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                return;
            }

            GUICore::id_t new_popup_id = core_id("gui_editor.tree.new_popup", 0);
            RectF new_rect = GUI::get_item_rect(app.editor_core.get(), handles.core_new_node);
            GUI::PopupDesc popup_desc;
            popup_desc.position = Float2U(new_rect.offset_x, new_rect.offset_y + new_rect.height + 4.0f);
            popup_desc.layout = core_layout_pixels(220.0f, 0.0f);
            if(GUI::begin_popup(app.editor_core.get(), new_popup_id, popup_desc, &handles.core_new_node_popup))
            {
                for(usize i = 0; i < app.node_types.size(); ++i)
                {
                    GUICore::ElementHandle item = GUI::menu_item(app.editor_core.get(),
                        core_id("gui_editor.tree.new_item", (u64)i), app.node_types[i].c_str());
                    handles.core_new_node_items.push_back({(u32)i, item});
                }
                lupanic_if_failed(GUI::end_popup(app.editor_core.get(), handles.core_new_node_popup,
                    RectF(0.0f, 0.0f, 220.0f, max(28.0f, (f32)app.node_types.size() * 28.0f))));
            }

            GUICore::id_t context_popup_id = core_id("gui_editor.tree.context_popup", 0);
            GUI::PopupDesc context_desc;
            context_desc.position = app.tree_context_position;
            context_desc.layout = core_layout_pixels(180.0f, 0.0f);
            if(GUI::begin_popup(app.editor_core.get(), context_popup_id, context_desc, &handles.core_node_context_popup))
            {
                GUI::MenuItemDesc move_up_desc;
                move_up_desc.enabled = can_move_node(document, app.tree_context_node, false);
                handles.core_node_context_move_up = GUI::menu_item(app.editor_core.get(),
                    core_id("gui_editor.tree.context.up", 0), "Move Up", false, move_up_desc);
                GUI::MenuItemDesc move_down_desc;
                move_down_desc.enabled = can_move_node(document, app.tree_context_node, true);
                handles.core_node_context_move_down = GUI::menu_item(app.editor_core.get(),
                    core_id("gui_editor.tree.context.down", 0), "Move Down", false, move_down_desc);
                GUI::menu_separator(app.editor_core.get(), core_id("gui_editor.tree.context.sep", 0));
                GUI::MenuItemDesc delete_desc;
                delete_desc.enabled = can_remove_node(document, app.tree_context_node);
                handles.core_node_context_delete = GUI::menu_item(app.editor_core.get(),
                    core_id("gui_editor.tree.context.delete", 0), "Delete", false, delete_desc);
                lupanic_if_failed(GUI::end_popup(app.editor_core.get(), handles.core_node_context_popup,
                    RectF(0.0f, 0.0f, 180.0f, 96.0f)));
            }
        }

        static RV render_core_context(GUICore::IContext* context, GUICore::IRenderer* renderer,
            RHI::ICommandBuffer* cmdbuf, RHI::ITexture* back_buffer)
        {
            if(!context || !renderer || !cmdbuf || !back_buffer)
            {
                return ok;
            }
            lutry
            {
                luexp(renderer->prepare(context, cmdbuf, back_buffer));
                RHI::RenderPassDesc render_pass;
                render_pass.color_attachments[0] = RHI::ColorAttachment(
                    back_buffer, RHI::LoadOp::load, RHI::StoreOp::store);
                cmdbuf->begin_render_pass(render_pass);
                renderer->render(cmdbuf);
                cmdbuf->end_render_pass();
            }
            lucatchret;
            return ok;
        }

        static void build_core_history_panel(App& app)
        {
            EditorDocument* document = app.service.active_document();
            if(!document)
            {
                GUI::text(app.editor_core.get(), core_id("gui_editor.history.empty", 0), "No document.", core_text_layout());
            }
            else
            {
                c8 status[256];
                snprintf(status, sizeof(status), "Undo %llu | Redo %llu",
                    (unsigned long long)document->undo_stack.size(), (unsigned long long)document->redo_stack.size());
                GUI::text(app.editor_core.get(), core_id("gui_editor.history.status", 0), status, core_text_layout());
                u64 next_id = 1;
                if(!app.service.last_status.empty())
                {
                    GUI::text(app.editor_core.get(), core_id("gui_editor.history.status_line", next_id++),
                        app.service.last_status.c_str(), core_text_layout());
                }
                for(isize i = (isize)document->undo_stack.size() - 1; i >= 0 && i >= (isize)document->undo_stack.size() - 16; --i)
                {
                    GUI::text(app.editor_core.get(), core_id("gui_editor.history.undo", (u64)i),
                        document->undo_stack[(usize)i]->label.c_str(), core_text_layout());
                }
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
            GUICore::id_t dock_space,
            GUICore::id_t palette,
            GUICore::id_t preview,
            GUICore::id_t inspector,
            GUICore::id_t tree,
            GUICore::id_t history)
        {
            if(app.dockspace_layout_initialized || !dock_space || !palette || !preview || !inspector || !tree || !history)
            {
                return;
            }
            GUI::DockSpaceLayoutDesc layout;
            layout.root_node = 0;
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::x, 0.10f, 1, 2));
            layout.nodes.push_back(dock_leaf(palette));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::x, 0.72f, 3, 4));
            layout.nodes.push_back(dock_leaf(preview));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::y, 0.42f, 5, 6));
            layout.nodes.push_back(dock_leaf(tree));
            layout.nodes.push_back(dock_split(GUI::DockSplitAxis::y, 0.62f, 7, 8));
            layout.nodes.push_back(dock_leaf(inspector));
            layout.nodes.push_back(dock_leaf(history));
            GUI::set_dockspace_layout(app.editor_core.get(), dock_space, layout);
            app.dockspace_layout_initialized = true;
        }

        static GUICore::ElementHandle draw_editor(App& app, FrameHandles& handles, const Float2U& surface_size)
        {
            sync_inspector(app);
            EditorDocument* document = app.service.active_document();
            bool can_save = document && document->asset;
            bool can_undo = document && !document->undo_stack.empty();
            bool can_redo = document && !document->redo_stack.empty();
            bool can_delete = can_remove_selected(document);
            GUICore::IContext* context = app.editor_core.get();
            context->push_layer(core_id("gui_editor.layer.main", 0), Float2U(0.0f));

            GUICore::LayoutConfig root_layout;
            root_layout.width.kind = GUICore::SizeKind::fixed;
            root_layout.width.value = surface_size.x;
            root_layout.height.kind = GUICore::SizeKind::fixed;
            root_layout.height.value = surface_size.y;
            GUICore::ElementHandle root = GUI::begin_v_layout(context, core_id("gui_editor.root", 0),
                "GUIEditor Root", root_layout);
            f32 menu_height = 34.0f;
            GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, core_id("gui_editor.menu_bar", 0),
                "Main Menu", core_layout_pixels(0.0f, menu_height));
            if(GUI::begin_menu(context, core_id("gui_editor.menu.file", 0), "File"))
            {
                GUI::MenuItemDesc new_desc;
                new_desc.shortcut = "Ctrl+N";
                handles.new_document = GUI::menu_item(context, core_id("gui_editor.menu.file.new", 0),
                    "New", false, new_desc);
                GUI::menu_separator(context, core_id("gui_editor.menu.file.sep0", 0));
                GUI::input_text(context, core_id("gui_editor.menu.file.open_path", 0), app.open_path);
                handles.open_document = GUI::menu_item(context, core_id("gui_editor.menu.file.open", 0), "Open");
                GUI::input_text(context, core_id("gui_editor.menu.file.save_path", 0), app.save_path);
                GUI::MenuItemDesc save_desc;
                save_desc.shortcut = "Ctrl+S";
                save_desc.enabled = can_save;
                handles.save_document = GUI::menu_item(context, core_id("gui_editor.menu.file.save", 0),
                    "Save", false, save_desc);
                lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 260.0f, 150.0f)));
            }
            if(GUI::begin_menu(context, core_id("gui_editor.menu.edit", 0), "Edit"))
            {
                GUI::MenuItemDesc undo_desc;
                undo_desc.shortcut = "Ctrl+Z";
                undo_desc.enabled = can_undo;
                handles.undo = GUI::menu_item(context, core_id("gui_editor.menu.edit.undo", 0),
                    "Undo", false, undo_desc);
                GUI::MenuItemDesc redo_desc;
                redo_desc.shortcut = "Ctrl+Y";
                redo_desc.enabled = can_redo;
                handles.redo = GUI::menu_item(context, core_id("gui_editor.menu.edit.redo", 0),
                    "Redo", false, redo_desc);
                GUI::menu_separator(context, core_id("gui_editor.menu.edit.sep0", 0));
                GUI::MenuItemDesc delete_desc;
                delete_desc.shortcut = "Del";
                delete_desc.enabled = can_delete;
                handles.remove_node_menu = GUI::menu_item(context, core_id("gui_editor.menu.edit.delete", 0),
                    "Delete Node", false, delete_desc);
                lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 105.0f)));
            }
            if(GUI::begin_menu(context, core_id("gui_editor.menu.view", 0), "View"))
            {
                GUI::menu_item(context, core_id("gui_editor.menu.view.preview", 0), "Preview", &app.show_preview);
                GUI::menu_item(context, core_id("gui_editor.menu.view.properties", 0), "Properties", &app.show_properties);
                lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 78.0f)));
            }
            GUI::end_menu_bar(context, menu_bar);

            GUICore::id_t dock_space_id = core_id("gui_editor.dockspace", 0);
            GUICore::id_t palette_id = core_id("gui_editor.panel.palette", 0);
            GUICore::id_t preview_id = core_id("gui_editor.panel.preview", 0);
            GUICore::id_t inspector_id = core_id("gui_editor.panel.inspector", 0);
            GUICore::id_t tree_id = core_id("gui_editor.panel.tree", 0);
            GUICore::id_t history_id = core_id("gui_editor.panel.history", 0);
            set_default_dockspace_layout(app, dock_space_id, palette_id, preview_id, inspector_id, tree_id, history_id);

            GUICore::LayoutConfig dock_layout = core_layout_pixels(0.0f, 0.0f);
            dock_layout.flex_grow = 1.0f;
            GUICore::ElementHandle dock_space = GUI::begin_dock_space(context, dock_space_id, "GUIEditor DockSpace",
                dock_layout);

            if(GUI::begin_dock_panel(context, palette_id, "Node Palette"))
            {
                build_core_palette_panel(app);
                GUI::end_dock_panel(context);
            }
            if(app.show_preview)
            {
                if(GUI::begin_dock_panel(context, preview_id, "Preview", &app.show_preview))
                {
                    draw_preview_panel(app, handles);
                    GUI::end_dock_panel(context);
                }
            }
            if(app.show_properties)
            {
                if(GUI::begin_dock_panel(context, inspector_id, "Inspector", &app.show_properties))
                {
                    build_core_properties_panel(app, handles);
                    GUI::end_dock_panel(context);
                }
            }
            if(GUI::begin_dock_panel(context, tree_id, "Widget Tree"))
            {
                build_core_tree_panel_base(app, handles);
                GUI::end_dock_panel(context);
            }
            if(GUI::begin_dock_panel(context, history_id, "History"))
            {
                build_core_history_panel(app);
                GUI::end_dock_panel(context);
            }
            GUI::end_dock_space(context);
            build_core_tree_popups(app, handles);
            GUICore::FlexLayoutDesc root_desc;
            root_desc.axis = GUICore::LayoutAxis::y;
            root_desc.cross_alignment = GUICore::FlexAlignment::stretch;
            GUI::end_v_layout(context, root, root_desc);
            context->pop_layer();
            return root;
        }

        static void build_core_preview(App& app, const FrameHandles& handles, const GUICore::FrameDesc& frame,
            GUIWindow::GUICoreWindowInputAdapter* input_adapter)
        {
            app.preview_error.clear();
            if(!app.show_preview || !GUI::is_item_valid(app.editor_core.get(), handles.preview_canvas) || !app.preview_core)
            {
                return;
            }
            EditorDocument* document = app.service.active_document();
            if(!document || !document->asset)
            {
                return;
            }
            RectF rect = GUI::get_item_rect(app.editor_core.get(), handles.preview_canvas);
            if(rect.width <= 0.0f || rect.height <= 0.0f)
            {
                return;
            }
            GUICore::FrameDesc core_frame;
            core_frame.screen_size = frame.screen_size;
            core_frame.framebuffer_size = frame.framebuffer_size;
            core_frame.dpi_scale = frame.dpi_scale;
            core_frame.delta_time = frame.delta_time;
            app.preview_core->begin_frame(core_frame);
            if(input_adapter)
            {
                GUIWindow::update_input(input_adapter);
            }
            app.preview_core->push_layer(1, Float2U(rect.offset_x, rect.offset_y));
            GA::GenerateContext generate_context;
            generate_context.core_root_rect = RectF(0.0f, 0.0f, rect.width, rect.height);
            RV r = GA::generate(app.preview_core.get(), document->asset.get(), generate_context);
            app.preview_core->pop_layer();
            app.preview_core->route_input();
            GUI::resolve_interactions(app.preview_core.get());
            if(input_adapter && app.preview_core->get_text_input_state().active)
            {
                RV text_input_result = GUIWindow::update_text_input(input_adapter);
                if(failed(text_input_result))
                {
                    app.preview_error = explain(text_input_result.errcode());
                    return;
                }
            }
            if(failed(r))
            {
                app.preview_error = explain(r.errcode());
            }
        }

        static RV render_core_preview(App& app, RHI::ITexture* back_buffer)
        {
            if(!app.show_preview || !app.preview_core || !app.preview_renderer || !back_buffer)
            {
                return ok;
            }
            return render_core_context(app.preview_core.get(), app.preview_renderer.get(), app.cmdbuf, back_buffer);
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

        static Variant property_edit_value(const PropertyEditHandle& edit)
        {
            switch(edit.kind)
            {
            case GA::NodePropertyKind::string:
            case GA::NodePropertyKind::asset:
                return Variant(edit.string_value.c_str());
            case GA::NodePropertyKind::boolean:
                return Variant(edit.bool_value);
            case GA::NodePropertyKind::integer:
                return Variant((i64)edit.int_value);
            case GA::NodePropertyKind::number:
                return Variant((f64)edit.values[0]);
            case GA::NodePropertyKind::enum_string:
                if(!edit.enum_items.empty() && edit.int_value >= 0 && (usize)edit.int_value < edit.enum_items.size())
                {
                    return Variant(edit.enum_items[(usize)edit.int_value].c_str());
                }
                return Variant(edit.string_value.c_str());
            case GA::NodePropertyKind::size:
                return make_size_variant(edit.values[0], edit.values[1]);
            case GA::NodePropertyKind::edge_insets:
                return make_edge_insets_variant(edit.values[0], edit.values[1], edit.values[2], edit.values[3]);
            case GA::NodePropertyKind::layout_desc:
            {
                Variant layout(VariantType::object);
                layout[Name("padding")] = make_edge_insets_variant(edit.values[0], edit.values[1], edit.values[2], edit.values[3]);
                layout[Name("gap")] = (f64)edit.values[4];
                return layout;
            }
            case GA::NodePropertyKind::string_array:
                return parse_string_array(edit.string_value);
            case GA::NodePropertyKind::number_array:
                return parse_number_array(edit.string_value);
            }
            return Variant();
        }

        static bool process_property_edits(App& app, const FrameHandles& handles, EditorDocument& document)
        {
            for(const UniquePtr<PropertyEditHandle>& edit : handles.property_edits)
            {
                if(!edit || edit->document_id != document.id)
                {
                    continue;
                }
                bool changed = false;
                if(GUI::is_item_valid(app.editor_core.get(), edit->core_handle))
                {
                    Variant new_value = property_edit_value(*edit.get());
                    changed = changed || !(new_value == edit->original_value);
                }
                if(changed)
                {
                    (void)set_property_if_changed(app, document, edit->node, edit->key, property_edit_value(*edit.get()));
                    app.inspector_node = Guid(0, 0);
                    return true;
                }
            }
            return false;
        }

        static bool apply_common_if_changed(App& app, EditorDocument& document)
        {
            Ref<GA::Node> node = GA::find_node(document.asset.get(), document.selected_node);
            if(!node)
            {
                return false;
            }
            if(!strcmp(node->label.c_str(), app.edit_label.c_str()) &&
                !strcmp(node->style.c_str(), app.edit_style.c_str()) &&
                node->enabled == app.edit_enabled)
            {
                return false;
            }
            RV r = app.service.set_node_common(document.id, document.selected_node, app.edit_label.c_str(), app.edit_enabled, Name(app.edit_style.c_str()));
            if(failed(r))
            {
                app.service.last_status = explain(r.errcode());
                return true;
            }
            app.inspector_node = Guid(0, 0);
            return true;
        }

        static bool process_common_edits(App& app, const FrameHandles& handles, EditorDocument& document)
        {
            bool label_focused = GUI::is_item_focused(app.editor_core.get(), handles.core_common_label);
            bool style_focused = GUI::is_item_focused(app.editor_core.get(), handles.core_common_style);
            bool label_lost_focus = app.edit_label_was_focused && !label_focused;
            bool style_lost_focus = app.edit_style_was_focused && !style_focused;
            bool enabled_changed = false;
            if(GUI::is_item_valid(app.editor_core.get(), handles.core_common_enabled))
            {
                Ref<GA::Node> node = GA::find_node(document.asset.get(), document.selected_node);
                enabled_changed = enabled_changed || (node && node->enabled != app.edit_enabled);
            }
            app.edit_label_was_focused = label_focused;
            app.edit_style_was_focused = style_focused;
            if(label_lost_focus || style_lost_focus || enabled_changed)
            {
                return apply_common_if_changed(app, document);
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
            bool new_requested = GUI::is_item_clicked(app.editor_core.get(), handles.new_document) ||
                shortcut_pressed(app, KeyCode::n, app.shortcut_new_down, true);
            bool open_requested = GUI::is_item_clicked(app.editor_core.get(), handles.open_document) ||
                shortcut_pressed(app, KeyCode::o, app.shortcut_open_down, true);
            bool save_requested = GUI::is_item_clicked(app.editor_core.get(), handles.save_document) ||
                shortcut_pressed(app, KeyCode::s, app.shortcut_save_down, true);
            bool undo_requested = GUI::is_item_clicked(app.editor_core.get(), handles.undo) ||
                shortcut_pressed(app, KeyCode::z, app.shortcut_undo_down, true);
            bool redo_requested = GUI::is_item_clicked(app.editor_core.get(), handles.redo) ||
                shortcut_pressed(app, KeyCode::y, app.shortcut_redo_down, true);
            bool delete_requested = GUI::is_item_clicked(app.editor_core.get(), handles.remove_node_menu) ||
                shortcut_pressed(app, KeyCode::del, app.shortcut_delete_down, false);
            bool move_up_requested = shortcut_pressed(app, KeyCode::up, app.shortcut_move_up_down, true);
            bool move_down_requested = shortcut_pressed(app, KeyCode::down, app.shortcut_move_down_down, true);

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
            GUICore::id_t core_new_popup_id = core_id("gui_editor.tree.new_popup", 0);
            GUICore::id_t core_context_popup_id = core_id("gui_editor.tree.context_popup", 0);
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_new_node))
            {
                GUI::open_popup(app.editor_core.get(), core_new_popup_id);
            }
            for(const CoreTypeItemHandle& item : handles.core_new_node_items)
            {
                if(GUI::is_item_clicked(app.editor_core.get(), item.handle) && (usize)item.type_index < app.node_types.size())
                {
                    Guid parent = document->selected_node != Guid(0, 0) ? document->selected_node : GA::get_root(document->asset.get());
                    (void)app.service.create_node(document->id, parent, app.node_types[item.type_index], app.node_types[item.type_index].c_str());
                    GUI::close_popup(app.editor_core.get(), core_new_popup_id);
                    app.inspector_node = Guid(0, 0);
                    return;
                }
            }
            if(process_property_edits(app, handles, *document))
            {
                return;
            }
            if(process_common_edits(app, handles, *document))
            {
                return;
            }
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_node_context_move_up) &&
                can_move_node(document, app.tree_context_node, false))
            {
                Guid parent;
                usize index = 0;
                usize count = 0;
                if(node_order(document, app.tree_context_node, parent, index, count))
                {
                    (void)app.service.reorder_node(document->id, app.tree_context_node, index - 1);
                    GUI::close_popup(app.editor_core.get(), core_context_popup_id);
                    app.inspector_node = Guid(0, 0);
                    return;
                }
            }
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_node_context_move_down) &&
                can_move_node(document, app.tree_context_node, true))
            {
                Guid parent;
                usize index = 0;
                usize count = 0;
                if(node_order(document, app.tree_context_node, parent, index, count))
                {
                    (void)app.service.reorder_node(document->id, app.tree_context_node, index + 1);
                    GUI::close_popup(app.editor_core.get(), core_context_popup_id);
                    app.inspector_node = Guid(0, 0);
                    return;
                }
            }
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_node_context_delete) &&
                can_remove_node(document, app.tree_context_node))
            {
                (void)app.service.remove_node(document->id, app.tree_context_node);
                GUI::close_popup(app.editor_core.get(), core_context_popup_id);
                app.tree_context_node = Guid(0, 0);
                app.inspector_node = Guid(0, 0);
                return;
            }
            for(const CoreNodeHandle& h : handles.core_tree_nodes)
            {
                if(GUI::is_item_clicked(app.editor_core.get(), h.handle))
                {
                    select_node(app, h.node);
                }
                if(GUI::is_item_right_clicked(app.editor_core.get(), h.handle))
                {
                    select_node(app, h.node);
                    app.tree_context_node = h.node;
                    app.tree_context_position = app.editor_core->get_pointer_position();
                    GUI::open_popup(app.editor_core.get(), core_context_popup_id);
                    return;
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
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_set_property))
            {
                Variant value = make_property_value(app.property_type, app.property_value);
                (void)app.service.set_node_property(document->id, document->selected_node, Name(app.property_key.c_str()), move(value));
            }
            if(GUI::is_item_clicked(app.editor_core.get(), handles.core_erase_property))
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
                    GUICore::module_gui_core(),
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
                luset(app.swap_chain, dev->new_swap_chain(app.queue, app.window,
                    RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true, RHI::ColorSpace::srgb})));
                luset(app.cmdbuf, dev->new_command_buffer(app.queue));
                app.editor_core = GUICore::new_context();
                luexp(app.editor_core->register_font(Name("default"), Font::get_default_font()));
                GUI::register_style_schemas(app.editor_core.get());
                luset(app.editor_renderer, GUICore::new_renderer(dev));
                app.preview_core = GUICore::new_context();
                luexp(app.preview_core->register_font(Name("default"), Font::get_default_font()));
                GUI::register_style_schemas(app.preview_core.get());
                luset(app.preview_renderer, GUICore::new_renderer(dev));

                GUIWindow::GUICoreWindowInputAdapter editor_input_adapter;
                editor_input_adapter.window = app.window;
                editor_input_adapter.gui = app.editor_core;
                GUIWindow::install_window_event_handler(&editor_input_adapter);
                GUIWindow::GUICoreWindowInputAdapter preview_input_adapter;
                preview_input_adapter.window = app.window;
                preview_input_adapter.gui = app.preview_core;
                GUIWindow::install_window_event_handler(&preview_input_adapter);

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
                    GUICore::FrameDesc frame;
                    frame.screen_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                    frame.framebuffer_size = fb_sz;
                    frame.dpi_scale = app.window->get_dpi_scale_factor();
                    frame.delta_time = 1.0f / 60.0f;
                    app.editor_core->begin_frame(frame);
                    GUIWindow::update_input(&editor_input_adapter);

                    FrameHandles handles;
                    GUICore::ElementHandle root = draw_editor(app, handles, frame.screen_size);
                    luexp(GUI::layout_tree(app.editor_core.get(), root,
                        RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                    app.editor_core->route_input();
                    GUI::ResolveResult resolved = GUI::resolve_interactions(app.editor_core.get());
                    if(resolved.relayout_requested)
                    {
                        luexp(GUI::layout_tree(app.editor_core.get(), root,
                            RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                    }
                    luexp(GUIWindow::update_text_input(&editor_input_adapter));
                    process_actions(app, handles);
                    build_core_preview(app, handles, frame, &preview_input_adapter);

                    EditorDocument* document = app.service.active_document();
                    c8 title[256];
                    snprintf(title, sizeof(title), "Luna GUI Editor%s", document && document->dirty ? " *" : "");
                    luexp(app.window->set_title(title));

                    lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                    RHI::RenderPassDesc render_pass;
                    render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U(0.02f, 0.025f, 0.03f, 1.0f));
                    app.cmdbuf->begin_render_pass(render_pass);
                    app.cmdbuf->end_render_pass();
                    luexp(render_core_context(app.editor_core.get(), app.editor_renderer.get(), app.cmdbuf, back_buffer));
                    luexp(render_core_preview(app, back_buffer));
                    app.cmdbuf->resource_barrier({}, {
                        {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                    });
                    luexp(app.cmdbuf->submit({}, {}, true));
                    app.cmdbuf->wait();
                    luexp(app.cmdbuf->reset());
                    luexp(app.swap_chain->present());
                }
                GUIWindow::uninstall_window_event_handler(&preview_input_adapter);
                GUIWindow::uninstall_window_event_handler(&editor_input_adapter);
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
