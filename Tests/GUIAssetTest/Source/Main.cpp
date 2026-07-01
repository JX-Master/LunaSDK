/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/10
*/
#include <Luna/Asset/Asset.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/Editor.hpp>
#include <Luna/GUIAsset/GUIAsset.hpp>
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Window.hpp>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace Luna;

namespace Luna
{
    namespace GA = GUIAsset;
    namespace LA = Asset;

    static constexpr const c8* name_asset = "asset";
    static constexpr const c8* name_background_mode = "background_mode";
    static constexpr const c8* name_cell_height = "cell_height";
    static constexpr const c8* name_cell_width = "cell_width";
    static constexpr const c8* name_checkable = "checkable";
    static constexpr const c8* name_checked = "checked";
    static constexpr const c8* name_clip_children = "clip_children";
    static constexpr const c8* name_column_separators = "column_separators";
    static constexpr const c8* name_columns = "columns";
    static constexpr const c8* name_current_item = "current_item";
    static constexpr const c8* name_fixed_row_height = "fixed_row_height";
    static constexpr const c8* name_fixed_row_height_mode = "fixed_row_height_mode";
    static constexpr const c8* name_fraction = "fraction";
    static constexpr const c8* name_gap_x = "gap_x";
    static constexpr const c8* name_gap_y = "gap_y";
    static constexpr const c8* name_items = "items";
    static constexpr const c8* name_max = "max";
    static constexpr const c8* name_min = "min";
    static constexpr const c8* name_multi_select = "multi_select";
    static constexpr const c8* name_open = "open";
    static constexpr const c8* name_overlay = "overlay";
    static constexpr const c8* name_popup_height = "popup_height";
    static constexpr const c8* name_popup_width = "popup_width";
    static constexpr const c8* name_row_separators = "row_separators";
    static constexpr const c8* name_selected = "selected";
    static constexpr const c8* name_sizing_mode = "sizing_mode";
    static constexpr const c8* name_size = "size";
    static constexpr const c8* name_speed = "speed";
    static constexpr const c8* name_shortcut = "shortcut";
    static constexpr const c8* name_tooltip_height = "tooltip_height";
    static constexpr const c8* name_tooltip_width = "tooltip_width";
    static constexpr const c8* name_value = "value";
    static constexpr const c8* name_virtualize_fixed_rows = "virtualize_fixed_rows";

    static Ref<GA::Node> must_node(const Name& type, const c8* label)
    {
        auto node = GA::new_node(type, label);
        lupanic_if_failed(node);
        return node.get();
    }

    static Variant make_size(f32 width, f32 height)
    {
        Variant r(VariantType::object);
        r[Name("width")] = (f64)width;
        r[Name("height")] = (f64)height;
        return r;
    }

    static Variant make_edge_insets(f32 left, f32 top, f32 right, f32 bottom)
    {
        Variant r(VariantType::object);
        r[Name("left")] = (f64)left;
        r[Name("top")] = (f64)top;
        r[Name("right")] = (f64)right;
        r[Name("bottom")] = (f64)bottom;
        return r;
    }

    static Variant make_items(Span<const c8*> values)
    {
        Variant r(VariantType::array);
        for(const c8* value : values)
        {
            r.push_back(value);
        }
        return r;
    }

    static Variant make_numbers(Span<const f64> values)
    {
        Variant r(VariantType::array);
        for(f64 value : values)
        {
            r.push_back(value);
        }
        return r;
    }

    static Ref<GA::Node> root_node(const Ref<GA::Asset>& asset)
    {
        luassert_always(asset && GA::get_root(asset.get()) != Guid(0, 0));
        Ref<GA::Node> node = GA::find_node(asset.get(), GA::get_root(asset.get()));
        luassert_always(node);
        return node;
    }

    static Ref<GA::Node> find_node_by_label(const Ref<GA::Asset>& asset, const Guid& id, const c8* label)
    {
        Ref<GA::Node> node = GA::find_node(asset.get(), id);
        if(!node)
        {
            return nullptr;
        }
        if(!std::strcmp(node->label.c_str(), label))
        {
            return node;
        }
        for(Guid child_id : GA::get_children(node.get()))
        {
            Ref<GA::Node> child = find_node_by_label(asset, child_id, label);
            if(child)
            {
                return child;
            }
        }
        return nullptr;
    }

    static Ref<GA::Node> find_node_by_label(const Ref<GA::Asset>& asset, const c8* label)
    {
        if(!asset || GA::get_root(asset.get()) == Guid(0, 0))
        {
            return nullptr;
        }
        return find_node_by_label(asset, GA::get_root(asset.get()), label);
    }

    static void add_child_node(const Ref<GA::Asset>& asset, const Guid& parent, const Ref<GA::Node>& node)
    {
        lupanic_if_failed(GA::add_node(asset.get(), node, parent));
    }

    static Ref<GA::Node> add_child(const Ref<GA::Asset>& asset, const Guid& parent, const Name& type, const c8* label)
    {
        Ref<GA::Node> node = must_node(type, label);
        add_child_node(asset, parent, node);
        return node;
    }

    static usize count_asset_nodes(const GA::Asset& asset)
    {
        return GA::get_node_count(&asset);
    }

    static bool contains_asset(const Vector<LA::asset_t>& assets, LA::asset_t asset)
    {
        for(LA::asset_t item : assets)
        {
            if(item == asset)
            {
                return true;
            }
        }
        return false;
    }

    static RV generate_test_widget_node_core(GUICore::IContext* context, GA::Node& node, const GA::GenerateContext&)
    {
        GUI::text(context, GA::node_core_id(node), node.label.c_str());
        return ok;
    }

    static void register_test_node_type()
    {
        GA::NodeTypeDesc desc;
        desc.type = "test_custom";
        desc.default_properties = Variant(VariantType::object);
        desc.on_generate_core = generate_test_widget_node_core;
        GA::register_node_type(desc);
    }

    static Ref<GA::Asset> make_nested_asset()
    {
        Ref<GA::Asset> asset = GA::new_asset();
        Ref<GA::Node> root = root_node(asset);
        add_child(asset, root->id, "text", "Nested GUIAsset text");
        add_child(asset, root->id, "button", "Nested Button");
        return asset;
    }

    static Ref<GA::Asset> make_sample_asset(LA::asset_t nested_asset)
    {
        Ref<GA::Asset> asset = GA::new_asset();
        Ref<GA::Node> root = root_node(asset);
        root->label = "GUIAsset Test Root";
        root->layout_config.width.kind = GUICore::SizeKind::percent;
        root->layout_config.width.value = 1.0f;
        root->layout_config.height.kind = GUICore::SizeKind::percent;
        root->layout_config.height.value = 1.0f;
        root->layout_config.flex_grow = 1.0f;
        root->has_layout_config = true;

        Ref<GA::Node> header = add_child(asset, root->id, "h_layout", "Header");
        add_child(asset, header->id, "text", "Hello GUIAsset");
        add_child(asset, header->id, "button", "Apply");
        Ref<GA::Node> progress = add_child(asset, header->id, "progress_bar", "Progress");
        progress->properties[name_fraction] = (f64)0.42;
        progress->properties[name_size] = make_size(160.0f, 20.0f);
        progress->properties[name_overlay] = "42%";

        Ref<GA::Node> controls = add_child(asset, root->id, "v_layout", "Controls");
        Ref<GA::Node> checkbox = add_child(asset, controls->id, "checkbox", "Checkbox");
        checkbox->properties[name_value] = true;
        Ref<GA::Node> radio = add_child(asset, controls->id, "radio_button", "Radio");
        radio->properties[name_selected] = true;
        Ref<GA::Node> toggle = add_child(asset, controls->id, "toggle_switch", "Switch");
        toggle->properties[name_value] = true;
        Ref<GA::Node> input = add_child(asset, controls->id, "input_text", "Input");
        input->properties[name_value] = "Editable text";
        add_child(asset, controls->id, "selectable", "Selectable");

        Ref<GA::Node> button_group = add_child(asset, root->id, "button_group", "Button Group");
        const c8* group_items[] = {"A", "B", "C"};
        button_group->properties[name_items] = make_items(Span<const c8*>(group_items, 3));
        button_group->properties[name_current_item] = (i64)1;

        Ref<GA::Node> multi_button_group = add_child(asset, root->id, "button_group", "Multi Button Group");
        multi_button_group->properties[name_items] = make_items(Span<const c8*>(group_items, 3));
        multi_button_group->properties[name_multi_select] = true;
        Variant selected(VariantType::array);
        selected.push_back(true);
        selected.push_back(false);
        selected.push_back(true);
        multi_button_group->properties[name_selected] = move(selected);

        Ref<GA::Node> combo = add_child(asset, root->id, "combo", "Combo");
        const c8* combo_items[] = {"Alpha", "Beta", "Gamma"};
        combo->properties[name_items] = make_items(Span<const c8*>(combo_items, 3));
        combo->properties[name_current_item] = (i64)2;
        combo->layout_config.width.kind = GUICore::SizeKind::fixed;
        combo->layout_config.width.value = 180.0f;
        combo->layout_config.height.kind = GUICore::SizeKind::fixed;
        combo->layout_config.height.value = 28.0f;
        combo->has_layout_config = true;

        Ref<GA::Node> numeric = add_child(asset, root->id, "v_layout", "Numeric");
        Ref<GA::Node> slider_float = add_child(asset, numeric->id, "slider_float", "Slider Float");
        slider_float->properties[name_value] = (f64)0.25;
        slider_float->properties[name_min] = (f64)0.0;
        slider_float->properties[name_max] = (f64)1.0;
        Ref<GA::Node> slider_int = add_child(asset, numeric->id, "slider_int", "Slider Int");
        slider_int->properties[name_value] = (i64)4;
        slider_int->properties[name_min] = (i64)0;
        slider_int->properties[name_max] = (i64)10;
        Ref<GA::Node> drag_float = add_child(asset, numeric->id, "drag_float", "Drag Float");
        drag_float->properties[name_value] = (f64)0.5;
        drag_float->properties[name_speed] = (f64)0.01;
        drag_float->properties[name_min] = (f64)0.0;
        drag_float->properties[name_max] = (f64)1.0;
        Ref<GA::Node> drag_int = add_child(asset, numeric->id, "drag_int", "Drag Int");
        drag_int->properties[name_value] = (i64)8;
        drag_int->properties[name_speed] = (f64)1.0;
        drag_int->properties[name_min] = (i64)0;
        drag_int->properties[name_max] = (i64)100;

        Ref<GA::Node> colors = add_child(asset, root->id, "v_layout", "Colors");
        Ref<GA::Node> color3 = add_child(asset, colors->id, "color_edit3", "Color Edit 3");
        const f64 color3_values[] = {0.2, 0.5, 0.9};
        color3->properties[name_value] = make_numbers(Span<const f64>(color3_values, 3));
        Ref<GA::Node> color4 = add_child(asset, colors->id, "color_edit4", "Color Edit 4");
        const f64 color4_values[] = {0.9, 0.25, 0.1, 0.75};
        color4->properties[name_value] = make_numbers(Span<const f64>(color4_values, 4));

        Ref<GA::Node> grid = add_child(asset, root->id, "grid_layout", "Grid");
        grid->properties[name_sizing_mode] = "fixed_columns";
        grid->properties[name_columns] = (u64)3;
        grid->properties[name_cell_width] = (f64)96.0;
        grid->properties[name_cell_height] = (f64)48.0;
        grid->properties[name_gap_x] = (f64)4.0;
        grid->properties[name_gap_y] = (f64)4.0;
        add_child(asset, grid->id, "button", "Grid A");
        add_child(asset, grid->id, "button", "Grid B");
        add_child(asset, grid->id, "button", "Grid C");

        Ref<GA::Node> canvas = add_child(asset, root->id, "canvas_layout", "Canvas");
        canvas->properties[name_size] = make_size(320.0f, 120.0f);
        canvas->properties[Name("padding")] = make_edge_insets(8.0f, 10.0f, 0.0f, 0.0f);
        canvas->properties[name_clip_children] = true;
        Ref<GA::Node> canvas_text = add_child(asset, canvas->id, "text", "Canvas Child");
        canvas_text->canvas_layout.anchor_min = Float2U(0.0f);
        canvas_text->canvas_layout.anchor_max = Float2U(0.0f);
        canvas_text->canvas_layout.offset = Float4U(24.0f, 24.0f, 204.0f, 52.0f);
        canvas_text->has_canvas_layout = true;

        Ref<GA::Node> table = add_child(asset, root->id, "table_layout", "Table");
        Variant columns(VariantType::array);
        columns.push_back((f64)96.0);
        columns.push_back((f64)180.0);
        columns.push_back((f64)96.0);
        table->properties[name_columns] = move(columns);
        table->properties[name_fixed_row_height_mode] = true;
        table->properties[name_fixed_row_height] = (f64)24.0;
        table->properties[name_virtualize_fixed_rows] = true;
        table->properties[name_row_separators] = true;
        table->properties[name_column_separators] = true;
        table->properties[name_background_mode] = "alternate_rows";
        for(u32 row_index = 0; row_index < 4; ++row_index)
        {
            c8 row_index_text[32];
            c8 row_name_text[64];
            std::snprintf(row_index_text, sizeof(row_index_text), "Row %u", row_index);
            std::snprintf(row_name_text, sizeof(row_name_text), "Table row %u", row_index);
            Ref<GA::Node> row = add_child(asset, table->id, "table_row", "Row");
            add_child(asset, row->id, "text", row_index_text);
            add_child(asset, row->id, "text", row_name_text);
            add_child(asset, row->id, "button", "Action");
        }

        Ref<GA::Node> tree = add_child(asset, root->id, "tree_node", "Tree");
        tree->properties[Name("default_open")] = true;
        add_child(asset, tree->id, "text", "Tree leaf text");

        Ref<GA::Node> collapsing = add_child(asset, root->id, "collapsing_header", "Collapsing");
        add_child(asset, collapsing->id, "text", "Collapsed body");

        Ref<GA::Node> image = add_child(asset, root->id, "image", "Image");
        image->properties[name_size] = make_size(32.0f, 32.0f);

        add_child(asset, root->id, "test_custom", "Custom external node");

        Ref<GA::Node> asset_ref = add_child(asset, root->id, "asset_reference", "Nested Asset Reference");
        auto serialized_asset = serialize(nested_asset);
        lupanic_if_failed(serialized_asset);
        asset_ref->properties[name_asset] = serialized_asset.get();

        Ref<GA::Node> scroll = add_child(asset, root->id, "scroll_view", "Scroll Area");
        scroll->properties[name_size] = make_size(160.0f, 40.0f);
        Ref<GA::Node> scroll_text = add_child(asset, scroll->id, "text", "Scrollable Body");
        scroll_text->layout_config.width.kind = GUICore::SizeKind::fixed;
        scroll_text->layout_config.width.value = 160.0f;
        scroll_text->layout_config.height.kind = GUICore::SizeKind::fixed;
        scroll_text->layout_config.height.value = 160.0f;
        scroll_text->has_layout_config = true;

        Ref<GA::Node> tabs = add_child(asset, root->id, "tab_bar", "Tabs");
        tabs->properties[name_size] = make_size(260.0f, 120.0f);
        Ref<GA::Node> first_tab = add_child(asset, tabs->id, "tab_item", "Tab One");
        add_child(asset, first_tab->id, "text", "Tab One Content");
        Ref<GA::Node> second_tab = add_child(asset, tabs->id, "tab_item", "Tab Two");
        second_tab->properties[name_open] = true;
        add_child(asset, second_tab->id, "text", "Tab Two Content");

        Ref<GA::Node> menu_bar = add_child(asset, root->id, "menu_bar", "Main Menu");
        menu_bar->properties[name_size] = make_size(280.0f, 28.0f);
        Ref<GA::Node> view_menu = add_child(asset, menu_bar->id, "menu", "View");
        view_menu->properties[name_popup_width] = (f64)190.0;
        view_menu->properties[name_popup_height] = (f64)72.0;
        Ref<GA::Node> show_grid = add_child(asset, view_menu->id, "menu_item", "Show Grid");
        show_grid->properties[name_checkable] = true;
        show_grid->properties[name_checked] = false;
        show_grid->properties[name_shortcut] = "Ctrl+G";
        add_child(asset, view_menu->id, "menu_separator", "View Separator");
        Ref<GA::Node> disabled_menu_item = add_child(asset, view_menu->id, "menu_item", "Disabled Item");
        disabled_menu_item->enabled = false;

        Ref<GA::Node> popup = add_child(asset, root->id, "popup", "Open Asset Popup");
        popup->properties[name_size] = make_size(150.0f, 28.0f);
        popup->properties[name_popup_width] = (f64)190.0;
        popup->properties[name_popup_height] = (f64)56.0;
        add_child(asset, popup->id, "text", "Asset Popup Content");

        Ref<GA::Node> tooltip = add_child(asset, root->id, "tooltip", "Hover Asset Tooltip");
        tooltip->properties[name_size] = make_size(170.0f, 28.0f);
        tooltip->properties[name_tooltip_width] = (f64)190.0;
        tooltip->properties[name_tooltip_height] = (f64)48.0;
        add_child(asset, tooltip->id, "text", "Asset Tooltip Content");

        return asset;
    }

    static void test_node_indexing_and_mutation(const Ref<GA::Asset>& asset)
    {
        Ref<GA::Node> root = root_node(asset);
        luassert_always(GA::get_child_count(root.get()) > 0);
        Ref<GA::Node> first_child = GA::find_node(asset.get(), GA::get_child(root.get(), 0));
        luassert_always(first_child && GA::get_parent(first_child.get()) == root->id);

        Ref<GA::Node> temp_parent = add_child(asset, root->id, "v_layout", "Mutation Parent");
        Ref<GA::Node> temp_child = add_child(asset, root->id, "button", "Move Me");
        luassert_always(GA::find_node(asset.get(), temp_child->id));

        lupanic_if_failed(GA::move_node(asset.get(), temp_child->id, temp_parent->id));
        luassert_always(GA::get_parent(temp_child.get()) == temp_parent->id);
        luassert_always(GA::get_child_count(temp_parent.get()) == 1 && GA::get_child(temp_parent.get(), 0) == temp_child->id);

        Ref<GA::Node> temp_second_child = add_child(asset, temp_parent->id, "button", "Move Me Too");
        lupanic_if_failed(GA::reorder_node(asset.get(), temp_second_child->id, 0));
        luassert_always(GA::get_child(temp_parent.get(), 0) == temp_second_child->id);
        luassert_always(GA::get_child(temp_parent.get(), 1) == temp_child->id);

        lupanic_if_failed(GA::remove_node(asset.get(), temp_parent->id));
        luassert_always(!GA::find_node(asset.get(), temp_parent->id));
        luassert_always(!GA::find_node(asset.get(), temp_child->id));
        luassert_always(!GA::find_node(asset.get(), temp_second_child->id));

        Ref<GA::Node> detached = add_child(asset, root->id, "button", "Detach Me");
        lupanic_if_failed(GA::detach_node(asset.get(), detached->id));
        luassert_always(GA::get_parent(detached.get()) == Guid(0, 0));
        luassert_always(GA::find_node(asset.get(), detached->id));
        lupanic_if_failed(GA::remove_node(asset.get(), detached->id));

        Ref<GA::Asset> root_asset = GA::new_asset();
        Ref<GA::Node> old_root = root_node(root_asset);
        Ref<GA::Node> new_root = add_child(root_asset, old_root->id, "v_layout", "New Root");
        lupanic_if_failed(GA::set_root(root_asset.get(), new_root->id));
        luassert_always(GA::get_root(root_asset.get()) == new_root->id);
        luassert_always(GA::get_parent(new_root.get()) == Guid(0, 0));
        luassert_always(GA::get_child_count(old_root.get()) == 0);
    }

    static void test_serialization_and_file_io(const GA::Asset& asset)
    {
        auto serialized = GA::serialize_asset(asset);
        lupanic_if_failed(serialized);
        String json = VariantUtils::write_json(serialized.get());
        auto parsed = VariantUtils::read_json(json.c_str(), json.size());
        lupanic_if_failed(parsed);
        auto deserialized = GA::deserialize_asset(parsed.get());
        lupanic_if_failed(deserialized);
        luassert_always(GA::get_root(deserialized.get().get()) != Guid(0, 0));
        Ref<GA::Node> loaded_root = GA::find_node(deserialized.get().get(), GA::get_root(deserialized.get().get()));
        Ref<GA::Node> source_root = GA::find_node(&asset, GA::get_root(&asset));
        luassert_always(loaded_root && source_root);
        luassert_always(loaded_root->id == source_root->id);
        luassert_always(count_asset_nodes(*deserialized.get().get()) == count_asset_nodes(asset));

        lupanic_if_failed(VFS::mount(VFS::get_platform_filesystem_driver(), "/tmp", "/tmp"));
        Path path("/tmp/luna_gui_asset_test.guiasset.json");
        Ref<GA::Asset> roundtripped = deserialized.get();
        lupanic_if_failed(GA::save_asset_to_json_file(*roundtripped.get(), path));
        auto from_file = GA::load_asset_from_json_file(path);
        lupanic_if_failed(from_file);
        luassert_always(GA::get_root(from_file.get().get()) == GA::get_root(&asset));
        luassert_always(count_asset_nodes(*from_file.get().get()) == count_asset_nodes(asset));
        (void)VFS::delete_file(path);
        (void)VFS::unmount("/tmp");
    }

    static void test_referred_assets(LA::asset_t owner_asset, LA::asset_t nested_asset, const Ref<GA::Asset>& asset)
    {
        lupanic_if_failed(LA::set_asset_data(owner_asset, asset.object()));
        Vector<LA::asset_t> referred_assets;
        LA::get_referred_assets(owner_asset, referred_assets);
        luassert_always(contains_asset(referred_assets, nested_asset));

        referred_assets.clear();
        GA::get_referred_assets(*asset.get(), referred_assets);
        luassert_always(contains_asset(referred_assets, nested_asset));
    }

    static bool has_debug_element(const GUICore::DebugInfo& debug_info, const c8* debug_name)
    {
        Name name(debug_name);
        for(const GUICore::DebugElementInfo& element : debug_info.elements)
        {
            if(element.debug_name == name)
            {
                return true;
            }
        }
        return false;
    }

    static const GUICore::DebugElementInfo* find_debug_element(const GUICore::DebugInfo& debug_info, const c8* debug_name)
    {
        Name name(debug_name);
        for(const GUICore::DebugElementInfo& element : debug_info.elements)
        {
            if(element.debug_name == name)
            {
                return &element;
            }
        }
        return nullptr;
    }

    static bool close_to(f32 lhs, f32 rhs)
    {
        return std::fabs(lhs - rhs) <= 0.01f;
    }

    static void assert_rect(const GUICore::DebugElementInfo& element, f32 x, f32 y, f32 width, f32 height)
    {
        if(!close_to(element.rect.offset_x, x) || !close_to(element.rect.offset_y, y) ||
            !close_to(element.rect.width, width) || !close_to(element.rect.height, height))
        {
            std::printf("Rect mismatch for %s: actual %.2f %.2f %.2f %.2f, expected %.2f %.2f %.2f %.2f\n",
                element.debug_name.c_str(), element.rect.offset_x, element.rect.offset_y, element.rect.width, element.rect.height,
                x, y, width, height);
        }
        luassert_always(close_to(element.rect.offset_x, x));
        luassert_always(close_to(element.rect.offset_y, y));
        luassert_always(close_to(element.rect.width, width));
        luassert_always(close_to(element.rect.height, height));
    }

    static GUICore::InputEvent pointer_event(GUICore::InputEventType type, const Float2U& position)
    {
        GUICore::InputEvent event;
        event.type = type;
        event.position = position;
        event.button = GUICore::PointerButton::left;
        return event;
    }

    static void add_pointer_click(GUICore::IContext* context, const Float2U& position)
    {
        context->add_input_event(pointer_event(GUICore::InputEventType::pointer_move, position));
        context->add_input_event(pointer_event(GUICore::InputEventType::pointer_down, position));
        context->add_input_event(pointer_event(GUICore::InputEventType::pointer_up, position));
    }

    static i32 runtime_i32(const Ref<GA::Node>& node, const Name& key, i32 default_value)
    {
        auto iter = node->runtime_values.find(key);
        if(iter == node->runtime_values.end())
        {
            return default_value;
        }
        const i32* value = iter->second.as<i32>();
        return value ? *value : default_value;
    }

    static bool runtime_bool(const Ref<GA::Node>& node, const Name& key, bool default_value)
    {
        auto iter = node->runtime_values.find(key);
        if(iter == node->runtime_values.end())
        {
            return default_value;
        }
        const bool* value = iter->second.as<bool>();
        return value ? *value : default_value;
    }

    static Float2U debug_element_screen_point(const GUICore::DebugInfo& debug_info,
        const GUICore::DebugElementInfo& element, const Float2U& local_offset)
    {
        Float2U layer_position(0.0f);
        if(element.layer < debug_info.layers.size())
        {
            layer_position = debug_info.layers[element.layer].screen_position;
        }
        return Float2U(layer_position.x + element.rect.offset_x + local_offset.x,
            layer_position.y + element.rect.offset_y + local_offset.y);
    }

    static void test_generate_core(const Ref<GA::Asset>& asset)
    {
        Ref<GUICore::IContext> context = GUICore::new_context();
        luassert_always(context);
        GUICore::FrameDesc frame_desc;
        frame_desc.screen_size = Float2U(800.0f, 600.0f);
        frame_desc.framebuffer_size = UInt2U(800, 600);
        frame_desc.delta_time = 1.0f / 60.0f;
        GA::GenerateContext generate_context;
        generate_context.core_root_rect = RectF(0.0f, 0.0f, 800.0f, 600.0f);
        auto build_frame = [&]() {
            context->push_layer(1, Float2U(0.0f), "GUIAssetTest");
            lupanic_if_failed(GA::generate(context.get(), asset.get(), generate_context));
            context->pop_layer();
            context->route_input();
        };

        context->begin_frame(frame_desc);
        build_frame();

        GUICore::DebugInfo debug_info = context->dump_debug_info();
        luassert_always(debug_info.layers.size() == 1);
        luassert_always(debug_info.elements.size() >= 24);
        luassert_always(debug_info.draw_commands.size() > 0);
        luassert_always(has_debug_element(debug_info, "Custom external node"));
        luassert_always(has_debug_element(debug_info, "Nested GUIAsset text"));
        luassert_always(has_debug_element(debug_info, "Color Edit 3"));
        luassert_always(has_debug_element(debug_info, "Color Edit 4"));
        luassert_always(has_debug_element(debug_info, "Tab One Content"));
        luassert_always(!has_debug_element(debug_info, "Tab Two Content"));
        luassert_always(has_debug_element(debug_info, "View"));
        luassert_always(!has_debug_element(debug_info, "Show Grid"));
        luassert_always(has_debug_element(debug_info, "Open Asset Popup"));
        luassert_always(!has_debug_element(debug_info, "Asset Popup Content"));
        luassert_always(has_debug_element(debug_info, "Hover Asset Tooltip"));
        luassert_always(!has_debug_element(debug_info, "Asset Tooltip Content"));

        const GUICore::DebugElementInfo* header = find_debug_element(debug_info, "Header");
        luassert_always(header);
        luassert_always(header->rect.height < 200.0f);
        luassert_always(header->rect.width <= 800.0f);

        const GUICore::DebugElementInfo* canvas_child = find_debug_element(debug_info, "Canvas Child");
        luassert_always(canvas_child);
        assert_rect(*canvas_child, 32.0f, 110.0f, 180.0f, 28.0f);

        const GUICore::DebugElementInfo* first_index_cell = find_debug_element(debug_info, "Row 0");
        const GUICore::DebugElementInfo* first_name_cell = find_debug_element(debug_info, "Table row 0");
        const GUICore::DebugElementInfo* second_index_cell = find_debug_element(debug_info, "Row 1");
        luassert_always(first_index_cell);
        luassert_always(first_name_cell);
        luassert_always(second_index_cell);
        assert_rect(*first_index_cell, 0.0f, 202.0f, 96.0f, 24.0f);
        assert_rect(*first_name_cell, 96.0f, 202.0f, 180.0f, 24.0f);
        assert_rect(*second_index_cell, 0.0f, 226.0f, 96.0f, 24.0f);

        const GUICore::DebugElementInfo* scroll_area = find_debug_element(debug_info, "Scroll Area");
        const GUICore::DebugElementInfo* scroll_body = find_debug_element(debug_info, "Scrollable Body");
        const GUICore::DebugElementInfo* combo = find_debug_element(debug_info, "Combo");
        luassert_always(scroll_area);
        luassert_always(scroll_body);
        luassert_always(combo);
        f32 scroll_body_initial_y = scroll_body->rect.offset_y;

        Ref<GA::Node> combo_node = find_node_by_label(asset, "Combo");
        luassert_always(combo_node);
        luassert_always(runtime_i32(combo_node, Name("current_item"), -1) == 2);

        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *combo, Float2U(12.0f)));
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        luassert_always(debug_info.layers.size() == 2);
        const GUICore::DebugElementInfo* combo_beta = find_debug_element(debug_info, "Beta");
        luassert_always(combo_beta);

        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *combo_beta, Float2U(12.0f)));
        build_frame();
        luassert_always(runtime_i32(combo_node, Name("current_item"), -1) == 2);

        context->begin_frame(frame_desc);
        build_frame();
        luassert_always(runtime_i32(combo_node, Name("current_item"), -1) == 1);

        context->begin_frame(frame_desc);
        GUICore::InputEvent wheel;
        wheel.type = GUICore::InputEventType::pointer_wheel;
        wheel.position = Float2U(scroll_area->rect.offset_x + 8.0f, scroll_area->rect.offset_y + 8.0f);
        wheel.wheel_delta = Float2U(0.0f, -8.0f);
        context->add_input_event(wheel);
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        scroll_body = find_debug_element(debug_info, "Scrollable Body");
        luassert_always(scroll_body);
        luassert_always(close_to(scroll_body->rect.offset_y, scroll_body_initial_y - 120.0f));
        Ref<GA::Node> scroll_node = find_node_by_label(asset, "Scroll Area");
        luassert_always(scroll_node);
        object_t scroll_state_object = context->get_state(GUICore::make_state_id<GUI::CoreScrollViewState>(GA::node_core_id(*scroll_node.get())));
        luassert_always(scroll_state_object);
        GUI::CoreScrollViewState* scroll_state = cast_object<GUI::CoreScrollViewState>(scroll_state_object);
        luassert_always(scroll_state && close_to(scroll_state->scroll.y, 120.0f));

        const GUICore::DebugElementInfo* second_tab = find_debug_element(debug_info, "Tab Two");
        luassert_always(second_tab);
        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *second_tab, Float2U(12.0f)));
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        luassert_always(!has_debug_element(debug_info, "Tab One Content"));
        luassert_always(has_debug_element(debug_info, "Tab Two Content"));

        const GUICore::DebugElementInfo* view_menu = find_debug_element(debug_info, "View");
        luassert_always(view_menu);
        Ref<GA::Node> show_grid_node = find_node_by_label(asset, "Show Grid");
        luassert_always(show_grid_node);
        luassert_always(!runtime_bool(show_grid_node, Name("checked"), false));

        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *view_menu, Float2U(12.0f)));
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        luassert_always(debug_info.layers.size() == 2);
        const GUICore::DebugElementInfo* show_grid_item = find_debug_element(debug_info, "Show Grid");
        luassert_always(show_grid_item);

        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *show_grid_item, Float2U(36.0f, 13.0f)));
        build_frame();
        luassert_always(!runtime_bool(show_grid_node, Name("checked"), false));

        context->begin_frame(frame_desc);
        build_frame();
        luassert_always(runtime_bool(show_grid_node, Name("checked"), false));

        debug_info = context->dump_debug_info();
        const GUICore::DebugElementInfo* tooltip_owner = find_debug_element(debug_info, "Hover Asset Tooltip");
        luassert_always(tooltip_owner);
        context->begin_frame(frame_desc);
        context->add_input_event(pointer_event(GUICore::InputEventType::pointer_move,
            debug_element_screen_point(debug_info, *tooltip_owner, Float2U(12.0f))));
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        luassert_always(debug_info.layers.size() == 2);
        luassert_always(has_debug_element(debug_info, "Asset Tooltip Content"));

        const GUICore::DebugElementInfo* popup_trigger = find_debug_element(debug_info, "Open Asset Popup");
        luassert_always(popup_trigger);
        context->begin_frame(frame_desc);
        add_pointer_click(context.get(), debug_element_screen_point(debug_info, *popup_trigger, Float2U(12.0f)));
        build_frame();

        context->begin_frame(frame_desc);
        build_frame();
        debug_info = context->dump_debug_info();
        luassert_always(debug_info.layers.size() == 2);
        luassert_always(has_debug_element(debug_info, "Asset Popup Content"));
    }

    static void run_gui_asset_tests()
    {
        register_test_node_type();
        LA::asset_t nested_handle = LA::get_asset(Guid("{D9602558-52C4-4108-82D7-3EE015067203}"));
        LA::asset_t owner_handle = LA::get_asset(Guid("{99F77688-88D2-496D-B4E5-B129B32AC110}"));
        lupanic_if_failed(LA::register_asset(nested_handle, GA::asset_type_name()));
        lupanic_if_failed(LA::register_asset(owner_handle, GA::asset_type_name()));

        Ref<GA::Asset> nested_asset = make_nested_asset();
        lupanic_if_failed(LA::set_asset_data(nested_handle, nested_asset.object()));
        Ref<GA::Asset> asset = make_sample_asset(nested_handle);

        test_node_indexing_and_mutation(asset);
        test_serialization_and_file_io(*asset.get());
        test_referred_assets(owner_handle, nested_handle, asset);
        test_generate_core(asset);
    }
}

int luna_main(int, const char**)
{
    Luna::init();
    lupanic_if_failed(add_modules({
        module_window(),
        module_rhi(),
        module_font(),
        module_vg(),
        GUICore::module_gui_core(),
        GUI::module_gui(),
        module_asset(),
        module_variant_utils(),
        module_vfs(),
        GUIAsset::module_gui_asset()
    }));
    lupanic_if_failed(init_modules());
    set_log_to_platform_enabled(true);
    set_log_to_platform_verbosity(LogVerbosity::warning);
    run_gui_asset_tests();
    Luna::close();
    std::printf("GUIAssetTest passed.\n");
    return 0;
}
