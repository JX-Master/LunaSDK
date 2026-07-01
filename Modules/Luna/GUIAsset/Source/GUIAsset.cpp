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
#include <Luna/GUI/Editor.hpp>
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

        enum class AssetSizePolicy : u8
        {
            fixed,
            hug,
            fill
        };

        struct AssetSize
        {
            f32 width = 0.0f;
            f32 height = 0.0f;

            static AssetSize fixed(f32 width, f32 height)
            {
                AssetSize r;
                r.width = width;
                r.height = height;
                return r;
            }
        };

        struct AssetEdgeInsets
        {
            f32 left = 0.0f;
            f32 top = 0.0f;
            f32 right = 0.0f;
            f32 bottom = 0.0f;

            static AssetEdgeInsets all(f32 value)
            {
                AssetEdgeInsets r;
                r.left = value;
                r.top = value;
                r.right = value;
                r.bottom = value;
                return r;
            }
        };

        struct AssetLayoutDesc
        {
            AssetEdgeInsets padding;
            f32 gap = 6.0f;
        };

        struct AssetGridLayoutDesc
        {
            Float2U cell_size = Float2U(96.0f, 118.0f);
            u32 columns = 4;
            AssetEdgeInsets padding = AssetEdgeInsets::all(6.0f);
            Float2U gap = Float2U(8.0f, 8.0f);
        };

        struct AssetCanvasLayoutDesc
        {
            AssetEdgeInsets padding;
        };

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

        static Vector<f32> read_float_items(const Variant& data, Span<const f32> default_values)
        {
            Vector<f32> items;
            for(const Variant& item : data.values())
            {
                items.push_back((f32)item.fnum(0.0));
            }
            if(items.empty())
            {
                items.assign(default_values.begin(), default_values.end());
            }
            return items;
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

        static AssetSize read_size(const Variant& data, const AssetSize& default_value = AssetSize())
        {
            AssetSize r = default_value;
            r.width = (f32)data[Name("width")].fnum(r.width);
            r.height = (f32)data[Name("height")].fnum(r.height);
            return r;
        }

        static Variant write_size(const AssetSize& value)
        {
            Variant r(VariantType::object);
            r[name_width()] = (f64)value.width;
            r[Name("height")] = (f64)value.height;
            return r;
        }

        static AssetEdgeInsets read_edge_insets(const Variant& data, const AssetEdgeInsets& default_value = AssetEdgeInsets())
        {
            AssetEdgeInsets r = default_value;
            r.left = (f32)data[name_left()].fnum(r.left);
            r.top = (f32)data[name_top()].fnum(r.top);
            r.right = (f32)data[name_right()].fnum(r.right);
            r.bottom = (f32)data[name_bottom()].fnum(r.bottom);
            return r;
        }

        static Variant write_edge_insets(const AssetEdgeInsets& value)
        {
            Variant r(VariantType::object);
            r[name_left()] = (f64)value.left;
            r[name_top()] = (f64)value.top;
            r[name_right()] = (f64)value.right;
            r[name_bottom()] = (f64)value.bottom;
            return r;
        }

        static Variant write_edge_insets(const Float4U& value)
        {
            AssetEdgeInsets insets;
            insets.left = value.x;
            insets.top = value.y;
            insets.right = value.z;
            insets.bottom = value.w;
            return write_edge_insets(insets);
        }

        static AssetLayoutDesc read_layout_desc(const Variant& data)
        {
            AssetLayoutDesc r;
            r.padding = read_edge_insets(data[Name("padding")], r.padding);
            r.gap = (f32)data[Name("gap")].fnum(r.gap);
            return r;
        }

        static Variant write_core_size_value(const GUICore::SizeValue& value)
        {
            Variant r(VariantType::object);
            r[Name("kind")] = (u64)value.kind;
            r[Name("value")] = (f64)value.value;
            r[Name("min")] = (f64)value.min;
            r[Name("max")] = (f64)value.max;
            return r;
        }

        static GUICore::SizeValue read_core_size_value(const Variant& data, const GUICore::SizeValue& default_value)
        {
            GUICore::SizeValue r = default_value;
            r.kind = (GUICore::SizeKind)data[Name("kind")].unum((u64)r.kind);
            r.value = (f32)data[Name("value")].fnum(r.value);
            r.min = (f32)data[Name("min")].fnum(r.min);
            r.max = (f32)data[Name("max")].fnum(r.max);
            return r;
        }

        static Variant write_layout_input(const GUICore::LayoutInput& value)
        {
            Variant r(VariantType::object);
            r[Name("width")] = write_core_size_value(value.width);
            r[Name("height")] = write_core_size_value(value.height);
            r[Name("margin")] = write_edge_insets(value.margin);
            r[Name("padding")] = write_edge_insets(value.padding);
            return r;
        }

        static GUICore::LayoutInput read_layout_input(const Variant& data)
        {
            GUICore::LayoutInput r;
            if(data[Name("width")].type() == VariantType::object)
            {
                r.width = read_core_size_value(data[Name("width")], r.width);
            }
            else if(data[Name("width_policy")].valid())
            {
                AssetSizePolicy policy = (AssetSizePolicy)data[Name("width_policy")].unum((u64)AssetSizePolicy::hug);
                if(policy == AssetSizePolicy::fixed)
                {
                    r.width.kind = GUICore::SizeKind::fixed;
                    r.width.value = (f32)data[Name("fixed_width")].fnum(0.0);
                }
                else if(policy == AssetSizePolicy::fill)
                {
                    r.width.kind = GUICore::SizeKind::percent;
                    r.width.value = 1.0f;
                    r.flex_grow = max(r.flex_grow, (f32)data[Name("fill_weight_x")].fnum(1.0));
                }
            }
            if(data[Name("height")].type() == VariantType::object)
            {
                r.height = read_core_size_value(data[Name("height")], r.height);
            }
            else if(data[Name("height_policy")].valid())
            {
                AssetSizePolicy policy = (AssetSizePolicy)data[Name("height_policy")].unum((u64)AssetSizePolicy::hug);
                if(policy == AssetSizePolicy::fixed)
                {
                    r.height.kind = GUICore::SizeKind::fixed;
                    r.height.value = (f32)data[Name("fixed_height")].fnum(0.0);
                }
                else if(policy == AssetSizePolicy::fill)
                {
                    r.height.kind = GUICore::SizeKind::percent;
                    r.height.value = 1.0f;
                    r.flex_grow = max(r.flex_grow, (f32)data[Name("fill_weight_y")].fnum(1.0));
                }
            }
            AssetEdgeInsets margin = read_edge_insets(data[Name("margin")]);
            AssetEdgeInsets padding = read_edge_insets(data[Name("padding")]);
            r.margin = Float4U(margin.left, margin.top, margin.right, margin.bottom);
            r.padding = Float4U(padding.left, padding.top, padding.right, padding.bottom);
            return r;
        }

        static Variant write_canvas_layout(const GUICore::CanvasLayoutItem& value)
        {
            Variant r(VariantType::object);
            r[Name("anchor_min_x")] = (f64)value.anchor_min.x;
            r[Name("anchor_min_y")] = (f64)value.anchor_min.y;
            r[Name("anchor_max_x")] = (f64)value.anchor_max.x;
            r[Name("anchor_max_y")] = (f64)value.anchor_max.y;
            r[Name("offset_min_x")] = (f64)value.offset.x;
            r[Name("offset_min_y")] = (f64)value.offset.y;
            r[Name("offset_max_x")] = (f64)value.offset.z;
            r[Name("offset_max_y")] = (f64)value.offset.w;
            r[Name("pivot_x")] = (f64)value.pivot.x;
            r[Name("pivot_y")] = (f64)value.pivot.y;
            return r;
        }

        static GUICore::CanvasLayoutItem read_canvas_layout(const Variant& data)
        {
            GUICore::CanvasLayoutItem r;
            r.anchor_min.x = (f32)data[Name("anchor_min_x")].fnum(r.anchor_min.x);
            r.anchor_min.y = (f32)data[Name("anchor_min_y")].fnum(r.anchor_min.y);
            r.anchor_max.x = (f32)data[Name("anchor_max_x")].fnum(r.anchor_max.x);
            r.anchor_max.y = (f32)data[Name("anchor_max_y")].fnum(r.anchor_max.y);
            r.offset.x = (f32)data[Name("offset_min_x")].fnum(r.offset.x);
            r.offset.y = (f32)data[Name("offset_min_y")].fnum(r.offset.y);
            r.offset.z = (f32)data[Name("offset_max_x")].fnum(r.offset.z);
            r.offset.w = (f32)data[Name("offset_max_y")].fnum(r.offset.w);
            r.pivot.x = (f32)data[Name("pivot_x")].fnum(r.pivot.x);
            r.pivot.y = (f32)data[Name("pivot_y")].fnum(r.pivot.y);
            return r;
        }

        LUNA_GUI_ASSET_API GUICore::id_t node_core_id(const Node& node)
        {
            GUICore::id_t id = (GUICore::id_t)hash<Guid>()(node.id);
            return id ? id : 1;
        }

        static GUICore::id_t derived_core_id(const Node& node, const c8* local_name)
        {
            return GUICore::make_scoped_id(node_core_id(node), local_name);
        }

        static GUICore::LayoutInput read_core_layout_input(const Node& node)
        {
            GUICore::LayoutInput input;
            AssetEdgeInsets padding = read_edge_insets(node.properties[Name("padding")]);
            input.padding = Float4U(padding.left, padding.top, padding.right, padding.bottom);
            if(node.has_layout_input)
            {
                input = node.layout_input;
                if(padding.left != 0.0f || padding.top != 0.0f || padding.right != 0.0f || padding.bottom != 0.0f)
                {
                    input.padding = Float4U(padding.left, padding.top, padding.right, padding.bottom);
                }
            }
            return input;
        }

        static GUICore::LayoutInput read_core_layout_input(const Node& node, const AssetSize& size)
        {
            GUICore::LayoutInput input = read_core_layout_input(node);
            if(size.width > 0.0f)
            {
                input.width.kind = GUICore::SizeKind::fixed;
                input.width.value = size.width;
            }
            if(size.height > 0.0f)
            {
                input.height.kind = GUICore::SizeKind::fixed;
                input.height.value = size.height;
            }
            return input;
        }

        static GUICore::LayoutInput fixed_core_layout(f32 width, f32 height)
        {
            GUICore::LayoutInput input;
            if(width > 0.0f)
            {
                input.width.kind = GUICore::SizeKind::fixed;
                input.width.value = width;
            }
            if(height > 0.0f)
            {
                input.height.kind = GUICore::SizeKind::fixed;
                input.height.value = height;
            }
            return input;
        }

        static RectF core_generation_rect(GUICore::IContext* context, const GenerateContext& generate_context)
        {
            RectF rect = generate_context.core_root_rect;
            if(rect.width <= 0.0f || rect.height <= 0.0f)
            {
                GUICore::FrameDesc frame_desc = context->get_frame_desc();
                rect = RectF(0.0f, 0.0f, frame_desc.screen_size.x, frame_desc.screen_size.y);
            }
            if(rect.width <= 0.0f || rect.height <= 0.0f)
            {
                rect = RectF(0.0f, 0.0f, 800.0f, 600.0f);
            }
            return rect;
        }

        static void apply_core_common_modifiers(GUICore::IContext* context, const Node& node)
        {
            if(!node.style.empty())
            {
                context->push_style(node.style);
            }
        }

        static void finish_core_common_modifiers(GUICore::IContext* context, const Node& node)
        {
            if(!node.style.empty())
            {
                context->pop_style();
            }
        }

        static void apply_core_enabled(GUICore::IContext* context, const Node& node, const GUICore::ElementHandle& element)
        {
            if(node.enabled || element.index == GUICore::INVALID_ELEMENT)
            {
                return;
            }
            const GUICore::Element* core_element = context->get_element(element.index);
            if(!core_element)
            {
                return;
            }
            GUICore::Interactable interactable = core_element->interactable;
            set_flags(interactable.flags, GUICore::InteractableFlag::disabled);
            context->set_interactable(element, interactable);
        }

        static GUICore::GridLayoutDesc read_core_grid_layout_desc(const Node& node)
        {
            GUICore::GridLayoutDesc desc;
            const c8* mode = property_c_str(node, "sizing_mode", "fixed_cell_size");
            desc.mode = strcmp(mode, "fixed_columns") == 0 ? GUICore::GridLayoutMode::fixed_column_count : GUICore::GridLayoutMode::fixed_cell_size;
            desc.cell_size.x = property_f32(node, "cell_width", desc.cell_size.x);
            desc.cell_size.y = property_f32(node, "cell_height", desc.cell_size.y);
            desc.column_count = property_u32(node, "columns", desc.column_count);
            desc.gap.x = property_f32(node, "gap_x", desc.gap.x);
            desc.gap.y = property_f32(node, "gap_y", desc.gap.y);
            return desc;
        }

        static GUICore::TableTrackDesc core_table_track_pixels(f32 value)
        {
            GUICore::TableTrackDesc desc;
            desc.kind = GUICore::TableTrackSizeKind::pixels;
            desc.value = value;
            return desc;
        }

        static GUICore::TableTrackDesc core_table_track_fit()
        {
            GUICore::TableTrackDesc desc;
            desc.kind = GUICore::TableTrackSizeKind::fit;
            return desc;
        }

        static GUICore::CanvasLayoutItem core_canvas_item(const Node& node)
        {
            GUICore::CanvasLayoutItem item;
            item.element_id = node_core_id(node);
            item.anchor_min = node.canvas_layout.anchor_min;
            item.anchor_max = node.canvas_layout.anchor_max;
            item.offset = node.canvas_layout.offset;
            item.pivot = node.canvas_layout.pivot;
            return item;
        }

        static void apply_core_canvas_item_size(Node& node)
        {
            if(!node.has_canvas_layout)
            {
                return;
            }
            bool stretch_x = node.canvas_layout.anchor_min.x != node.canvas_layout.anchor_max.x;
            bool stretch_y = node.canvas_layout.anchor_min.y != node.canvas_layout.anchor_max.y;
            if(stretch_x && stretch_y)
            {
                return;
            }
            if(!stretch_x)
            {
                node.layout_input.width.kind = GUICore::SizeKind::fixed;
                node.layout_input.width.value = max(node.canvas_layout.offset.z - node.canvas_layout.offset.x, 1.0f);
            }
            if(!stretch_y)
            {
                node.layout_input.height.kind = GUICore::SizeKind::fixed;
                node.layout_input.height.value = max(node.canvas_layout.offset.w - node.canvas_layout.offset.y, 1.0f);
            }
            node.has_layout_input = true;
        }

        static void read_core_table_columns(const Node& node, Vector<GUICore::TableTrackDesc>& columns)
        {
            columns.clear();
            const Variant& values = property(node, "columns");
            for(const Variant& value : values.values())
            {
                columns.push_back(core_table_track_pixels((f32)value.fnum(0.0)));
            }
            if(columns.empty())
            {
                columns.push_back(core_table_track_fit());
            }
        }

        static void read_core_table_rows(const Node& node, usize row_count, Vector<GUICore::TableTrackDesc>& rows)
        {
            rows.clear();
            rows.reserve(row_count);
            bool fixed_height = property_bool(node, "fixed_row_height_mode", false);
            f32 row_height = property_f32(node, "fixed_row_height", 28.0f);
            for(usize i = 0; i < row_count; ++i)
            {
                rows.push_back(fixed_height ? core_table_track_pixels(row_height) : core_table_track_fit());
            }
        }

        static RV generate_core_h_layout(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_h_layout(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = read_layout_desc(node.properties).gap;
            return GUI::end_h_layout(context, layout, desc);
        }

        static RV generate_core_v_layout(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_v_layout(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = read_layout_desc(node.properties).gap;
            return GUI::end_v_layout(context, layout, desc);
        }

        static RV generate_core_scroll_view(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_scroll_view(context, node_core_id(node), node.label.c_str(),
                read_core_layout_input(node, read_size(node.properties[Name("size")])));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            return GUI::end_scroll_view(context, layout);
        }

        static RV generate_core_grid_layout(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_grid_layout(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            return GUI::end_grid_layout(context, layout, read_core_grid_layout_desc(node));
        }

        static RV generate_core_canvas_layout(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_canvas_layout(context, node_core_id(node), node.label.c_str(),
                read_core_layout_input(node, read_size(node.properties[Name("size")])));
            Vector<GUICore::CanvasLayoutItem> items;
            for(const Guid& child_id : AssetTopologyAccess::children(&node))
            {
                Ref<Node> child = find_node(generate_context.asset, child_id);
                if(!child)
                {
                    continue;
                }
                bool old_has_layout_input = child->has_layout_input;
                GUICore::LayoutInput old_layout_input = child->layout_input;
                if(child->has_canvas_layout)
                {
                    items.push_back(core_canvas_item(*child.get()));
                    apply_core_canvas_item_size(*child.get());
                }
                RV r = generate_node(context, *child.get(), generate_context);
                child->has_layout_input = old_has_layout_input;
                child->layout_input = old_layout_input;
                if(failed(r))
                {
                    context->end_element();
                    return r;
                }
            }
            GUICore::CanvasLayoutDesc desc;
            desc.items = Span<const GUICore::CanvasLayoutItem>(items.data(), items.size());
            desc.clip_children = property_bool(node, "clip_children", desc.clip_children);
            return GUI::end_canvas_layout(context, layout, desc);
        }

        static RV generate_core_table_layout(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_table_layout(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            Vector<GUICore::TableLayoutCell> cells;
            u32 row_index = 0;

            auto append_cell = [&](Node& cell_node, u32 column_index) -> RV {
                RV r = generate_node(context, cell_node, generate_context);
                if(failed(r))
                {
                    return r;
                }
                GUICore::TableLayoutCell cell;
                cell.element_id = node_core_id(cell_node);
                cell.row = row_index;
                cell.column = column_index;
                cells.push_back(cell);
                return ok;
            };

            for(const Guid& child_id : AssetTopologyAccess::children(&node))
            {
                auto child = find_node(generate_context.asset, child_id);
                if(!child)
                {
                    continue;
                }
                if(child->type == Name("table_row"))
                {
                    apply_core_common_modifiers(context, *child.get());
                    u32 column_index = 0;
                    for(const Guid& cell_id : AssetTopologyAccess::children(child.get()))
                    {
                        auto cell_node = find_node(generate_context.asset, cell_id);
                        if(!cell_node)
                        {
                            continue;
                        }
                        RV r = append_cell(*cell_node.get(), column_index++);
                        if(failed(r))
                        {
                            finish_core_common_modifiers(context, *child.get());
                            context->end_element();
                            return r;
                        }
                    }
                    finish_core_common_modifiers(context, *child.get());
                    ++row_index;
                }
                else
                {
                    RV r = append_cell(*child.get(), 0);
                    if(failed(r))
                    {
                        context->end_element();
                        return r;
                    }
                    ++row_index;
                }
            }

            Vector<GUICore::TableTrackDesc> columns;
            Vector<GUICore::TableTrackDesc> rows;
            read_core_table_columns(node, columns);
            read_core_table_rows(node, row_index, rows);

            GUICore::TableLayoutDesc desc;
            desc.columns = Span<const GUICore::TableTrackDesc>(columns.data(), columns.size());
            desc.rows = Span<const GUICore::TableTrackDesc>(rows.data(), rows.size());
            desc.cells = Span<const GUICore::TableLayoutCell>(cells.data(), cells.size());
            RV r = GUI::end_table_layout(context, layout, desc);
            if(failed(r))
            {
                return r;
            }
            return ok;
        }

        static RV generate_core_table_row(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle layout = GUI::begin_h_layout(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            GUICore::FlexLayoutDesc desc;
            desc.main_axis_gap = 0.0f;
            return GUI::end_h_layout(context, layout, desc);
        }

        static RV generate_core_text(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUICore::ElementHandle element = GUI::text(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_button(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUICore::ElementHandle element = GUI::text_button(context, node_core_id(node), node.label.c_str(), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_progress_bar(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            const Variant& overlay_value = property(node, "overlay");
            const c8* overlay = overlay_value.valid() ? overlay_value.c_str("") : nullptr;
            GUICore::ElementHandle element = GUI::progress_bar(context, node_core_id(node), property_f32(node, "fraction", 0.0f),
                overlay, read_core_layout_input(node, read_size(node.properties[Name("size")])));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_selectable(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUICore::ElementHandle element = GUI::selectable(context, node_core_id(node), node.label.c_str(),
                property_bool(node, "selected", false), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_checkbox(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            bool& value = runtime_value(node, Name("value"), property_bool(node, "value", false));
            GUICore::ElementHandle element = GUI::checkbox(context, node_core_id(node), node.label.c_str(), &value, read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_radio_button(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUICore::ElementHandle element = GUI::radio_button(context, node_core_id(node), node.label.c_str(),
                property_bool(node, "selected", false), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_toggle_switch(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            bool& value = runtime_value(node, Name("value"), property_bool(node, "value", false));
            GUICore::ElementHandle element = GUI::toggle_switch(context, node_core_id(node), node.label.c_str(), &value, read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_input_text(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            String default_value(property_c_str(node, "value", ""));
            String& value = runtime_value(node, Name("value"), default_value);
            GUICore::ElementHandle element = GUI::input_text(context, node_core_id(node), value, read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_image(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUICore::ElementHandle element = GUI::image(context, node_core_id(node), nullptr,
                read_core_layout_input(node, read_size(node.properties[Name("size")])));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_collapsing_header(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            if(GUI::collapsing_header(context, node_core_id(node), node.label.c_str(), true, read_core_layout_input(node)))
            {
                return generate_children(context, node, generate_context);
            }
            return ok;
        }

        static RV generate_core_tree_node(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::none;
            if(property_bool(node, "selected", false)) flags |= GUI::TreeNodeFlag::selected;
            if(property_bool(node, "leaf", false)) flags |= GUI::TreeNodeFlag::leaf;
            if(property_bool(node, "default_open", false)) flags |= GUI::TreeNodeFlag::default_open;
            if(GUI::tree_node(context, node_core_id(node), node.label.c_str(), flags, 0, read_core_layout_input(node)))
            {
                return generate_children(context, node, generate_context);
            }
            return ok;
        }

        static RV generate_core_tab_bar(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle tab_bar = GUI::begin_tab_bar(context, node_core_id(node), node.label.c_str(),
                GUI::TabBarFlag::fitting_shrink, read_core_layout_input(node, read_size(node.properties[Name("size")])));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            return GUI::end_tab_bar(context, tab_bar);
        }

        static RV generate_core_tab_item(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUI::TabItemFlag flags = GUI::TabItemFlag::none;
            if(property_bool(node, "selected", false))
            {
                flags |= GUI::TabItemFlag::selected;
            }
            bool open = property_bool(node, "open", true);
            if(GUI::begin_tab_item(context, node_core_id(node), node.label.c_str(), &open, flags))
            {
                RV r = generate_children(context, node, generate_context);
                GUI::end_tab_item(context);
                return r;
            }
            return ok;
        }

        static RV generate_core_menu_bar(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, node_core_id(node), node.label.c_str(),
                read_core_layout_input(node, read_size(node.properties[Name("size")])));
            RV r = generate_children(context, node, generate_context);
            if(failed(r))
            {
                context->end_element();
                return r;
            }
            return GUI::end_menu_bar(context, menu_bar);
        }

        static RV generate_core_menu(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle menu;
            if(GUI::begin_menu(context, node_core_id(node), node.label.c_str(), node.enabled,
                &menu, read_core_layout_input(node)))
            {
                RV r = generate_children(context, node, generate_context);
                if(failed(r))
                {
                    return r;
                }
                f32 popup_width = property_f32(node, "popup_width", 190.0f);
                f32 popup_height = property_f32(node, "popup_height", 240.0f);
                return GUI::end_menu(context, RectF(0.0f, 0.0f, popup_width, popup_height));
            }
            return ok;
        }

        static RV generate_core_menu_item(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            bool checkable = property_bool(node, "checkable", false);
            const c8* shortcut = property_c_str(node, "shortcut", "");
            GUICore::ElementHandle element;
            if(checkable)
            {
                bool& checked = runtime_value(node, Name("checked"), property_bool(node, "checked", false));
                element = GUI::menu_item(context, node_core_id(node), node.label.c_str(), shortcut, &checked,
                    node.enabled, read_core_layout_input(node));
            }
            else
            {
                element = GUI::menu_item(context, node_core_id(node), node.label.c_str(), shortcut,
                    property_bool(node, "checked", false), node.enabled, read_core_layout_input(node));
            }
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_menu_separator(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            GUI::menu_separator(context, node_core_id(node), read_core_layout_input(node));
            return ok;
        }

        static RV generate_core_popup(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle owner = GUI::text_button(context, node_core_id(node), node.label.c_str(),
                read_core_layout_input(node, read_size(node.properties[Name("size")])), node.enabled);
            apply_core_enabled(context, node, owner);

            GUICore::id_t popup_id = derived_core_id(node, "popup");
            GUICore::InteractionState owner_state = context->get_interaction_state(owner.id);
            Float2U& popup_position = runtime_value(node, Name("popup_position"), Float2U(0.0f));
            if(node.enabled && owner_state.clicked)
            {
                popup_position = Float2U(owner_state.clicked_screen_position.x - owner_state.clicked_element_position.x,
                    owner_state.clicked_screen_position.y - owner_state.clicked_element_position.y +
                    owner_state.clicked_element_rect.height);
                if(GUI::is_popup_open(context, popup_id))
                {
                    GUI::close_popup(context, popup_id);
                }
                else
                {
                    GUI::open_popup(context, popup_id);
                }
            }

            GUI::PopupDesc desc;
            desc.position = popup_position;
            f32 popup_width = property_f32(node, "popup_width", 220.0f);
            f32 popup_height = property_f32(node, "popup_height", 120.0f);
            desc.layout = fixed_core_layout(popup_width, popup_height);
            GUICore::ElementHandle popup;
            if(GUI::begin_popup(context, popup_id, desc, &popup))
            {
                RV r = generate_children(context, node, generate_context);
                if(failed(r))
                {
                    return r;
                }
                return GUI::end_popup(context, popup, RectF(0.0f, 0.0f, popup_width, popup_height));
            }
            return ok;
        }

        static RV generate_core_tooltip(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            GUICore::ElementHandle owner = GUI::text_button(context, node_core_id(node), node.label.c_str(),
                read_core_layout_input(node, read_size(node.properties[Name("size")])), node.enabled);
            apply_core_enabled(context, node, owner);

            GUI::TooltipDesc desc;
            desc.delay = property_f32(node, "delay", 0.0f);
            desc.offset = Float2U(property_f32(node, "offset_x", 10.0f), property_f32(node, "offset_y", 12.0f));
            f32 tooltip_width = property_f32(node, "tooltip_width", 180.0f);
            f32 tooltip_height = property_f32(node, "tooltip_height", 48.0f);
            desc.layout = fixed_core_layout(tooltip_width, tooltip_height);
            desc.max_width = property_f32(node, "max_width", tooltip_width);
            GUICore::ElementHandle tooltip;
            if(GUI::begin_tooltip(context, derived_core_id(node, "tooltip"), owner, desc, &tooltip))
            {
                RV r = generate_children(context, node, generate_context);
                if(failed(r))
                {
                    return r;
                }
                return GUI::end_tooltip(context, tooltip, RectF(0.0f, 0.0f, tooltip_width, tooltip_height));
            }
            return ok;
        }

        static RV generate_core_button_group(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            Vector<String> item_strings;
            Vector<const c8*> items;
            read_string_items(node.properties[Name("items")], item_strings, items);
            if(items.empty())
            {
                return ok;
            }
            GUICore::ElementHandle element;
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
                element = GUI::button_group(context, node_core_id(node), Span<bool>(selected_values.data(), selected_values.size()),
                    Span<const c8*>(items.data(), items.size()), read_core_layout_input(node));
            }
            else
            {
                i32& current_item = runtime_value(node, Name("current_item"), property_i32(node, "current_item", 0));
                element = GUI::button_group(context, node_core_id(node), &current_item, Span<const c8*>(items.data(), items.size()),
                    read_core_layout_input(node));
            }
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_combo(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            Vector<String> item_strings;
            Vector<const c8*> items;
            read_string_items(node.properties[Name("items")], item_strings, items);
            if(items.empty())
            {
                return ok;
            }
            i32& current_item = runtime_value(node, Name("current_item"), property_i32(node, "current_item", 0));
            GUICore::ElementHandle element = GUI::combo(context, node_core_id(node), node.label.c_str(), &current_item,
                Span<const c8*>(items.data(), items.size()), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_slider_float(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            f32& value = runtime_value(node, Name("value"), property_f32(node, "value", 0.0f));
            GUICore::ElementHandle element = GUI::slider_float(context, node_core_id(node), &value,
                property_f32(node, "min", 0.0f), property_f32(node, "max", 1.0f), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_slider_int(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            i32& value = runtime_value(node, Name("value"), property_i32(node, "value", 0));
            GUICore::ElementHandle element = GUI::slider_int(context, node_core_id(node), &value,
                property_i32(node, "min", 0), property_i32(node, "max", 100), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_drag_float(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            f32& value = runtime_value(node, Name("value"), property_f32(node, "value", 0.0f));
            GUICore::ElementHandle element = GUI::drag_float(context, node_core_id(node), &value,
                property_f32(node, "speed", 1.0f), property_f32(node, "min", 0.0f), property_f32(node, "max", 1.0f),
                read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_drag_int(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            i32& value = runtime_value(node, Name("value"), property_i32(node, "value", 0));
            GUICore::ElementHandle element = GUI::drag_int(context, node_core_id(node), &value,
                property_f32(node, "speed", 1.0f), property_i32(node, "min", 0), property_i32(node, "max", 100),
                read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_color_edit3(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            const f32 default_values[] = {1.0f, 1.0f, 1.0f};
            Vector<f32>& value = runtime_value(node, Name("value"),
                read_float_items(node.properties[Name("value")], Span<const f32>(default_values, 3)));
            value.resize(3, 1.0f);
            GUICore::ElementHandle element = GUI::color_edit3(context, node_core_id(node), node.label.c_str(),
                value.data(), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_color_edit4(GUICore::IContext* context, Node& node, const GenerateContext&)
        {
            const f32 default_values[] = {1.0f, 1.0f, 1.0f, 1.0f};
            Vector<f32>& value = runtime_value(node, Name("value"),
                read_float_items(node.properties[Name("value")], Span<const f32>(default_values, 4)));
            value.resize(4, 1.0f);
            GUICore::ElementHandle element = GUI::color_edit4(context, node_core_id(node), node.label.c_str(),
                value.data(), read_core_layout_input(node));
            apply_core_enabled(context, node, element);
            return ok;
        }

        static RV generate_core_asset_reference(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
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
            return write_size(AssetSize::fixed(width, height));
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

        static NodeTypeDesc make_desc(const c8* type, node_generate_core_func_t generate, Variant default_properties = Variant(VariantType::object))
        {
            NodeTypeDesc desc;
            desc.type = type;
            desc.default_properties = move(default_properties);
            desc.on_generate_core = generate;
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

        static NodeTypeDesc make_layout_desc(const c8* type, node_generate_core_func_t generate)
        {
            NodeTypeDesc desc = make_desc(type, generate);
            AssetLayoutDesc layout_desc;
            add_property(desc, "padding", "Padding", NodePropertyKind::edge_insets, write_edge_insets(layout_desc.padding), "Layout");
            add_property(desc, "gap", "Gap", NodePropertyKind::number, (f64)layout_desc.gap, "Layout", 0.0, 128.0, 1.0f);
            return desc;
        }

        static void register_builtin_node_types()
        {
            register_node_type(make_layout_desc("h_layout", generate_core_h_layout));
            register_node_type(make_layout_desc("v_layout", generate_core_v_layout));
            {
                NodeTypeDesc desc = make_desc("scroll_view", generate_core_scroll_view);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 240.0f), "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("grid_layout", generate_core_grid_layout);
                AssetGridLayoutDesc grid_desc;
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
                NodeTypeDesc desc = make_desc("canvas_layout", generate_core_canvas_layout);
                AssetCanvasLayoutDesc canvas_desc;
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 240.0f), "Layout");
                add_property(desc, "padding", "Padding", NodePropertyKind::edge_insets, write_edge_insets(canvas_desc.padding), "Layout");
                add_property(desc, "clip_children", "Clip Children", NodePropertyKind::boolean, true, "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("table_layout", generate_core_table_layout);
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
            register_node_type(make_desc("table_row", generate_core_table_row));
            register_node_type(make_desc("text", generate_core_text));
            register_node_type(make_desc("button", generate_core_button));
            {
                NodeTypeDesc desc = make_desc("tab_bar", generate_core_tab_bar);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 140.0f), "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("tab_item", generate_core_tab_item);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                add_property(desc, "open", "Open", NodePropertyKind::boolean, true, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("menu_bar", generate_core_menu_bar);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(320.0f, 28.0f), "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("menu", generate_core_menu);
                add_property(desc, "popup_width", "Popup Width", NodePropertyKind::number, 190.0, "Menu", 48.0, 4096.0, 1.0f);
                add_property(desc, "popup_height", "Popup Height", NodePropertyKind::number, 240.0, "Menu", 24.0, 4096.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("menu_item", generate_core_menu_item);
                add_property(desc, "checkable", "Checkable", NodePropertyKind::boolean, false, "State");
                add_property(desc, "checked", "Checked", NodePropertyKind::boolean, false, "State");
                add_property(desc, "shortcut", "Shortcut", NodePropertyKind::string, "", "Menu");
                register_node_type(desc);
            }
            register_node_type(make_desc("menu_separator", generate_core_menu_separator));
            {
                NodeTypeDesc desc = make_desc("popup", generate_core_popup);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(120.0f, 28.0f), "Layout");
                add_property(desc, "popup_width", "Popup Width", NodePropertyKind::number, 220.0, "Popup", 48.0, 4096.0, 1.0f);
                add_property(desc, "popup_height", "Popup Height", NodePropertyKind::number, 120.0, "Popup", 24.0, 4096.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("tooltip", generate_core_tooltip);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(140.0f, 28.0f), "Layout");
                add_property(desc, "tooltip_width", "Tooltip Width", NodePropertyKind::number, 180.0, "Tooltip", 48.0, 4096.0, 1.0f);
                add_property(desc, "tooltip_height", "Tooltip Height", NodePropertyKind::number, 48.0, "Tooltip", 24.0, 4096.0, 1.0f);
                add_property(desc, "delay", "Delay", NodePropertyKind::number, 0.0, "Tooltip", 0.0, 10.0, 0.05f);
                add_property(desc, "offset_x", "Offset X", NodePropertyKind::number, 10.0, "Tooltip", -4096.0, 4096.0, 1.0f);
                add_property(desc, "offset_y", "Offset Y", NodePropertyKind::number, 12.0, "Tooltip", -4096.0, 4096.0, 1.0f);
                add_property(desc, "max_width", "Max Width", NodePropertyKind::number, 180.0, "Tooltip", 48.0, 4096.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("progress_bar", generate_core_progress_bar);
                add_property(desc, "fraction", "Fraction", NodePropertyKind::number, 0.5, "Progress", 0.0, 1.0, 0.01f);
                add_property(desc, "overlay", "Overlay", NodePropertyKind::string, "", "Progress");
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(0.0f, 0.0f), "Layout");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("selectable", generate_core_selectable);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("checkbox", generate_core_checkbox);
                add_property(desc, "value", "Value", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("radio_button", generate_core_radio_button);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("toggle_switch", generate_core_toggle_switch);
                add_property(desc, "value", "Value", NodePropertyKind::boolean, false, "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("input_text", generate_core_input_text);
                add_property(desc, "value", "Value", NodePropertyKind::string, "", "State");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("image", generate_core_image);
                add_property(desc, "size", "Size", NodePropertyKind::size, default_size(64.0f, 64.0f), "Layout");
                register_node_type(desc);
            }
            register_node_type(make_desc("collapsing_header", generate_core_collapsing_header));
            {
                NodeTypeDesc desc = make_desc("tree_node", generate_core_tree_node);
                add_property(desc, "selected", "Selected", NodePropertyKind::boolean, false, "State");
                add_property(desc, "leaf", "Leaf", NodePropertyKind::boolean, false, "Behavior");
                add_property(desc, "default_open", "Default Open", NodePropertyKind::boolean, false, "Behavior");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("button_group", generate_core_button_group);
                const c8* items[] = {"One", "Two", "Three"};
                add_property(desc, "items", "Items", NodePropertyKind::string_array, string_array(Span<const c8* const>(items, 3)), "Items");
                add_property(desc, "current_item", "Current Item", NodePropertyKind::integer, (i64)0, "State", 0.0, 64.0, 1.0f);
                add_property(desc, "multi_select", "Multi Select", NodePropertyKind::boolean, false, "Behavior");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("combo", generate_core_combo);
                const c8* items[] = {"Alpha", "Beta", "Gamma"};
                add_property(desc, "items", "Items", NodePropertyKind::string_array, string_array(Span<const c8* const>(items, 3)), "Items");
                add_property(desc, "current_item", "Current Item", NodePropertyKind::integer, (i64)0, "State", 0.0, 64.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("slider_float", generate_core_slider_float);
                add_property(desc, "value", "Value", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "max", "Max", NodePropertyKind::number, 1.0, "Numeric", -10000.0, 10000.0, 0.01f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("slider_int", generate_core_slider_int);
                add_property(desc, "value", "Value", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "min", "Min", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "max", "Max", NodePropertyKind::integer, (i64)100, "Numeric", -10000.0, 10000.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("drag_float", generate_core_drag_float);
                add_property(desc, "value", "Value", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "speed", "Speed", NodePropertyKind::number, 1.0, "Numeric", 0.001, 1000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::number, 0.0, "Numeric", -10000.0, 10000.0, 0.01f);
                add_property(desc, "max", "Max", NodePropertyKind::number, 1.0, "Numeric", -10000.0, 10000.0, 0.01f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("drag_int", generate_core_drag_int);
                add_property(desc, "value", "Value", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "speed", "Speed", NodePropertyKind::number, 1.0, "Numeric", 0.001, 1000.0, 0.01f);
                add_property(desc, "min", "Min", NodePropertyKind::integer, (i64)0, "Numeric", -10000.0, 10000.0, 1.0f);
                add_property(desc, "max", "Max", NodePropertyKind::integer, (i64)100, "Numeric", -10000.0, 10000.0, 1.0f);
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("color_edit3", generate_core_color_edit3);
                const f64 values[] = {1.0, 1.0, 1.0};
                add_property(desc, "value", "Value", NodePropertyKind::number_array, number_array(Span<const f64>(values, 3)), "Color");
                register_node_type(desc);
            }
            {
                NodeTypeDesc desc = make_desc("color_edit4", generate_core_color_edit4);
                const f64 values[] = {1.0, 1.0, 1.0, 1.0};
                add_property(desc, "value", "Value", NodePropertyKind::number_array, number_array(Span<const f64>(values, 4)), "Color");
                register_node_type(desc);
            }
            {
                NodeTypeDesc asset_ref = make_desc("asset_reference", generate_core_asset_reference);
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
                root.get()->layout_input.width.kind = GUICore::SizeKind::percent;
                root.get()->layout_input.width.value = 1.0f;
                root.get()->layout_input.height.kind = GUICore::SizeKind::percent;
                root.get()->layout_input.height.value = 1.0f;
                root.get()->layout_input.flex_grow = 1.0f;
                root.get()->has_layout_input = true;
                add_node(asset.get(), root.get());
            }
            return asset;
        }

        LUNA_GUI_ASSET_API RV generate(GUICore::IContext* context, Asset* asset, const GenerateContext& generate_context)
        {
            Guid root_id = get_root(asset);
            if(!context || !asset || root_id == Guid(0, 0))
            {
                return set_error(BasicError::bad_arguments(), "GUIAsset::generate requires a valid GUI Core context and asset root.");
            }
            Ref<Node> root = find_node(asset, root_id);
            if(!root)
            {
                return set_error(BasicError::not_found(), "GUIAsset root node is not found in the node map.");
            }
            GenerateContext effective_context = generate_context;
            effective_context.asset = asset;
            RV r = generate_node(context, *root.get(), effective_context);
            if(failed(r))
            {
                return r;
            }
            return GUI::layout_editor_tree(context, context->find_element_handle(node_core_id(*root.get())),
                core_generation_rect(context, effective_context));
        }

        LUNA_GUI_ASSET_API RV generate_node(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            lutry
            {
                lulet(desc, get_node_type(node.type));
                if(!desc.on_generate_core)
                {
                    return set_error(BasicError::not_supported(), "GUIAsset node type '%s' has no GUI Core generate callback.", node.type.c_str());
                }
                if(node.id == Guid(0, 0))
                {
                    node.id = random_guid();
                }
                apply_core_common_modifiers(context, node);
                RV r = desc.on_generate_core(context, node, generate_context);
                finish_core_common_modifiers(context, node);
                if(failed(r))
                {
                    return r;
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_ASSET_API RV generate_children(GUICore::IContext* context, Node& node, const GenerateContext& generate_context)
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
                if(node.has_layout_input)
                {
                    r[name_layout] = write_layout_input(node.layout_input);
                }
                if(node.has_canvas_layout)
                {
                    r[name_canvas_layout] = write_canvas_layout(node.canvas_layout);
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
                    node->layout_input = read_layout_input(data[name_layout]);
                    node->has_layout_input = true;
                }
                if(data[name_canvas_layout].valid())
                {
                    node->canvas_layout = read_canvas_layout(data[name_canvas_layout]);
                    node->has_canvas_layout = true;
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
                return add_dependency_modules(this, {
                    GUI::module_gui(),
                    GUICore::module_gui_core(),
                    module_asset(),
                    module_variant_utils(),
                    module_vfs()});
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
