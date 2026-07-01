/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUICORE_API LUNA_EXPORT
#include "../Layout.hpp"

namespace Luna
{
    namespace GUICore
    {
        namespace
        {
            f32 axis_value(const Float2U& value, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? value.x : value.y;
            }

            const SizeValue& requested_size(const LayoutInput& layout, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? layout.width : layout.height;
            }

            f32 rect_size(const RectF& rect, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? rect.width : rect.height;
            }

            f32 margin_begin(const Float4U& margin, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? margin.x : margin.y;
            }

            f32 margin_end(const Float4U& margin, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? margin.z : margin.w;
            }

            f32 padding_begin(const Float4U& padding, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? padding.x : padding.y;
            }

            f32 padding_end(const Float4U& padding, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? padding.z : padding.w;
            }

            RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 min_x = max(a.offset_x, b.offset_x);
                f32 min_y = max(a.offset_y, b.offset_y);
                f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            f32 clamp_size(f32 value, const SizeValue& size)
            {
                f32 r = max(value, size.min);
                if(size.max >= 0.0f)
                {
                    r = min(r, size.max);
                }
                return r;
            }

            f32 content_axis_size(const Element& element, LayoutAxis axis)
            {
                return axis_value(element.layout_result.content_size, axis);
            }

            f32 resolve_non_flexible_size(const Element& element, LayoutAxis axis, f32 available, f32 fit_largest_size = 0.0f)
            {
                const SizeValue& size = requested_size(element.layout, axis);
                f32 value = 0.0f;
                switch(size.kind)
                {
                case SizeKind::pixels:
                    value = size.value;
                    break;
                case SizeKind::percent:
                    value = available * size.value;
                    break;
                case SizeKind::fit:
                    value = content_axis_size(element, axis);
                    break;
                case SizeKind::fit_largest:
                    value = fit_largest_size;
                    break;
                default:
                    value = 0.0f;
                    break;
                }
                return clamp_size(value, size);
            }

            f32 flexible_weight(const SizeValue& size)
            {
                if(size.kind == SizeKind::expand)
                {
                    return size.value > 0.0f ? size.value : 1.0f;
                }
                if(size.kind == SizeKind::ratio)
                {
                    return size.value > 0.0f ? size.value : 1.0f;
                }
                return 0.0f;
            }

            bool is_flexible(const SizeValue& size)
            {
                return size.kind == SizeKind::expand || size.kind == SizeKind::ratio;
            }

            RectF inset_rect(const RectF& rect, const Float4U& inset)
            {
                return RectF(
                    rect.offset_x + inset.x,
                    rect.offset_y + inset.y,
                    max(rect.width - inset.x - inset.z, 0.0f),
                    max(rect.height - inset.y - inset.w, 0.0f));
            }

            bool rect_valid(const RectF& rect)
            {
                return rect.width > 0.0f || rect.height > 0.0f;
            }

            RectF inherited_layout_clip(const Element& parent, const RectF& rect)
            {
                bool has_previous_layout = rect_valid(parent.layout_result.rect) || rect_valid(parent.layout_result.clip_rect);
                return has_previous_layout ? intersect_rect(rect, parent.layout_result.clip_rect) : rect;
            }

            Vector<u32> collect_children(IContext* context, const Element& parent)
            {
                Vector<u32> children;
                for(u32 child = parent.first_child; child != INVALID_ELEMENT;)
                {
                    const Element* child_element = context->get_element(child);
                    if(!child_element)
                    {
                        break;
                    }
                    children.push_back(child);
                    child = child_element->next_sibling;
                }
                return children;
            }

            f32 largest_content_size(IContext* context, const Vector<u32>& children, LayoutAxis axis)
            {
                f32 r = 0.0f;
                for(u32 child_index : children)
                {
                    const Element* child = context->get_element(child_index);
                    if(child)
                    {
                        r = max(r, content_axis_size(*child, axis));
                    }
                }
                return r;
            }

            f32 resolve_stack_axis_size(const Element& child, LayoutAxis axis, f32 available, f32 fit_largest_size = 0.0f)
            {
                const SizeValue& size = requested_size(child.layout, axis);
                if(is_flexible(size))
                {
                    return clamp_size(available, size);
                }
                return resolve_non_flexible_size(child, axis, available, fit_largest_size);
            }

            const CanvasLayoutItem& find_canvas_item(const CanvasLayoutDesc& desc, id_t element_id)
            {
                for(const CanvasLayoutItem& item : desc.items)
                {
                    if(item.element_id == element_id)
                    {
                        return item;
                    }
                }
                return desc.default_item;
            }

            f32 resolve_canvas_axis_position(f32 content_offset, f32 content_size, f32 anchor, f32 offset)
            {
                return content_offset + content_size * anchor + offset;
            }

            f32 clamp_table_track_size(f32 value, const TableTrackDesc& track)
            {
                f32 r = max(value, track.min);
                if(track.max >= 0.0f)
                {
                    r = min(r, track.max);
                }
                return r;
            }

            f32 table_track_weight(const TableTrackDesc& track)
            {
                return track.value > 0.0f ? track.value : 1.0f;
            }

            bool find_direct_child(IContext* context, const Vector<u32>& children, id_t id, u32& out_index, const Element*& out_element)
            {
                for(u32 child_index : children)
                {
                    const Element* child = context->get_element(child_index);
                    if(child && child->id == id)
                    {
                        out_index = child_index;
                        out_element = child;
                        return true;
                    }
                }
                out_index = INVALID_ELEMENT;
                out_element = nullptr;
                return false;
            }

            f32 sum_tracks(Span<const f32> tracks, u32 begin, u32 count, f32 gap)
            {
                if(count == 0)
                {
                    return 0.0f;
                }
                f32 r = 0.0f;
                for(u32 i = 0; i < count; ++i)
                {
                    r += tracks[begin + i];
                }
                r += gap * (f32)(count - 1);
                return r;
            }

            void resolve_table_tracks(IContext* context, const Vector<u32>& children, Span<const TableLayoutCell> cells,
                Span<const TableTrackDesc> tracks, bool columns, f32 available, f32 gap, Vector<f32>& out_sizes)
            {
                out_sizes.resize(tracks.size());
                for(usize i = 0; i < out_sizes.size(); ++i)
                {
                    out_sizes[i] = 0.0f;
                }
                for(usize i = 0; i < tracks.size(); ++i)
                {
                    const TableTrackDesc& track = tracks[i];
                    switch(track.kind)
                    {
                    case TableTrackSizeKind::pixels:
                        out_sizes[i] = clamp_table_track_size(track.value, track);
                        break;
                    case TableTrackSizeKind::percent:
                        out_sizes[i] = clamp_table_track_size(available * track.value, track);
                        break;
                    case TableTrackSizeKind::fit:
                        out_sizes[i] = clamp_table_track_size(0.0f, track);
                        break;
                    default:
                        break;
                    }
                }

                for(const TableLayoutCell& cell : cells)
                {
                    u32 track_index = columns ? cell.column : cell.row;
                    u32 span = max(columns ? cell.column_span : cell.row_span, 1U);
                    if(track_index >= tracks.size() || track_index + span > tracks.size() || span != 1)
                    {
                        continue;
                    }
                    const TableTrackDesc& track = tracks[track_index];
                    if(track.kind != TableTrackSizeKind::fit)
                    {
                        continue;
                    }
                    u32 child_index = INVALID_ELEMENT;
                    const Element* child = nullptr;
                    if(!find_direct_child(context, children, cell.element_id, child_index, child))
                    {
                        continue;
                    }
                    f32 desired = 0.0f;
                    if(columns)
                    {
                        desired = resolve_non_flexible_size(*child, LayoutAxis::x, available) +
                            child->layout.margin.x + child->layout.margin.z + cell.padding.x + cell.padding.z;
                    }
                    else
                    {
                        desired = resolve_non_flexible_size(*child, LayoutAxis::y, available) +
                            child->layout.margin.y + child->layout.margin.w + cell.padding.y + cell.padding.w;
                    }
                    out_sizes[track_index] = max(out_sizes[track_index], clamp_table_track_size(desired, track));
                }

                f32 total_gap = out_sizes.empty() ? 0.0f : gap * (f32)(out_sizes.size() - 1);
                f32 fixed_size = 0.0f;
                f32 total_weight = 0.0f;
                for(usize i = 0; i < tracks.size(); ++i)
                {
                    if(tracks[i].kind == TableTrackSizeKind::ratio)
                    {
                        total_weight += table_track_weight(tracks[i]);
                    }
                    else
                    {
                        fixed_size += out_sizes[i];
                    }
                }
                f32 remaining = max(available - fixed_size - total_gap, 0.0f);
                for(usize i = 0; i < tracks.size(); ++i)
                {
                    if(tracks[i].kind == TableTrackSizeKind::ratio)
                    {
                        f32 value = total_weight > 0.0f ? remaining * table_track_weight(tracks[i]) / total_weight : 0.0f;
                        out_sizes[i] = clamp_table_track_size(value, tracks[i]);
                    }
                }
            }
        }

        LUNA_GUICORE_API RV layout_linear(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const LinearLayoutDesc& desc = *reinterpret_cast<const LinearLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_linear"), Name("explicit_layout_call"), parent->id);

            LayoutAxis main_axis = desc.axis;
            LayoutAxis cross_axis = main_axis == LayoutAxis::x ? LayoutAxis::y : LayoutAxis::x;
            f32 pad_main_begin = padding_begin(parent->layout.padding, main_axis);
            f32 pad_main_end = padding_end(parent->layout.padding, main_axis);
            f32 pad_cross_begin = padding_begin(parent->layout.padding, cross_axis);
            f32 pad_cross_end = padding_end(parent->layout.padding, cross_axis);
            RectF content_rect = rect;
            if(main_axis == LayoutAxis::x)
            {
                content_rect.offset_x += pad_main_begin;
                content_rect.width = max(content_rect.width - pad_main_begin - pad_main_end, 0.0f);
                content_rect.offset_y += pad_cross_begin;
                content_rect.height = max(content_rect.height - pad_cross_begin - pad_cross_end, 0.0f);
            }
            else
            {
                content_rect.offset_y += pad_main_begin;
                content_rect.height = max(content_rect.height - pad_main_begin - pad_main_end, 0.0f);
                content_rect.offset_x += pad_cross_begin;
                content_rect.width = max(content_rect.width - pad_cross_begin - pad_cross_end, 0.0f);
            }

            Vector<u32> children = collect_children(context, *parent);

            f32 available_main = rect_size(content_rect, main_axis);
            f32 available_cross = rect_size(content_rect, cross_axis);
            f32 largest_main = largest_content_size(context, children, main_axis);
            f32 largest_cross = largest_content_size(context, children, cross_axis);
            f32 fixed_outer_main = 0.0f;
            f32 total_weight = 0.0f;
            for(u32 child_index : children)
            {
                const Element* child = context->get_element(child_index);
                const SizeValue& main_size = requested_size(child->layout, main_axis);
                fixed_outer_main += margin_begin(child->layout.margin, main_axis) + margin_end(child->layout.margin, main_axis);
                if(is_flexible(main_size))
                {
                    total_weight += flexible_weight(main_size);
                }
                else
                {
                    fixed_outer_main += resolve_non_flexible_size(*child, main_axis, available_main, largest_main);
                }
            }
            f32 total_gap = children.empty() ? 0.0f : desc.gap * (f32)(children.size() - 1);
            f32 remaining_main = max(available_main - fixed_outer_main - total_gap, 0.0f);
            f32 cursor = main_axis == LayoutAxis::x ? content_rect.offset_x : content_rect.offset_y;
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;

            f32 measured_main = 0.0f;
            f32 measured_cross = 0.0f;
            for(usize i = 0; i < children.size(); ++i)
            {
                const Element* child = context->get_element(children[i]);
                const SizeValue& main_size = requested_size(child->layout, main_axis);
                const SizeValue& cross_size = requested_size(child->layout, cross_axis);
                f32 main = is_flexible(main_size) && total_weight > 0.0f ?
                    clamp_size(remaining_main * flexible_weight(main_size) / total_weight, main_size) :
                    resolve_non_flexible_size(*child, main_axis, available_main, largest_main);
                f32 cross = is_flexible(cross_size) ?
                    clamp_size(max(available_cross - margin_begin(child->layout.margin, cross_axis) -
                        margin_end(child->layout.margin, cross_axis), 0.0f), cross_size) :
                    resolve_non_flexible_size(*child, cross_axis, available_cross, largest_cross);

                cursor += margin_begin(child->layout.margin, main_axis);
                f32 cross_position = (main_axis == LayoutAxis::x ? content_rect.offset_y : content_rect.offset_x) +
                    margin_begin(child->layout.margin, cross_axis);
                RectF child_rect;
                if(main_axis == LayoutAxis::x)
                {
                    child_rect = RectF(cursor, cross_position, main, cross);
                }
                else
                {
                    child_rect = RectF(cross_position, cursor, cross, main);
                }
                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, children[i], element.generation }, result);

                cursor += main + margin_end(child->layout.margin, main_axis);
                if(i + 1 < children.size())
                {
                    cursor += desc.gap;
                }
                measured_main = cursor - (main_axis == LayoutAxis::x ? content_rect.offset_x : content_rect.offset_y);
                measured_cross = max(measured_cross, cross + margin_begin(child->layout.margin, cross_axis) +
                    margin_end(child->layout.margin, cross_axis));
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            if(main_axis == LayoutAxis::x)
            {
                parent_result.content_size = Float2U(measured_main, measured_cross);
            }
            else
            {
                parent_result.content_size = Float2U(measured_cross, measured_main);
            }
            context->set_layout_result(element, parent_result);
            return ok;
        }

        LUNA_GUICORE_API RV layout_grid(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const GridLayoutDesc& desc = *reinterpret_cast<const GridLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_grid"), Name("explicit_layout_call"), parent->id);

            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);

            f32 cell_width = max(desc.cell_size.x, 0.0f);
            f32 cell_height = max(desc.cell_size.y, 0.0f);
            u32 column_count = max(desc.column_count, 1U);
            if(desc.mode == GridLayoutMode::fixed_cell_size)
            {
                if(cell_width <= 0.0f)
                {
                    column_count = 1;
                }
                else
                {
                    f32 stride = cell_width + desc.gap.x;
                    column_count = stride <= 0.0f ? 1U : max((u32)((content_rect.width + desc.gap.x) / stride), 1U);
                }
            }
            else
            {
                f32 total_gap = desc.gap.x * (f32)(column_count - 1);
                cell_width = max((content_rect.width - total_gap) / (f32)column_count, 0.0f);
            }

            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            f32 measured_width = 0.0f;
            f32 measured_height = 0.0f;
            for(usize i = 0; i < children.size(); ++i)
            {
                const Element* child = context->get_element(children[i]);
                u32 column = (u32)(i % column_count);
                u32 row = (u32)(i / column_count);
                RectF cell_rect(
                    content_rect.offset_x + (f32)column * (cell_width + desc.gap.x),
                    content_rect.offset_y + (f32)row * (cell_height + desc.gap.y),
                    cell_width,
                    cell_height);
                RectF child_rect = inset_rect(cell_rect, child->layout.margin);

                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, children[i], element.generation }, result);

                measured_width = max(measured_width, (f32)(column + 1) * cell_width + (f32)column * desc.gap.x);
                measured_height = max(measured_height, (f32)(row + 1) * cell_height + (f32)row * desc.gap.y);
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            parent_result.content_size = Float2U(measured_width, measured_height);
            context->set_layout_result(element, parent_result);
            return ok;
        }

        LUNA_GUICORE_API RV layout_stack(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const StackLayoutDesc& desc = *reinterpret_cast<const StackLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_stack"), Name("explicit_layout_call"), parent->id);

            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            f32 largest_width = largest_content_size(context, children, LayoutAxis::x);
            f32 largest_height = largest_content_size(context, children, LayoutAxis::y);
            f32 measured_width = 0.0f;
            f32 measured_height = 0.0f;
            for(u32 child_index : children)
            {
                const Element* child = context->get_element(child_index);
                f32 available_width = max(content_rect.width - child->layout.margin.x - child->layout.margin.z, 0.0f);
                f32 available_height = max(content_rect.height - child->layout.margin.y - child->layout.margin.w, 0.0f);
                f32 width = resolve_stack_axis_size(*child, LayoutAxis::x, available_width, largest_width);
                f32 height = resolve_stack_axis_size(*child, LayoutAxis::y, available_height, largest_height);
                f32 offset_x = content_rect.offset_x + child->layout.margin.x + max(available_width - width, 0.0f) * desc.alignment.x;
                f32 offset_y = content_rect.offset_y + child->layout.margin.y + max(available_height - height, 0.0f) * desc.alignment.y;

                RectF child_rect(offset_x, offset_y, width, height);
                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, child_index, element.generation }, result);

                measured_width = max(measured_width, width + child->layout.margin.x + child->layout.margin.z);
                measured_height = max(measured_height, height + child->layout.margin.y + child->layout.margin.w);
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            parent_result.content_size = Float2U(measured_width, measured_height);
            context->set_layout_result(element, parent_result);
            return ok;
        }

        LUNA_GUICORE_API RV layout_canvas(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const CanvasLayoutDesc& desc = *reinterpret_cast<const CanvasLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_canvas"), Name("explicit_layout_call"), parent->id);

            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            f32 largest_width = largest_content_size(context, children, LayoutAxis::x);
            f32 largest_height = largest_content_size(context, children, LayoutAxis::y);
            f32 measured_width = 0.0f;
            f32 measured_height = 0.0f;
            for(u32 child_index : children)
            {
                const Element* child = context->get_element(child_index);
                const CanvasLayoutItem& item = find_canvas_item(desc, child->id);

                bool stretch_x = item.anchor_min.x != item.anchor_max.x;
                bool stretch_y = item.anchor_min.y != item.anchor_max.y;
                f32 available_width = max(content_rect.width - child->layout.margin.x - child->layout.margin.z, 0.0f);
                f32 available_height = max(content_rect.height - child->layout.margin.y - child->layout.margin.w, 0.0f);

                f32 left = 0.0f;
                f32 top = 0.0f;
                f32 width = 0.0f;
                f32 height = 0.0f;
                if(stretch_x)
                {
                    left = resolve_canvas_axis_position(content_rect.offset_x, content_rect.width, item.anchor_min.x, item.offset.x);
                    f32 right = resolve_canvas_axis_position(content_rect.offset_x, content_rect.width, item.anchor_max.x, item.offset.z);
                    width = max(right - left, 0.0f);
                }
                else
                {
                    width = resolve_stack_axis_size(*child, LayoutAxis::x, available_width, largest_width);
                    f32 anchor_x = resolve_canvas_axis_position(content_rect.offset_x, content_rect.width, item.anchor_min.x, item.offset.x);
                    left = anchor_x - width * item.pivot.x;
                }
                if(stretch_y)
                {
                    top = resolve_canvas_axis_position(content_rect.offset_y, content_rect.height, item.anchor_min.y, item.offset.y);
                    f32 bottom = resolve_canvas_axis_position(content_rect.offset_y, content_rect.height, item.anchor_max.y, item.offset.w);
                    height = max(bottom - top, 0.0f);
                }
                else
                {
                    height = resolve_stack_axis_size(*child, LayoutAxis::y, available_height, largest_height);
                    f32 anchor_y = resolve_canvas_axis_position(content_rect.offset_y, content_rect.height, item.anchor_min.y, item.offset.y);
                    top = anchor_y - height * item.pivot.y;
                }

                RectF child_rect = inset_rect(RectF(left, top, width, height), child->layout.margin);
                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, child_index, element.generation }, result);

                measured_width = max(measured_width, child_rect.offset_x + child_rect.width - content_rect.offset_x);
                measured_height = max(measured_height, child_rect.offset_y + child_rect.height - content_rect.offset_y);
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            parent_result.content_size = Float2U(measured_width, measured_height);
            context->set_layout_result(element, parent_result);
            return ok;
        }

        LUNA_GUICORE_API RV layout_scroll_viewport(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const ScrollViewportLayoutDesc& desc = *reinterpret_cast<const ScrollViewportLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_scroll_viewport"), Name("explicit_layout_call"),
                parent->id);

            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            f32 largest_width = largest_content_size(context, children, LayoutAxis::x);
            f32 largest_height = largest_content_size(context, children, LayoutAxis::y);
            f32 measured_width = 0.0f;
            f32 measured_height = 0.0f;
            for(u32 child_index : children)
            {
                const Element* child = context->get_element(child_index);
                f32 available_width = max(content_rect.width - child->layout.margin.x - child->layout.margin.z, 0.0f);
                f32 available_height = max(content_rect.height - child->layout.margin.y - child->layout.margin.w, 0.0f);
                f32 width = resolve_stack_axis_size(*child, LayoutAxis::x, available_width, largest_width);
                f32 height = resolve_stack_axis_size(*child, LayoutAxis::y, available_height, largest_height);
                RectF child_rect(
                    content_rect.offset_x - desc.scroll_offset.x + child->layout.margin.x,
                    content_rect.offset_y - desc.scroll_offset.y + child->layout.margin.y,
                    width,
                    height);

                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, child_index, element.generation }, result);

                measured_width = max(measured_width, width + child->layout.margin.x + child->layout.margin.z);
                measured_height = max(measured_height, height + child->layout.margin.y + child->layout.margin.w);
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            parent_result.content_size = Float2U(measured_width, measured_height);
            context->set_layout_result(element, parent_result);
            return ok;
        }

        LUNA_GUICORE_API RV layout_table(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            if(!userdata)
            {
                return BasicError::bad_arguments();
            }
            const TableLayoutDesc& desc = *reinterpret_cast<const TableLayoutDesc*>(userdata);
            context->log_debug_pass(DebugPassKind::layout, Name("layout_table"), Name("explicit_layout_call"), parent->id);

            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            Vector<f32> column_sizes;
            Vector<f32> row_sizes;
            resolve_table_tracks(context, children, desc.cells, desc.columns, true, content_rect.width, desc.gap.x, column_sizes);
            resolve_table_tracks(context, children, desc.cells, desc.rows, false, content_rect.height, desc.gap.y, row_sizes);

            Vector<f32> column_offsets;
            column_offsets.resize(column_sizes.size());
            f32 cursor = content_rect.offset_x;
            for(usize i = 0; i < column_sizes.size(); ++i)
            {
                column_offsets[i] = cursor;
                cursor += column_sizes[i] + desc.gap.x;
            }
            Vector<f32> row_offsets;
            row_offsets.resize(row_sizes.size());
            cursor = content_rect.offset_y;
            for(usize i = 0; i < row_sizes.size(); ++i)
            {
                row_offsets[i] = cursor;
                cursor += row_sizes[i] + desc.gap.y;
            }

            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            for(const TableLayoutCell& cell : desc.cells)
            {
                u32 row_span = max(cell.row_span, 1U);
                u32 column_span = max(cell.column_span, 1U);
                if(cell.row >= row_sizes.size() || cell.column >= column_sizes.size() ||
                    cell.row + row_span > row_sizes.size() || cell.column + column_span > column_sizes.size())
                {
                    continue;
                }
                u32 child_index = INVALID_ELEMENT;
                const Element* child = nullptr;
                if(!find_direct_child(context, children, cell.element_id, child_index, child))
                {
                    continue;
                }
                RectF cell_rect(
                    column_offsets[cell.column],
                    row_offsets[cell.row],
                    sum_tracks(Span<const f32>(column_sizes.data(), column_sizes.size()), cell.column, column_span, desc.gap.x),
                    sum_tracks(Span<const f32>(row_sizes.data(), row_sizes.size()), cell.row, row_span, desc.gap.y));
                RectF child_rect = inset_rect(inset_rect(cell_rect, cell.padding), child->layout.margin);

                LayoutResult result;
                result.rect = child_rect;
                result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                result.content_size = Float2U(child_rect.width, child_rect.height);
                context->set_layout_result(ElementHandle { child->id, child_index, element.generation }, result);
            }

            LayoutResult parent_result;
            parent_result.rect = rect;
            parent_result.clip_rect = parent_clip;
            parent_result.content_size = Float2U(
                sum_tracks(Span<const f32>(column_sizes.data(), column_sizes.size()), 0, (u32)column_sizes.size(), desc.gap.x),
                sum_tracks(Span<const f32>(row_sizes.data(), row_sizes.size()), 0, (u32)row_sizes.size(), desc.gap.y));
            context->set_layout_result(element, parent_result);
            return ok;
        }
    }
}
