/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIAsset.cpp
* @author JXMaster
* @date 2026/6/10
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_ASSET_API LUNA_EXPORT
#include "../GUIAsset.hpp"
#include "GUIAsset.meta.generated.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <cstring>

namespace Luna
{
    namespace GUIAsset
    {
        static HashMap<Name, NodeTypeDesc> g_node_types;

        static constexpr const c8* name_children = "children";
        static constexpr const c8* name_enabled = "enabled";
        static constexpr const c8* name_id = "id";
        static constexpr const c8* name_label = "label";
        static constexpr const c8* name_layout = "layout";
        static constexpr const c8* name_canvas_layout = "canvas_layout";
        static constexpr const c8* name_nodes = "nodes";
        static constexpr const c8* name_parent = "parent";
        static constexpr const c8* name_properties = "properties";
        static constexpr const c8* name_root = "root";
        static constexpr const c8* name_style = "style";
        static constexpr const c8* name_type = "type";
        static constexpr const c8* name_version = "version";

        namespace AssetTopologyAccess
        {
            Guid root(const Asset* asset)
            {
                return asset ? asset->m_root : Guid(0, 0);
            }

            void set_root(Asset* asset, const Guid& root)
            {
                asset->m_root = root;
            }

            HashMap<Guid, Ref<Node>>& nodes(Asset* asset)
            {
                return asset->m_nodes;
            }

            const HashMap<Guid, Ref<Node>>& nodes(const Asset* asset)
            {
                return asset->m_nodes;
            }

            Guid parent(const Node* node)
            {
                return node ? node->m_parent : Guid(0, 0);
            }

            void set_parent(Node* node, const Guid& parent)
            {
                node->m_parent = parent;
            }

            Vector<Guid>& children(Node* node)
            {
                return node->m_children;
            }

            const Vector<Guid>& children(const Node* node)
            {
                return node->m_children;
            }
        }

        static const Variant& property(const Node& node, const c8* name)
        {
            return node.properties[Name(name)];
        }

        static const Name& name_width()
        {
            static Name v("width");
            return v;
        }

        static const Name& name_height()
        {
            static Name v("height");
            return v;
        }

        static const Name& name_left()
        {
            static Name v("left");
            return v;
        }

        static const Name& name_top()
        {
            static Name v("top");
            return v;
        }

        static const Name& name_right()
        {
            static Name v("right");
            return v;
        }

        static const Name& name_bottom()
        {
            static Name v("bottom");
            return v;
        }

        static f32 property_f32(const Node& node, const c8* name, f32 default_value = 0.0f)
        {
            return (f32)property(node, name).fnum(default_value);
        }

        static i32 property_i32(const Node& node, const c8* name, i32 default_value = 0)
        {
            return (i32)property(node, name).inum(default_value);
        }

        static u32 property_u32(const Node& node, const c8* name, u32 default_value = 0)
        {
            return (u32)property(node, name).unum(default_value);
        }

        static bool property_bool(const Node& node, const c8* name, bool default_value = false)
        {
            return property(node, name).boolean(default_value);
        }

        static const c8* property_c_str(const Node& node, const c8* name, const c8* default_value = "")
        {
            return property(node, name).c_str(default_value);
        }

        static void read_string_items(const Variant& data, Vector<String>& strings, Vector<const c8*>& items)
        {
            strings.clear();
            items.clear();
            for(const Variant& item : data.values())
            {
                strings.push_back(item.c_str());
            }
            items.reserve(strings.size());
            for(const String& item : strings)
            {
                items.push_back(item.c_str());
            }
        }

        template <typename _Ty>
        static _Ty& runtime_value(Node& node, const Name& key, const _Ty& default_value)
        {
            auto iter = node.runtime_values.find(key);
            if(iter == node.runtime_values.end() || !iter->second.as<_Ty>())
            {
                node.runtime_values.insert_or_assign(key, Any(default_value));
                iter = node.runtime_values.find(key);
            }
            return *iter->second.as<_Ty>();
        }

        static GUI::Size read_size(const Variant& data, const GUI::Size& default_value = GUI::Size())
        {
            GUI::Size r = default_value;
            r.width = (f32)data[Name("width")].fnum(r.width);
            r.height = (f32)data[Name("height")].fnum(r.height);
            return r;
        }

        static Variant write_size(const GUI::Size& value)
        {
            Variant r(VariantType::object);
            r[name_width()] = (f64)value.width;
            r[Name("height")] = (f64)value.height;
            return r;
        }

        static GUI::EdgeInsets read_edge_insets(const Variant& data, const GUI::EdgeInsets& default_value = GUI::EdgeInsets())
        {
            GUI::EdgeInsets r = default_value;
            r.left = (f32)data[name_left()].fnum(r.left);
            r.top = (f32)data[name_top()].fnum(r.top);
            r.right = (f32)data[name_right()].fnum(r.right);
            r.bottom = (f32)data[name_bottom()].fnum(r.bottom);
            return r;
        }

        static Variant write_edge_insets(const GUI::EdgeInsets& value)
        {
            Variant r(VariantType::object);
            r[name_left()] = (f64)value.left;
            r[name_top()] = (f64)value.top;
            r[name_right()] = (f64)value.right;
            r[name_bottom()] = (f64)value.bottom;
            return r;
        }

        static GUI::LayoutDesc read_layout_desc(const Variant& data)
        {
            GUI::LayoutDesc r;
            r.padding = read_edge_insets(data[Name("padding")], r.padding);
            r.gap = (f32)data[Name("gap")].fnum(r.gap);
            return r;
        }

        static Variant write_layout_style(const GUI::LayoutStyle& value)
        {
            Variant r(VariantType::object);
            r[Name("width_policy")] = (u64)value.width_policy;
            r[Name("height_policy")] = (u64)value.height_policy;
            r[Name("fixed_width")] = (f64)value.fixed_width_value;
            r[Name("fixed_height")] = (f64)value.fixed_height_value;
            r[Name("fill_weight_x")] = (f64)value.fill_weight_x;
            r[Name("fill_weight_y")] = (f64)value.fill_weight_y;
            return r;
        }

        static GUI::LayoutStyle read_layout_style(const Variant& data)
        {
            GUI::LayoutStyle r;
            r.width_policy = (GUI::SizePolicy)data[Name("width_policy")].unum((u64)r.width_policy);
            r.height_policy = (GUI::SizePolicy)data[Name("height_policy")].unum((u64)r.height_policy);
            r.fixed_width_value = (f32)data[Name("fixed_width")].fnum(r.fixed_width_value);
            r.fixed_height_value = (f32)data[Name("fixed_height")].fnum(r.fixed_height_value);
            r.fill_weight_x = (f32)data[Name("fill_weight_x")].fnum(r.fill_weight_x);
            r.fill_weight_y = (f32)data[Name("fill_weight_y")].fnum(r.fill_weight_y);
            return r;
        }

        static Variant write_canvas_item_layout(const GUI::CanvasItemLayout& value)
        {
            Variant r(VariantType::object);
            r[Name("anchor_min_x")] = (f64)value.anchor_min.x;
            r[Name("anchor_min_y")] = (f64)value.anchor_min.y;
            r[Name("anchor_max_x")] = (f64)value.anchor_max.x;
            r[Name("anchor_max_y")] = (f64)value.anchor_max.y;
            r[Name("offset_min_x")] = (f64)value.offset_min.x;
            r[Name("offset_min_y")] = (f64)value.offset_min.y;
            r[Name("offset_max_x")] = (f64)value.offset_max.x;
            r[Name("offset_max_y")] = (f64)value.offset_max.y;
            return r;
        }

        static GUI::CanvasItemLayout read_canvas_item_layout(const Variant& data)
        {
            GUI::CanvasItemLayout r;
            r.anchor_min.x = (f32)data[Name("anchor_min_x")].fnum(r.anchor_min.x);
            r.anchor_min.y = (f32)data[Name("anchor_min_y")].fnum(r.anchor_min.y);
            r.anchor_max.x = (f32)data[Name("anchor_max_x")].fnum(r.anchor_max.x);
            r.anchor_max.y = (f32)data[Name("anchor_max_y")].fnum(r.anchor_max.y);
            r.offset_min.x = (f32)data[Name("offset_min_x")].fnum(r.offset_min.x);
            r.offset_min.y = (f32)data[Name("offset_min_y")].fnum(r.offset_min.y);
            r.offset_max.x = (f32)data[Name("offset_max_x")].fnum(r.offset_max.x);
            r.offset_max.y = (f32)data[Name("offset_max_y")].fnum(r.offset_max.y);
            return r;
        }

        static void apply_common_modifiers(GUI::IContext* context, const Node& node)
        {
            if(node.has_layout_style)
            {
                GUI::set_next_item_layout(context, node.layout_style);
            }
            if(node.has_canvas_item_layout)
            {
                GUI::set_next_canvas_item_layout(context, node.canvas_item_layout);
            }
            GUI::set_next_item_enabled(context, node.enabled);
            if(!node.style.empty())
            {
                GUI::push_style(context, node.style);
            }
        }

        static void finish_common_modifiers(GUI::IContext* context, const Node& node)
        {
            if(!node.style.empty())
            {
                GUI::pop_style(context);
            }
        }

        static RV generate_h_layout(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::begin_h_layout(context, node.label.c_str(), read_layout_desc(node.properties));
            RV r = generate_children(context, node, generate_context);
            GUI::end_h_layout(context);
            return r;
        }

        static RV generate_v_layout(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::begin_v_layout(context, node.label.c_str(), read_layout_desc(node.properties));
            RV r = generate_children(context, node, generate_context);
            GUI::end_v_layout(context);
            return r;
        }

        static RV generate_scroll_view(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::ScrollViewDesc desc;
            const c8* mode = property_c_str(node, "scroll_bar_mode", "auto_hide_overlay");
            desc.scroll_bar_mode = strcmp(mode, "always_visible_reserved") == 0 ? GUI::ScrollBarMode::always_visible_reserved : GUI::ScrollBarMode::auto_hide_overlay;
            GUI::begin_scroll_view(context, node.label.c_str(), read_size(node.properties[Name("size")]), desc);
            RV r = generate_children(context, node, generate_context);
            GUI::end_scroll_view(context);
            return r;
        }

        static RV generate_grid_layout(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::GridLayoutDesc desc;
            const c8* mode = property_c_str(node, "sizing_mode", "fixed_cell_size");
            desc.sizing_mode = strcmp(mode, "fixed_columns") == 0 ? GUI::GridSizingMode::fixed_columns : GUI::GridSizingMode::fixed_cell_size;
            desc.cell_size.x = property_f32(node, "cell_width", desc.cell_size.x);
            desc.cell_size.y = property_f32(node, "cell_height", desc.cell_size.y);
            desc.columns = property_u32(node, "columns", desc.columns);
            desc.padding = read_edge_insets(node.properties[Name("padding")], desc.padding);
            desc.gap.x = property_f32(node, "gap_x", desc.gap.x);
            desc.gap.y = property_f32(node, "gap_y", desc.gap.y);
            GUI::begin_grid_layout(context, node.label.c_str(), desc);
            RV r = generate_children(context, node, generate_context);
            GUI::end_grid_layout(context);
            return r;
        }

        static RV generate_canvas_layout(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::CanvasLayoutDesc desc;
            desc.padding = read_edge_insets(node.properties[Name("padding")], desc.padding);
            desc.clip_children = property_bool(node, "clip_children", desc.clip_children);
            GUI::begin_canvas_layout(context, node.label.c_str(), read_size(node.properties[Name("size")]), desc);
            RV r = generate_children(context, node, generate_context);
            GUI::end_canvas_layout(context);
            return r;
        }

        static RV generate_table_layout(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::TableDesc desc;
            const Variant& columns = node.properties[Name("columns")];
            for(const Variant& column : columns.values())
            {
                desc.column_sizes.push_back(GUI::TableTrackSize::fixed((f32)column.fnum(96.0f)));
            }
            desc.row_height_mode = property_bool(node, "fixed_row_height_mode", false) ? GUI::TableRowHeightMode::fixed : GUI::TableRowHeightMode::track_sizes;
            desc.fixed_row_height = property_f32(node, "fixed_row_height", desc.fixed_row_height);
            desc.virtualize_fixed_rows = property_bool(node, "virtualize_fixed_rows", desc.virtualize_fixed_rows);
            desc.style.row_separators = property_bool(node, "row_separators", desc.style.row_separators);
            desc.style.column_separators = property_bool(node, "column_separators", desc.style.column_separators);
            const c8* background = property_c_str(node, "background_mode", "none");
            if(strcmp(background, "alternate_rows") == 0)
            {
                desc.style.background_mode = GUI::TableBackgroundMode::alternate_rows;
            }
            else if(strcmp(background, "alternate_columns") == 0)
            {
                desc.style.background_mode = GUI::TableBackgroundMode::alternate_columns;
            }
            else if(strcmp(background, "solid") == 0)
            {
                desc.style.background_mode = GUI::TableBackgroundMode::solid;
            }
            GUI::begin_table_layout(context, node.label.c_str(), desc);
            RV r = generate_children(context, node, generate_context);
            GUI::end_table_layout(context);
            return r;
        }

        static RV generate_table_row(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            if(GUI::begin_table_row(context))
            {
                RV r = generate_children(context, node, generate_context);
                GUI::end_table_row(context);
                return r;
            }
            return ok;
        }

        static RV generate_text(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::text(context, node.label.c_str());
            return ok;
        }

        static RV generate_button(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::text_button(context, node.label.c_str());
            return ok;
        }

        static RV generate_progress_bar(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            const Variant& overlay_value = property(node, "overlay");
            const c8* overlay = overlay_value.valid() ? overlay_value.c_str("") : nullptr;
            GUI::progress_bar(context, node.label.c_str(), property_f32(node, "fraction", 0.0f), read_size(node.properties[Name("size")]), overlay);
            return ok;
        }

        static RV generate_selectable(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::selectable(context, node.label.c_str(), property_bool(node, "selected", false));
            return ok;
        }

        static RV generate_checkbox(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            bool& value = runtime_value(node, Name("value"), property_bool(node, "value", false));
            GUI::checkbox(context, node.label.c_str(), &value);
            return ok;
        }

        static RV generate_radio_button(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::radio_button(context, node.label.c_str(), property_bool(node, "selected", false));
            return ok;
        }

        static RV generate_toggle_switch(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            bool& value = runtime_value(node, Name("value"), property_bool(node, "value", false));
            GUI::toggle_switch(context, node.label.c_str(), &value);
            return ok;
        }

        static RV generate_input_text(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            String default_value(property_c_str(node, "value", ""));
            String& value = runtime_value(node, Name("value"), default_value);
            GUI::input_text(context, node.label.c_str(), value);
            return ok;
        }

        static RV generate_image(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::image(context, nullptr, read_size(node.properties[Name("size")]));
            return ok;
        }

        static RV generate_collapsing_header(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::ItemHandle handle = GUI::collapsing_header(context, node.label.c_str());
            if(GUI::get_item_state(handle, GUI::State::open()))
            {
                return generate_children(context, node, generate_context);
            }
            return ok;
        }

        static RV generate_tree_node(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::none;
            if(property_bool(node, "selected", false)) flags |= GUI::TreeNodeFlag::selected;
            if(property_bool(node, "leaf", false)) flags |= GUI::TreeNodeFlag::leaf;
            if(property_bool(node, "default_open", false)) flags |= GUI::TreeNodeFlag::default_open;
            GUI::ItemHandle handle = GUI::tree_node(context, node.label.c_str(), flags);
            if(!test_flags(flags, GUI::TreeNodeFlag::leaf) && GUI::get_item_state(handle, GUI::State::open()))
            {
                GUI::tree_push(context, handle);
                RV r = generate_children(context, node, generate_context);
                GUI::tree_pop(context);
                return r;
            }
            return ok;
        }

        static RV generate_button_group(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            Vector<String> item_strings;
            Vector<const c8*> items;
            read_string_items(node.properties[Name("items")], item_strings, items);
            if(items.empty())
            {
                return ok;
            }
            if(property_bool(node, "multi_select", false))
            {
                Vector<bool> default_values;
                for(const Variant& selected : node.properties[Name("selected")].values())
                {
                    default_values.push_back(selected.boolean(false));
                }
                default_values.resize(items.size(), false);
                Vector<bool>& selected_values = runtime_value(node, Name("selected"), default_values);
                selected_values.resize(items.size(), false);
                GUI::button_group(context, node.label.c_str(), Span<bool>(selected_values.data(), selected_values.size()), Span<const c8*>(items.data(), items.size()));
            }
            else
            {
                i32& current_item = runtime_value(node, Name("current_item"), property_i32(node, "current_item", 0));
                GUI::button_group(context, node.label.c_str(), &current_item, Span<const c8*>(items.data(), items.size()));
            }
            return ok;
        }

        static RV generate_combo(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            Vector<String> item_strings;
            Vector<const c8*> items;
            read_string_items(node.properties[Name("items")], item_strings, items);
            if(items.empty())
            {
                return ok;
            }
            i32& current_item = runtime_value(node, Name("current_item"), property_i32(node, "current_item", 0));
            GUI::combo(context, node.label.c_str(), &current_item, Span<const c8*>(items.data(), items.size()));
            return ok;
        }

        static RV generate_slider_float(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            f32& value = runtime_value(node, Name("value"), property_f32(node, "value", 0.0f));
            GUI::slider_float(context, node.label.c_str(), &value, property_f32(node, "min", 0.0f), property_f32(node, "max", 1.0f));
            return ok;
        }

        static RV generate_slider_int(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            i32& value = runtime_value(node, Name("value"), property_i32(node, "value", 0));
            GUI::slider_int(context, node.label.c_str(), &value, property_i32(node, "min", 0), property_i32(node, "max", 100));
            return ok;
        }

        static RV generate_drag_float(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            f32& value = runtime_value(node, Name("value"), property_f32(node, "value", 0.0f));
            GUI::drag_float(context, node.label.c_str(), &value, property_f32(node, "speed", 1.0f), property_f32(node, "min", 0.0f), property_f32(node, "max", 1.0f));
            return ok;
        }

        static RV generate_drag_int(GUI::IContext* context, Node& node, const GenerateContext&)
        {
            i32& value = runtime_value(node, Name("value"), property_i32(node, "value", 0));
            GUI::drag_int(context, node.label.c_str(), &value, property_f32(node, "speed", 1.0f), property_i32(node, "min", 0), property_i32(node, "max", 100));
            return ok;
        }

        static RV generate_asset_reference(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            Luna::Asset::asset_t asset_ref;
            RV deserialize_result = deserialize(asset_ref, property(node, "asset"));
            if(failed(deserialize_result) || !asset_ref)
            {
                return ok;
            }
            if(Luna::Asset::get_asset_state(asset_ref) == Luna::Asset::AssetState::unloaded)
            {
                RV load_result = Luna::Asset::load_asset(asset_ref);
                if(failed(load_result))
                {
                    return load_result;
                }
            }
            Ref<Asset> asset = Luna::Asset::get_asset_data<Asset>(asset_ref);
            if(asset)
            {
                GenerateContext nested_context = generate_context;
                nested_context.owner_asset = asset_ref;
                return generate(context, asset.get(), nested_context);
            }
            return ok;
        }

        static void get_asset_reference_referred_assets(const Node& node, Vector<Luna::Asset::asset_t>& referred_assets)
        {
            Luna::Asset::asset_t asset_ref;
            if(succeeded(deserialize(asset_ref, property(node, "asset"))) && asset_ref)
            {
                referred_assets.push_back(asset_ref);
            }
        }

        static Variant default_size(f32 width, f32 height)
        {
            return write_size(GUI::Size::fixed(width, height));
        }

        static Variant string_array(Span<const c8* const> items)
        {
            Variant r(VariantType::array);
            for(const c8* item : items)
            {
                r.push_back(item ? item : "");
            }
            return r;
        }

        static Variant number_array(Span<const f64> values)
        {
            Variant r(VariantType::array);
            for(f64 value : values)
            {
                r.push_back(value);
            }
            return r;
        }

        static NodeTypeDesc make_desc(const c8* type, node_generate_func_t generate, Variant default_properties = Variant(VariantType::object))
        {
            NodeTypeDesc desc;
            desc.type = type;
            desc.default_properties = move(default_properties);
            desc.on_generate = generate;
            return desc;
        }

        static void add_property(
            NodeTypeDesc& desc,
            const c8* key,
            const c8* display_name,
            NodePropertyKind kind,
            Variant default_value,
            const c8* category = "Properties",
            f64 min_value = 0.0,
            f64 max_value = 1.0,
            f32 speed = 1.0f,
            Span<const c8* const> enum_items = Span<const c8* const>())
        {
            NodePropertyDesc property_desc;
            property_desc.key = key;
            property_desc.display_name = display_name ? display_name : key;
            property_desc.category = category ? category : "";
            property_desc.kind = kind;
            property_desc.default_value = default_value;
            property_desc.min_value = min_value;
            property_desc.max_value = max_value;
            property_desc.speed = speed;
            for(const c8* item : enum_items)
            {
                property_desc.enum_items.push_back(item ? item : "");
            }
            desc.properties.push_back(move(property_desc));
            if(!key || !key[0])
            {
                return;
            }
            if(desc.default_properties.type() != VariantType::object)
            {
                desc.default_properties = Variant(VariantType::object);
            }
            desc.default_properties[Name(key)] = move(default_value);
        }

        static NodeTypeDesc make_layout_desc(const c8* type, node_generate_func_t generate)
        {
            NodeTypeDesc desc = make_desc(type, generate);
            GUI::LayoutDesc layout_desc;
            add_property(desc, "padding", "Padding", NodePropertyKind::edge_insets, write_edge_insets(layout_desc.padding), "Layout");
            add_property(desc, "gap", "Gap", NodePropertyKind::number, (f64)layout_desc.gap, "Layout", 0.0, 128.0, 1.0f);
            return desc;
        }

        static void register_builtin_node_types()
        {
            register_node_type(make_layout_desc("h_layout", generate_h_layout));
            register_node_type(make_layout_desc("v_layout", generate_v_layout));
            {
                NodeTypeDesc desc = make_desc("scroll_view", generate_scroll_view);
                const c8* modes[] = {"auto_hide_overlay", "always_visible_reserved"};
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 240.0f), "Layout");
                add_property(desc, "scroll_bar_mode", "Scroll Bar Mode", NodePropertyKind::enum_string, "auto_hide_overlay", "Scroll", 0.0, 1.0, 1.0f, Span<const c8* const>(modes, 2));
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("grid_layout", generate_grid_layout);
                GUI::GridLayoutDesc grid_desc;
                const c8* modes[] = {"fixed_cell_size", "fixed_columns"};
                add_property(desc, "sizing_mode", "Sizing Mode", NodePropertyKind::enum_string, "fixed_cell_size", "Layout", 0.0, 1.0, 1.0f, Span<const c8* const>(modes, 2));
                add_property(desc, "cell_width", "Cell Width", NodePropertyKind::number, (f64)grid_desc.cell_size.x, "Layout", 1.0, 4096.0, 1.0f);
                add_property(desc, "cell_height", "Cell Height", NodePropertyKind::number, (f64)grid_desc.cell_size.y, "Layout", 1.0, 4096.0, 1.0f);
                add_property(desc, "columns", "Columns", NodePropertyKind::integer, (i64)grid_desc.columns, "Layout", 1.0, 128.0, 1.0f);
                add_property(desc, "padding", "Padding", NodePropertyKind::edge_insets, write_edge_insets(grid_desc.padding), "Layout");
                add_property(desc, "gap_x", "Gap X", NodePropertyKind::number, (f64)grid_desc.gap.x, "Layout", 0.0, 256.0, 1.0f);
                add_property(desc, "gap_y", "Gap Y", NodePropertyKind::number, (f64)grid_desc.gap.y, "Layout", 0.0, 256.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("canvas_layout", generate_canvas_layout);
                GUI::CanvasLayoutDesc canvas_desc;
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 240.0f), "Layout");
                add_property(desc, "padding", "Padding", NodePropertyKind::edge_insets, write_edge_insets(canvas_desc.padding), "Layout");
                add_property(desc, "clip_children", "Clip Children", NodePropertyKind::boolean, true, "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("table_layout", generate_table_layout);
                const f64 columns[] = {120.0, 180.0, 120.0};
                const c8* backgrounds[] = {"none", "solid", "alternate_rows", "alternate_columns"};
                add_property(desc, "columns", "Columns", NodePropertyKind::number_array, number_array(Span<const f64>(columns, 3)), "Table");
                add_property(desc, "fixed_row_height_mode", "Fixed Row Height Mode", NodePropertyKind::boolean, false, "Table");
                add_property(desc, "fixed_row_height", "Fixed Row Height", NodePropertyKind::number, 28.0, "Table", 1.0, 512.0, 1.0f);
                add_property(desc, "virtualize_fixed_rows", "Virtualize Fixed Rows", NodePropertyKind::boolean, false, "Table");
                add_property(desc, "row_separators", "Row Separators", NodePropertyKind::boolean, true, "Table");
                add_property(desc, "column_separators", "Column Separators", NodePropertyKind::boolean, true, "Table");
                add_property(desc, "background_mode", "Background Mode", NodePropertyKind::enum_string, "alternate_rows", "Table", 0.0, 1.0, 1.0f, Span<const c8* const>(backgrounds, 4));
                register_node_type(desc);
            }
            register_node_type(make_desc("table_row", generate_table_row));
            register_node_type(make_desc("text", generate_text));
            register_node_type(make_desc("button", generate_button));
            {
                NodeTypeDesc desc = make_desc("progress_bar", generate_progress_bar);
                add_property(desc, "fraction", "Fraction", NodePropertyKind::number, 0.5, "Progress", 0.0, 1.0, 0.01f);
                add_property(desc, "overlay", "Overlay", NodePropertyKind::string, "", "Progress");
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(0.0f, 0.0f), "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("selectable", generate_selectable);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("checkbox", generate_checkbox);
                add_property(desc, "value", "Value", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("radio_button", generate_radio_button);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("toggle_switch", generate_toggle_switch);
                add_property(desc, "value", "Value", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("input_text", generate_input_text);
                add_property(desc, "value", "Value", NodePropertyKind::string, "", "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("image", generate_image);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(64.0f, 64.0f), "Layout");
                register_node_type(desc);
            }
            register_node_type(make_desc("collapsing_header", generate_collapsing_header));
            {
                NodeTypeDesc desc = make_desc("tree_node", generate_tree_node);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                add_property(desc, "leaf", "Leaf", NodePropertyKind::boolean, false, "Behavior");
                add_property(desc, "default_open", "Default Open", NodePropertyKind::boolean, false, "Behavior");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("button_group", generate_button_group);
                const c8* items[] = {"One", "Two", "Three"};
                add_property(desc, "items", "Items", NodePropertyKind::string_array, string_array(Span<const c8* const>(items, 3)), "Items");
                add_property(desc, "current_item", "Current Item", NodePropertyKind::integer, (i64)0, "State", 0.0, 64.0, 1.0f);
                add_property(desc, "multi_select", "Multi Select", NodePropertyKind::boolean, false, "Behavior");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("combo", generate_combo);
                const c8* items[] = {"Alpha", "Beta", "Gamma"};
                add_property(desc, "items", "Items", NodePropertyKind::string_array, string_array(Span<const c8* const>(items, 3)), "Items");
                add_property(desc, "current_item", "Current Item", NodePropertyKind::integer, (i64)0, "State", 0.0, 64.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("slider_float", generate_slider_float);
                add_property(desc, "value", "Value", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "max", "Max", NodePropertyKind::number, 1.0, "Numeric", -10000.0, 10000.0, 0.01f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("slider_int", generate_slider_int);
                add_property(desc, "value", "Value", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "min", "Min", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "max", "Max", NodePropertyKind::integer, (i64)100, "Numeric", -10000.0, 10000.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("drag_float", generate_drag_float);
                add_property(desc, "value", "Value", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "speed", "Speed", NodePropertyKind::number, 1.0, "Numeric", 0.001, 1000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "max", "Max", NodePropertyKind::number, 1.0, "Numeric", -10000.0, 10000.0, 0.01f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("drag_int", generate_drag_int);
                add_property(desc, "value", "Value", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "speed", "Speed", NodePropertyKind::number, 1.0, "Numeric", 0.001, 1000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "max", "Max", NodePropertyKind::integer, (i64)100, "Numeric", -10000.0, 10000.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc asset_ref = make_desc("asset_reference", generate_asset_reference);
                add_property(asset_ref, "asset", "Asset", NodePropertyKind::asset, Variant(), "Reference");
                asset_ref.on_get_referred_assets = get_asset_reference_referred_assets;
                register_node_type(asset_ref);
            }
        }

        LUNA_GUI_ASSET_API Name asset_type_name()
        {
            return "GUIAsset";
        }

        LUNA_GUI_ASSET_API void register_node_type(const NodeTypeDesc& desc)
        {
            if(!desc.type.empty())
            {
                g_node_types.insert_or_assign(desc.type, desc);
            }
        }

        LUNA_GUI_ASSET_API R<NodeTypeDesc> get_node_type(const Name& type)
        {
            auto iter = g_node_types.find(type);
            if(iter == g_node_types.end())
            {
                return set_error(BasicError::not_found(), "GUIAsset node type '%s' is not registered.", type.c_str());
            }
            return iter->second;
        }

        LUNA_GUI_ASSET_API void get_node_types(Vector<Name>& out_types)
        {
            for(const auto& pair : g_node_types)
            {
                out_types.push_back(pair.first);
            }
            sort(out_types.begin(), out_types.end(), [](const Name& lhs, const Name& rhs) {
                return strcmp(lhs.c_str(), rhs.c_str()) < 0;
            });
        }

        LUNA_GUI_ASSET_API R<Ref<Node>> new_node(const Name& type, const c8* label)
        {
            lutry
            {
                lulet(desc, get_node_type(type));
                Ref<Node> node = new_object<Node>();
                node->id = random_guid();
                node->type = type;
                node->label = label ? label : "";
                node->properties = desc.default_properties;
                return node;
            }
            lucatchret;
        }

        static void erase_child_reference(Vector<Guid>& children, const Guid& child)
        {
            for(usize i = 0; i < children.size(); ++i)
            {
                if(children[i] == child)
                {
                    children.erase(children.begin() + i);
                    return;
                }
            }
        }

        static void insert_child_reference(Vector<Guid>& children, const Guid& child, usize index)
        {
            if(index == USIZE_MAX || index >= children.size())
            {
                children.push_back(child);
            }
            else
            {
                children.insert(children.begin() + index, child);
            }
        }

        static bool is_ancestor(const Asset& asset, const Guid& ancestor, const Guid& child)
        {
            Guid current = child;
            while(current != Guid(0, 0))
            {
                if(current == ancestor)
                {
                    return true;
                }
                const HashMap<Guid, Ref<Node>>& nodes = AssetTopologyAccess::nodes(&asset);
                auto iter = nodes.find(current);
                if(iter == nodes.end() || !iter->second)
                {
                    return false;
                }
                current = AssetTopologyAccess::parent(iter->second.get());
            }
            return false;
        }

        static void remove_node_subtree(Asset& asset, const Guid& id)
        {
            Ref<Node> node = find_node(&asset, id);
            if(!node)
            {
                return;
            }
            Vector<Guid> children = AssetTopologyAccess::children(node.get());
            for(const Guid& child : children)
            {
                remove_node_subtree(asset, child);
            }
            AssetTopologyAccess::nodes(&asset).erase(id);
        }

        LUNA_GUI_ASSET_API Ref<Node> find_node(Asset* asset, const Guid& id)
        {
            if(!asset || id == Guid(0, 0))
            {
                return Ref<Node>();
            }
            HashMap<Guid, Ref<Node>>& nodes = AssetTopologyAccess::nodes(asset);
            auto iter = nodes.find(id);
            return iter == nodes.end() ? Ref<Node>() : iter->second;
        }

        LUNA_GUI_ASSET_API Ref<Node> find_node(const Asset* asset, const Guid& id)
        {
            if(!asset || id == Guid(0, 0))
            {
                return Ref<Node>();
            }
            const HashMap<Guid, Ref<Node>>& nodes = AssetTopologyAccess::nodes(asset);
            auto iter = nodes.find(id);
            return iter == nodes.end() ? Ref<Node>() : iter->second;
        }

        LUNA_GUI_ASSET_API Guid get_root(const Asset* asset)
        {
            return AssetTopologyAccess::root(asset);
        }

        LUNA_GUI_ASSET_API usize get_node_count(const Asset* asset)
        {
            return asset ? AssetTopologyAccess::nodes(asset).size() : 0;
        }

        LUNA_GUI_ASSET_API Guid get_parent(const Node* node)
        {
            return AssetTopologyAccess::parent(node);
        }

        LUNA_GUI_ASSET_API Span<const Guid> get_children(const Node* node)
        {
            if(!node)
            {
                return Span<const Guid>();
            }
            const Vector<Guid>& children = AssetTopologyAccess::children(node);
            return Span<const Guid>(children.data(), children.size());
        }

        LUNA_GUI_ASSET_API usize get_child_count(const Node* node)
        {
            return node ? AssetTopologyAccess::children(node).size() : 0;
        }

        LUNA_GUI_ASSET_API Guid get_child(const Node* node, usize index)
        {
            if(!node)
            {
                return Guid(0, 0);
            }
            const Vector<Guid>& children = AssetTopologyAccess::children(node);
            return index < children.size() ? children[index] : Guid(0, 0);
        }

        LUNA_GUI_ASSET_API RV add_node(Asset* asset, Ref<Node> node, const Guid& parent, usize index)
        {
            if(!asset || !node)
            {
                return BasicError::bad_arguments();
            }
            if(node->id == Guid(0, 0))
            {
                node->id = random_guid();
            }
            HashMap<Guid, Ref<Node>>& nodes = AssetTopologyAccess::nodes(asset);
            if(nodes.find(node->id) != nodes.end())
            {
                return set_error(BasicError::already_exists(), "GUIAsset node already exists.");
            }
            Ref<Node> parent_node;
            if(parent != Guid(0, 0))
            {
                parent_node = find_node(asset, parent);
                if(!parent_node || parent == node->id)
                {
                    return BasicError::bad_arguments();
                }
            }
            AssetTopologyAccess::set_parent(node.get(), parent);
            nodes.insert_or_assign(node->id, node);
            if(parent_node)
            {
                insert_child_reference(AssetTopologyAccess::children(parent_node.get()), node->id, index);
            }
            else if(AssetTopologyAccess::root(asset) == Guid(0, 0))
            {
                AssetTopologyAccess::set_root(asset, node->id);
            }
            return ok;
        }

        LUNA_GUI_ASSET_API RV remove_node(Asset* asset, const Guid& id)
        {
            if(!asset || id == Guid(0, 0))
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> node = find_node(asset, id);
            if(!node)
            {
                return BasicError::not_found();
            }
            Ref<Node> parent = find_node(asset, AssetTopologyAccess::parent(node.get()));
            if(parent)
            {
                erase_child_reference(AssetTopologyAccess::children(parent.get()), id);
            }
            remove_node_subtree(*asset, id);
            if(AssetTopologyAccess::root(asset) == id)
            {
                AssetTopologyAccess::set_root(asset, Guid(0, 0));
            }
            return ok;
        }

        LUNA_GUI_ASSET_API RV set_root(Asset* asset, const Guid& id)
        {
            if(!asset || id == Guid(0, 0))
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> node = find_node(asset, id);
            if(!node)
            {
                return BasicError::not_found();
            }
            Ref<Node> parent = find_node(asset, AssetTopologyAccess::parent(node.get()));
            if(parent)
            {
                erase_child_reference(AssetTopologyAccess::children(parent.get()), id);
            }
            AssetTopologyAccess::set_parent(node.get(), Guid(0, 0));
            AssetTopologyAccess::set_root(asset, id);
            return ok;
        }

        LUNA_GUI_ASSET_API RV detach_node(Asset* asset, const Guid& id)
        {
            if(!asset || id == Guid(0, 0))
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> node = find_node(asset, id);
            if(!node)
            {
                return BasicError::not_found();
            }
            if(AssetTopologyAccess::root(asset) == id)
            {
                return ok;
            }
            Ref<Node> parent = find_node(asset, AssetTopologyAccess::parent(node.get()));
            if(parent)
            {
                erase_child_reference(AssetTopologyAccess::children(parent.get()), id);
            }
            AssetTopologyAccess::set_parent(node.get(), Guid(0, 0));
            return ok;
        }

        LUNA_GUI_ASSET_API RV move_node(Asset* asset, const Guid& id, const Guid& new_parent, usize index)
        {
            if(!asset || id == Guid(0, 0) || new_parent == Guid(0, 0) || id == new_parent)
            {
                return BasicError::bad_arguments();
            }
            if(AssetTopologyAccess::root(asset) == id)
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> node = find_node(asset, id);
            Ref<Node> parent = find_node(asset, new_parent);
            if(!node || !parent)
            {
                return BasicError::not_found();
            }
            if(is_ancestor(*asset, id, new_parent))
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> old_parent = find_node(asset, AssetTopologyAccess::parent(node.get()));
            if(old_parent)
            {
                erase_child_reference(AssetTopologyAccess::children(old_parent.get()), id);
            }
            AssetTopologyAccess::set_parent(node.get(), new_parent);
            insert_child_reference(AssetTopologyAccess::children(parent.get()), id, index);
            return ok;
        }

        LUNA_GUI_ASSET_API RV reorder_node(Asset* asset, const Guid& id, usize index)
        {
            if(!asset || id == Guid(0, 0))
            {
                return BasicError::bad_arguments();
            }
            Ref<Node> node = find_node(asset, id);
            if(!node)
            {
                return BasicError::not_found();
            }
            Ref<Node> parent = find_node(asset, AssetTopologyAccess::parent(node.get()));
            if(!parent)
            {
                return ok;
            }
            Vector<Guid>& children = AssetTopologyAccess::children(parent.get());
            erase_child_reference(children, id);
            insert_child_reference(children, id, index);
            return ok;
        }

        LUNA_GUI_ASSET_API Ref<Asset> new_asset()
        {
            Ref<Asset> asset = new_object<Asset>();
            auto root = new_node("v_layout", "Root");
            if(succeeded(root))
            {
                root.get()->layout_style = GUI::LayoutStyle::fill();
                root.get()->has_layout_style = true;
                add_node(asset.get(), root.get());
            }
            return asset;
        }

        LUNA_GUI_ASSET_API RV generate(GUI::IContext* context, Asset* asset, const GenerateContext& generate_context)
        {
            Guid root_id = get_root(asset);
            if(!context || !asset || root_id == Guid(0, 0))
            {
                return set_error(BasicError::bad_arguments(), "GUIAsset::generate requires a valid context and asset root.");
            }
            Ref<Node> root = find_node(asset, root_id);
            if(!root)
            {
                return set_error(BasicError::not_found(), "GUIAsset root node is not found in the node map.");
            }
            GenerateContext effective_context = generate_context;
            effective_context.asset = asset;
            return generate_node(context, *root.get(), effective_context);
        }

        LUNA_GUI_ASSET_API RV generate_node(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            lutry
            {
                lulet(desc, get_node_type(node.type));
                if(!desc.on_generate)
                {
                    return set_error(BasicError::not_supported(), "GUIAsset node type '%s' has no generate callback.", node.type.c_str());
                }
                if(node.id == Guid(0, 0))
                {
                    node.id = random_guid();
                }
                GUI::push_id(context, node.id.high);
                GUI::push_id(context, node.id.low);
                apply_common_modifiers(context, node);
                RV r = desc.on_generate(context, node, generate_context);
                finish_common_modifiers(context, node);
                GUI::pop_id(context);
                GUI::pop_id(context);
                if(failed(r))
                {
                    return r;
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_ASSET_API RV generate_children(GUI::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            if(!generate_context.asset)
            {
                return set_error(BasicError::bad_arguments(), "GUIAsset::generate_children requires GenerateContext::asset.");
            }
            for(const Guid& child_id : get_children(&node))
            {
                Ref<Node> child = find_node(generate_context.asset, child_id);
                if(child)
                {
                    RV r = generate_node(context, *child.get(), generate_context);
                    if(failed(r))
                    {
                        return r;
                    }
                }
            }
            return ok;
        }

        LUNA_GUI_ASSET_API R<Variant> serialize_node(const Node& node)
        {
            lutry
            {
                Variant r(VariantType::object);
                luset(r[name_id], serialize(node.id));
                luset(r[name_parent], serialize(get_parent(&node)));
                r[name_type] = node.type;
                r[name_label] = node.label.c_str();
                r[name_properties] = node.properties;
                r[name_enabled] = node.enabled;
                if(!node.style.empty())
                {
                    r[name_style] = node.style;
                }
                if(node.has_layout_style)
                {
                    r[name_layout] = write_layout_style(node.layout_style);
                }
                if(node.has_canvas_item_layout)
                {
                    r[name_canvas_layout] = write_canvas_item_layout(node.canvas_item_layout);
                }
                Variant children(VariantType::array);
                for(const Guid& child : get_children(&node))
                {
                    lulet(serialized_child, serialize(child));
                    children.push_back(move(serialized_child));
                }
                r[name_children] = move(children);
                return r;
            }
            lucatchret;
        }

        LUNA_GUI_ASSET_API R<Ref<Node>> deserialize_node(const Variant& data)
        {
            lutry
            {
                Ref<Node> node = new_object<Node>();
                if(data[name_id].valid())
                {
                    luexp(deserialize(node->id, data[name_id]));
                }
                if(data[name_parent].valid())
                {
                    Guid parent;
                    luexp(deserialize(parent, data[name_parent]));
                    AssetTopologyAccess::set_parent(node.get(), parent);
                }
                if(node->id == Guid(0, 0))
                {
                    node->id = random_guid();
                }
                node->type = data[name_type].str();
                node->label = data[name_label].c_str();
                node->properties = data[name_properties].valid() ? data[name_properties] : Variant(VariantType::object);
                node->enabled = data[name_enabled].boolean(true);
                node->style = data[name_style].str();
                if(data[name_layout].valid())
                {
                    node->layout_style = read_layout_style(data[name_layout]);
                    node->has_layout_style = true;
                }
                if(data[name_canvas_layout].valid())
                {
                    node->canvas_item_layout = read_canvas_item_layout(data[name_canvas_layout]);
                    node->has_canvas_item_layout = true;
                }
                for(const Variant& child_data : data[name_children].values())
                {
                    Guid child;
                    luexp(deserialize(child, child_data));
                    AssetTopologyAccess::children(node.get()).push_back(child);
                }
                return node;
            }
            lucatchret;
        }

        static bool contains_guid(const Vector<Guid>& values, const Guid& value)
        {
            for(const Guid& item : values)
            {
                if(item == value)
                {
                    return true;
                }
            }
            return false;
        }

        static RV append_serialized_node(const Asset& asset, const Guid& id, Vector<Guid>& serialized_ids, Variant& nodes)
        {
            if(id == Guid(0, 0) || contains_guid(serialized_ids, id))
            {
                return ok;
            }
            Ref<Node> node = find_node(&asset, id);
            if(!node)
            {
                return ok;
            }
            serialized_ids.push_back(id);
            lutry
            {
                lulet(serialized_node, serialize_node(*node.get()));
                nodes.push_back(move(serialized_node));
                for(const Guid& child_id : get_children(node.get()))
                {
                    luexp(append_serialized_node(asset, child_id, serialized_ids, nodes));
                }
            }
            lucatchret;
            return ok;
        }

        static bool guid_less(const Guid& lhs, const Guid& rhs)
        {
            return lhs.high == rhs.high ? lhs.low < rhs.low : lhs.high < rhs.high;
        }

        LUNA_GUI_ASSET_API R<Variant> serialize_asset(const Asset& asset)
        {
            lutry
            {
                Variant r(VariantType::object);
                r[name_version] = (u64)asset.version;
                luset(r[name_root], serialize(get_root(&asset)));
                Variant nodes(VariantType::array);
                Vector<Guid> serialized_ids;
                luexp(append_serialized_node(asset, get_root(&asset), serialized_ids, nodes));
                Vector<Guid> remaining_ids;
                for(const auto& pair : AssetTopologyAccess::nodes(&asset))
                {
                    if(!contains_guid(serialized_ids, pair.first))
                    {
                        remaining_ids.push_back(pair.first);
                    }
                }
                sort(remaining_ids.begin(), remaining_ids.end(), guid_less);
                for(const Guid& id : remaining_ids)
                {
                    luexp(append_serialized_node(asset, id, serialized_ids, nodes));
                }
                r[name_nodes] = move(nodes);
                return r;
            }
            lucatchret;
        }

        LUNA_GUI_ASSET_API R<Ref<Asset>> deserialize_asset(const Variant& data)
        {
            lutry
            {
                Ref<Asset> asset = new_object<Asset>();
                asset->version = (u32)data[name_version].unum(1);
                if(data[name_root].valid())
                {
                    Guid root;
                    luexp(deserialize(root, data[name_root]));
                    AssetTopologyAccess::set_root(asset.get(), root);
                }
                for(const Variant& node_data : data[name_nodes].values())
                {
                    lulet(node, deserialize_node(node_data));
                    if(node->id == Guid(0, 0))
                    {
                        node->id = random_guid();
                    }
                    AssetTopologyAccess::nodes(asset.get()).insert_or_assign(node->id, node);
                }
                for(auto& pair : AssetTopologyAccess::nodes(asset.get()))
                {
                    Ref<Node>& node = pair.second;
                    if(!node)
                    {
                        continue;
                    }
                    Vector<Guid> valid_children;
                    for(const Guid& child_id : get_children(node.get()))
                    {
                        Ref<Node> child = find_node(asset.get(), child_id);
                        if(child && child_id != node->id)
                        {
                            AssetTopologyAccess::set_parent(child.get(), node->id);
                            valid_children.push_back(child_id);
                        }
                    }
                    AssetTopologyAccess::children(node.get()) = move(valid_children);
                }
                return asset;
            }
            lucatchret;
        }

        LUNA_GUI_ASSET_API RV save_asset_to_json_file(const Asset& asset, const Path& path)
        {
            lutry
            {
                lulet(file, VFS::open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
                lulet(data, serialize_asset(asset));
                luexp(VariantUtils::write_json(file, data));
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_ASSET_API R<Ref<Asset>> load_asset_from_json_file(const Path& path)
        {
            lutry
            {
                lulet(file, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
                lulet(data, VariantUtils::read_json(file));
                return deserialize_asset(data);
            }
            lucatchret;
        }

        static void collect_referred_assets(const Asset& asset, const Node& node, Vector<Luna::Asset::asset_t>& referred_assets)
        {
            auto desc = get_node_type(node.type);
            if(succeeded(desc) && desc.get().on_get_referred_assets)
            {
                desc.get().on_get_referred_assets(node, referred_assets);
            }
            for(const Guid& child_id : get_children(&node))
            {
                Ref<Node> child = find_node(&asset, child_id);
                if(child)
                {
                    collect_referred_assets(asset, *child.get(), referred_assets);
                }
            }
        }

        LUNA_GUI_ASSET_API void get_referred_assets(const Asset& asset, Vector<Luna::Asset::asset_t>& referred_assets)
        {
            Ref<Node> root = find_node(&asset, get_root(&asset));
            if(root)
            {
                collect_referred_assets(asset, *root.get(), referred_assets);
            }
        }

        static R<ObjRef> load_gui_asset(object_t, Luna::Asset::asset_t, const Path& path)
        {
            lutry
            {
                Path json_path = path;
                json_path.append_extension("json");
                lulet(asset, load_asset_from_json_file(json_path));
                return ObjRef(asset.object());
            }
            lucatchret;
        }

        static R<ObjRef> create_default_gui_asset(object_t, Luna::Asset::asset_t)
        {
            return ObjRef(new_asset().object());
        }

        static RV save_gui_asset(object_t, Luna::Asset::asset_t, const Path& path, object_t data)
        {
            Ref<Asset> asset = ObjRef(data);
            if(!asset)
            {
                return BasicError::bad_arguments();
            }
            Path json_path = path;
            json_path.append_extension("json");
            return save_asset_to_json_file(*asset.get(), json_path);
        }

        static void get_gui_asset_referred_assets(object_t, Luna::Asset::asset_t asset_handle, Vector<Luna::Asset::asset_t>& referred_assets)
        {
            Ref<Asset> asset = Luna::Asset::get_asset_data<Asset>(asset_handle);
            if(asset)
            {
                get_referred_assets(*asset.get(), referred_assets);
            }
        }

        struct ModuleImpl : Module
        {
            virtual const c8* get_name() override { return "GUIAsset"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {GUI::module_gui(), module_asset(), module_variant_utils(), module_vfs()});
            }
            virtual RV on_init() override
            {
                Meta::register_GUIAsset_types();
                register_builtin_node_types();
                Luna::Asset::AssetTypeDesc desc;
                desc.name = asset_type_name();
                desc.on_load_asset = load_gui_asset;
                desc.on_load_asset_default_data = create_default_gui_asset;
                desc.on_save_asset = save_gui_asset;
                desc.on_get_referred_assets = get_gui_asset_referred_assets;
                Luna::Asset::register_asset_type(desc);
                return ok;
            }
            virtual void on_close() override
            {
                g_node_types.clear();
            }
        };

        LUNA_GUI_ASSET_API Module* module_gui_asset()
        {
            static ModuleImpl m;
            return &m;
        }
    }
}
