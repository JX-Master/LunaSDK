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
#include "GUICore.hpp"

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

            void set_axis_value(Float2U& value, LayoutAxis axis, f32 axis_value)
            {
                if(axis == LayoutAxis::x)
                {
                    value.x = axis_value;
                }
                else
                {
                    value.y = axis_value;
                }
            }

            const SizeValue& requested_size(const LayoutConfig& layout, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? layout.width : layout.height;
            }

            f32 rect_size(const RectF& rect, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? rect.width : rect.height;
            }

            f32 rect_offset(const RectF& rect, LayoutAxis axis)
            {
                return axis == LayoutAxis::x ? rect.offset_x : rect.offset_y;
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

            f32 clamp_size(f32 value, f32 minimum, f32 maximum)
            {
                return min(max(value, minimum), maximum);
            }

            f32 resolve_requested_axis_size(const Element& element, LayoutAxis axis, f32 available, f32 content_desired)
            {
                const SizeValue& size = requested_size(element.layout, axis);
                f32 value = 0.0f;
                switch(size.kind)
                {
                case SizeKind::fixed:
                    value = size.value;
                    break;
                case SizeKind::percent:
                    value = available * size.value;
                    break;
                case SizeKind::fit:
                    value = content_desired;
                    break;
                default:
                    value = 0.0f;
                    break;
                }
                return value;
            }

            f32 resolve_axis_maximum(const SizeValue& size)
            {
                return size.max >= 0.0f ? size.max : F32_MAX;
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
                return has_previous_layout ? parent.layout_result.clip_rect : rect;
            }

            Ref<ScrollViewportHistoryState> find_scroll_viewport_history(IContext* context, id_t element_id)
            {
                Ref<ScrollViewportHistoryState> state;
                if(object_t state_object = context->get_state(make_state_id<ScrollViewportHistoryState>(element_id)))
                {
                    object_retain(state_object);
                    state.attach(state_object);
                }
                return state;
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

            f32 resolve_measured_axis_size(const Element& child, LayoutAxis axis, f32 available,
                const MeasureResult& measured)
            {
                const SizeValue& size = requested_size(child.layout, axis);
                f32 desired = resolve_requested_axis_size(child, axis, available, axis_value(measured.desired, axis));
                return clamp_size(desired, axis_value(measured.minimum, axis), axis_value(measured.maximum, axis));
            }

            struct FlexItem
            {
                u32 element_index = INVALID_ELEMENT;
                MeasureResult measured;
                f32 outer_min = 0.0f;
                f32 outer_desired = 0.0f;
                f32 outer_max = F32_MAX;
                f32 outer_cross_desired = 0.0f;
                f32 outer_cross_min = 0.0f;
                f32 outer_cross_max = F32_MAX;
                f32 resolved_outer_main = 0.0f;
                f32 resolved_cross = 0.0f;
                f32 grow = 0.0f;
                f32 shrink = 1.0f;
            };

            struct FlexLine
            {
                u32 first_item = 0;
                u32 item_count = 0;
                f32 minimum_main = 0.0f;
                f32 desired_main = 0.0f;
                f32 resolved_main = 0.0f;
                f32 cross_size = 0.0f;
            };

            MeasureResult measure_element_by_index(IContext* context, u32 element_index, const Float2U& available);

            Float2U content_available_size(const Element& element, const Float2U& available)
            {
                return Float2U(
                    max(available.x - element.layout.padding.x - element.layout.padding.z, 0.0f),
                    max(available.y - element.layout.padding.y - element.layout.padding.w, 0.0f));
            }

            MeasureResult sanitize_content_measure(const MeasureResult& content)
            {
                MeasureResult r = content;
                r.minimum.x = max(r.minimum.x, 0.0f);
                r.minimum.y = max(r.minimum.y, 0.0f);
                r.desired.x = max(r.desired.x, r.minimum.x);
                r.desired.y = max(r.desired.y, r.minimum.y);
                r.maximum.x = max(r.maximum.x, r.minimum.x);
                r.maximum.y = max(r.maximum.y, r.minimum.y);
                return r;
            }

            MeasureResult resolve_element_measure(const Element& element, const Float2U& available,
                const MeasureResult& content_measure)
            {
                MeasureResult content = sanitize_content_measure(content_measure);
                MeasureResult r;
                for(LayoutAxis axis : { LayoutAxis::x, LayoutAxis::y })
                {
                    const SizeValue& size = requested_size(element.layout, axis);
                    f32 available_axis = axis_value(available, axis);
                    f32 padding = padding_begin(element.layout.padding, axis) + padding_end(element.layout.padding, axis);
                    f32 content_min = axis_value(content.minimum, axis) + padding;
                    f32 content_desired = axis_value(content.desired, axis) + padding;
                    f32 content_max = axis_value(content.maximum, axis);
                    content_max = content_max >= F32_MAX * 0.5f ? F32_MAX : content_max + padding;
                    f32 minimum = size.kind == SizeKind::fit ? max(size.min, content_min) : size.min;
                    f32 maximum = min(resolve_axis_maximum(size), size.kind == SizeKind::fit ? content_max : F32_MAX);
                    maximum = max(maximum, minimum);
                    f32 desired = resolve_requested_axis_size(element, axis, available_axis, content_desired);
                    desired = clamp_size(desired, minimum, maximum);
                    set_axis_value(r.minimum, axis, minimum);
                    set_axis_value(r.desired, axis, desired);
                    set_axis_value(r.maximum, axis, maximum);
                }
                return r;
            }

            MeasureResult measure_content(IContext* context, u32 element_index, const Element& element,
                const Float2U& available)
            {
                if(element.layout_callback_config == U32_MAX)
                {
                    return MeasureResult();
                }
                ElementHandle handle { element.id, element_index, context->generation() };
                LayoutCallbackConfig callbacks = context->get_layout_callback_config(handle);
                if(!callbacks.measure_callback)
                {
                    return MeasureResult();
                }
                return callbacks.measure_callback(context, handle, content_available_size(element, available),
                    callbacks.userdata);
            }

            MeasureResult measure_leaf_element(IContext* context, u32 element_index, const Element& element,
                const Float2U& available)
            {
                return resolve_element_measure(element, available, measure_content(context, element_index, element, available));
            }

            void append_flex_line(Vector<FlexLine>& lines, u32 first_item, u32 item_count,
                f32 minimum_main, f32 desired_main, f32 cross_size)
            {
                if(item_count == 0)
                {
                    return;
                }
                FlexLine line;
                line.first_item = first_item;
                line.item_count = item_count;
                line.minimum_main = minimum_main;
                line.desired_main = desired_main;
                line.resolved_main = desired_main;
                line.cross_size = cross_size;
                lines.push_back(line);
            }

            void build_flex_items(IContext* context, const Vector<u32>& children, const Float2U& available,
                LayoutAxis main_axis, LayoutAxis cross_axis, Vector<FlexItem>& out_items)
            {
                out_items.clear();
                out_items.reserve(children.size());
                for(u32 child_index : children)
                {
                    const Element* child = context->get_element(child_index);
                    if(!child)
                    {
                        continue;
                    }
                    Float2U child_available(
                        max(available.x - child->layout.margin.x - child->layout.margin.z, 0.0f),
                        max(available.y - child->layout.margin.y - child->layout.margin.w, 0.0f));
                    MeasureResult measured = measure_element_by_index(context, child_index, child_available);
                    FlexItem item;
                    item.element_index = child_index;
                    item.measured = measured;
                    f32 main_margin = margin_begin(child->layout.margin, main_axis) + margin_end(child->layout.margin, main_axis);
                    f32 cross_margin = margin_begin(child->layout.margin, cross_axis) + margin_end(child->layout.margin, cross_axis);
                    item.outer_min = axis_value(measured.minimum, main_axis) + main_margin;
                    item.outer_desired = axis_value(measured.desired, main_axis) + main_margin;
                    item.outer_max = axis_value(measured.maximum, main_axis) + main_margin;
                    item.outer_cross_min = axis_value(measured.minimum, cross_axis) + cross_margin;
                    item.outer_cross_desired = axis_value(measured.desired, cross_axis) + cross_margin;
                    item.outer_cross_max = axis_value(measured.maximum, cross_axis) + cross_margin;
                    item.resolved_outer_main = item.outer_desired;
                    item.resolved_cross = axis_value(measured.desired, cross_axis);
                    item.grow = max(child->layout.flex_grow, 0.0f);
                    item.shrink = max(child->layout.flex_shrink, 0.0f);
                    out_items.push_back(item);
                }
            }

            void build_flex_lines(const Vector<FlexItem>& items, const FlexLayoutDesc& desc, f32 available_main,
                Vector<FlexLine>& out_lines)
            {
                out_lines.clear();
                if(items.empty())
                {
                    return;
                }
                bool wrap = desc.wrap != FlexWrap::none;
                u32 first = 0;
                u32 count = 0;
                f32 min_main = 0.0f;
                f32 desired_main = 0.0f;
                f32 cross_size = 0.0f;
                for(u32 i = 0; i < (u32)items.size(); ++i)
                {
                    const FlexItem& item = items[i];
                    f32 next_desired = item.outer_desired + (count ? desc.main_axis_gap : 0.0f);
                    if(wrap && count && desired_main + next_desired > available_main)
                    {
                        append_flex_line(out_lines, first, count, min_main, desired_main, cross_size);
                        first = i;
                        count = 0;
                        min_main = 0.0f;
                        desired_main = 0.0f;
                        cross_size = 0.0f;
                    }
                    if(count)
                    {
                        min_main += desc.main_axis_gap;
                        desired_main += desc.main_axis_gap;
                    }
                    min_main += item.outer_min;
                    desired_main += item.outer_desired;
                    cross_size = max(cross_size, item.outer_cross_desired);
                    ++count;
                }
                append_flex_line(out_lines, first, count, min_main, desired_main, cross_size);
            }

            f32 distribute_flex_line(Vector<FlexItem>& items, const FlexLine& line, f32 available_main,
                f32 gap)
            {
                f32 total_gap = line.item_count > 0 ? gap * (f32)(line.item_count - 1) : 0.0f;
                f32 target_items = max(available_main - total_gap, 0.0f);
                f32 min_items = max(line.minimum_main - total_gap, 0.0f);
                f32 desired_items = max(line.desired_main - total_gap, 0.0f);

                for(u32 i = 0; i < line.item_count; ++i)
                {
                    FlexItem& item = items[line.first_item + i];
                    item.resolved_outer_main = item.outer_desired;
                }

                if(target_items <= min_items)
                {
                    for(u32 i = 0; i < line.item_count; ++i)
                    {
                        FlexItem& item = items[line.first_item + i];
                        item.resolved_outer_main = item.outer_min;
                    }
                }
                else if(target_items < desired_items)
                {
                    f32 remaining_reduce = desired_items - target_items;
                    for(;;)
                    {
                        f32 total_weight = 0.0f;
                        for(u32 i = 0; i < line.item_count; ++i)
                        {
                            FlexItem& item = items[line.first_item + i];
                            if(item.resolved_outer_main > item.outer_min)
                            {
                                total_weight += item.shrink * max(item.outer_desired, 1.0f);
                            }
                        }
                        if(total_weight <= 0.0f || remaining_reduce <= 0.0001f)
                        {
                            break;
                        }
                        f32 applied = 0.0f;
                        for(u32 i = 0; i < line.item_count; ++i)
                        {
                            FlexItem& item = items[line.first_item + i];
                            if(item.resolved_outer_main <= item.outer_min)
                            {
                                continue;
                            }
                            f32 weight = item.shrink * max(item.outer_desired, 1.0f);
                            f32 reduce = remaining_reduce * weight / total_weight;
                            f32 old_size = item.resolved_outer_main;
                            item.resolved_outer_main = max(item.resolved_outer_main - reduce, item.outer_min);
                            applied += old_size - item.resolved_outer_main;
                        }
                        if(applied <= 0.0001f)
                        {
                            break;
                        }
                        remaining_reduce -= applied;
                    }
                }
                else
                {
                    f32 remaining_grow = target_items - desired_items;
                    for(;;)
                    {
                        f32 total_weight = 0.0f;
                        for(u32 i = 0; i < line.item_count; ++i)
                        {
                            FlexItem& item = items[line.first_item + i];
                            if(item.resolved_outer_main < item.outer_max)
                            {
                                total_weight += item.grow;
                            }
                        }
                        if(total_weight <= 0.0f || remaining_grow <= 0.0001f)
                        {
                            break;
                        }
                        f32 applied = 0.0f;
                        for(u32 i = 0; i < line.item_count; ++i)
                        {
                            FlexItem& item = items[line.first_item + i];
                            if(item.resolved_outer_main >= item.outer_max)
                            {
                                continue;
                            }
                            f32 grow = remaining_grow * item.grow / total_weight;
                            f32 old_size = item.resolved_outer_main;
                            item.resolved_outer_main = min(item.resolved_outer_main + grow, item.outer_max);
                            applied += item.resolved_outer_main - old_size;
                        }
                        if(applied <= 0.0001f)
                        {
                            break;
                        }
                        remaining_grow -= applied;
                    }
                }

                f32 used = total_gap;
                for(u32 i = 0; i < line.item_count; ++i)
                {
                    used += items[line.first_item + i].resolved_outer_main;
                }
                return used;
            }

            void resolve_axis_alignment(FlexAlignment alignment, u32 item_count, f32 free_space,
                f32 base_gap, f32& out_offset, f32& out_gap)
            {
                out_offset = 0.0f;
                out_gap = base_gap;
                free_space = max(free_space, 0.0f);
                switch(alignment)
                {
                case FlexAlignment::center:
                    out_offset = free_space * 0.5f;
                    break;
                case FlexAlignment::end:
                    out_offset = free_space;
                    break;
                case FlexAlignment::space_between:
                    if(item_count > 1)
                    {
                        out_gap += free_space / (f32)(item_count - 1);
                    }
                    break;
                case FlexAlignment::space_around:
                    if(item_count > 0)
                    {
                        f32 space = free_space / (f32)item_count;
                        out_offset = space * 0.5f;
                        out_gap += space;
                    }
                    break;
                case FlexAlignment::space_evenly:
                    if(item_count > 0)
                    {
                        f32 space = free_space / (f32)(item_count + 1);
                        out_offset = space;
                        out_gap += space;
                    }
                    break;
                default:
                    break;
                }
            }

            MeasureResult measure_flex_content(IContext* context, const Element& element,
                const Float2U& available_content, const FlexLayoutDesc& desc)
            {
                LayoutAxis main_axis = desc.axis;
                LayoutAxis cross_axis = main_axis == LayoutAxis::x ? LayoutAxis::y : LayoutAxis::x;
                Vector<u32> children = collect_children(context, element);
                Vector<FlexItem> items;
                build_flex_items(context, children, available_content, main_axis, cross_axis, items);
                Vector<FlexLine> lines;
                build_flex_lines(items, desc, axis_value(available_content, main_axis), lines);

                f32 min_main = 0.0f;
                f32 desired_main = 0.0f;
                f32 min_cross = 0.0f;
                f32 desired_cross = 0.0f;
                for(usize i = 0; i < lines.size(); ++i)
                {
                    const FlexLine& line = lines[i];
                    if(desc.wrap == FlexWrap::none)
                    {
                        min_main = line.minimum_main;
                        desired_main = line.desired_main;
                    }
                    else
                    {
                        min_main = max(min_main, line.minimum_main);
                        desired_main = max(desired_main, line.desired_main);
                    }
                    f32 line_min_cross = 0.0f;
                    for(u32 item = 0; item < line.item_count; ++item)
                    {
                        line_min_cross = max(line_min_cross, items[line.first_item + item].outer_cross_min);
                    }
                    if(i)
                    {
                        min_cross += desc.cross_axis_gap;
                        desired_cross += desc.cross_axis_gap;
                    }
                    min_cross += line_min_cross;
                    desired_cross += line.cross_size;
                }

                MeasureResult r;
                if(main_axis == LayoutAxis::x)
                {
                    r.minimum = Float2U(min_main, min_cross);
                    r.desired = Float2U(desired_main, desired_cross);
                }
                else
                {
                    r.minimum = Float2U(min_cross, min_main);
                    r.desired = Float2U(desired_cross, desired_main);
                }
                r.maximum = Float2U(F32_MAX, F32_MAX);
                return r;
            }

            MeasureResult measure_element_by_index(IContext* context, u32 element_index, const Float2U& available)
            {
                const Element* element = context->get_element(element_index);
                if(!element)
                {
                    return MeasureResult();
                }
                return measure_leaf_element(context, element_index, *element, available);
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
                    Float2U child_available(
                        max(available - child->layout.margin.x - child->layout.margin.z, 0.0f),
                        max(available - child->layout.margin.y - child->layout.margin.w, 0.0f));
                    MeasureResult measured = measure_element_by_index(context, child_index, child_available);
                    f32 desired = 0.0f;
                    if(columns)
                    {
                        desired = measured.desired.x +
                            child->layout.margin.x + child->layout.margin.z + cell.padding.x + cell.padding.z;
                    }
                    else
                    {
                        desired = measured.desired.y +
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

        LUNA_GUICORE_API MeasureResult measure_flex(IContext* context, const ElementHandle& element,
            const Float2U& available_content_size, void* userdata)
        {
            if(!context || !userdata)
            {
                return MeasureResult();
            }
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id)
            {
                return MeasureResult();
            }
            const FlexLayoutDesc& desc = *reinterpret_cast<const FlexLayoutDesc*>(userdata);
            return measure_flex_content(context, *parent, available_content_size, desc);
        }

        LUNA_GUICORE_API RV layout_flex(IContext* context, const ElementHandle& element, const RectF& rect,
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
            const FlexLayoutDesc& desc = *reinterpret_cast<const FlexLayoutDesc*>(userdata);
            LayoutAxis main_axis = desc.axis;
            LayoutAxis cross_axis = main_axis == LayoutAxis::x ? LayoutAxis::y : LayoutAxis::x;
            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            f32 available_main = rect_size(content_rect, main_axis);
            f32 available_cross = rect_size(content_rect, cross_axis);
            Float2U available_size(content_rect.width, content_rect.height);
            Vector<FlexItem> items;
            build_flex_items(context, children, available_size, main_axis, cross_axis, items);
            Vector<FlexLine> lines;
            build_flex_lines(items, desc, available_main, lines);

            for(FlexLine& line : lines)
            {
                line.resolved_main = distribute_flex_line(items, line, available_main, desc.main_axis_gap);
            }

            f32 total_cross = 0.0f;
            for(usize i = 0; i < lines.size(); ++i)
            {
                if(i)
                {
                    total_cross += desc.cross_axis_gap;
                }
                total_cross += lines[i].cross_size;
            }
            bool multiple_lines = desc.wrap != FlexWrap::none && lines.size() > 1;
            f32 line_offset = 0.0f;
            f32 line_gap = desc.cross_axis_gap;
            if(multiple_lines && desc.line_alignment == FlexAlignment::stretch && !lines.empty())
            {
                f32 extra_cross = max(available_cross - total_cross, 0.0f) / (f32)lines.size();
                for(FlexLine& line : lines)
                {
                    line.cross_size += extra_cross;
                }
                total_cross = available_cross;
            }
            else if(multiple_lines)
            {
                resolve_axis_alignment(desc.line_alignment, (u32)lines.size(), available_cross - total_cross,
                    desc.cross_axis_gap, line_offset, line_gap);
            }

            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;

            f32 measured_main = 0.0f;
            f32 measured_cross = total_cross;
            f32 cross_cursor = line_offset;
            for(u32 line_index = 0; line_index < (u32)lines.size(); ++line_index)
            {
                FlexLine& line = lines[line_index];
                f32 main_offset = 0.0f;
                f32 main_gap = desc.main_axis_gap;
                resolve_axis_alignment(desc.main_alignment, line.item_count, available_main - line.resolved_main,
                    desc.main_axis_gap, main_offset, main_gap);
                f32 main_cursor = main_offset;
                f32 line_cross_position = desc.wrap == FlexWrap::wrap_reverse ?
                    available_cross - cross_cursor - line.cross_size : cross_cursor;

                for(u32 visual_item = 0; visual_item < line.item_count; ++visual_item)
                {
                    u32 item_offset = desc.reverse ? line.item_count - 1 - visual_item : visual_item;
                    FlexItem& item = items[line.first_item + item_offset];
                    const Element* child = context->get_element(item.element_index);
                    if(!child)
                    {
                        continue;
                    }

                    f32 main_margin_begin = margin_begin(child->layout.margin, main_axis);
                    f32 main_margin_end = margin_end(child->layout.margin, main_axis);
                    f32 cross_margin_begin = margin_begin(child->layout.margin, cross_axis);
                    f32 cross_margin_end = margin_end(child->layout.margin, cross_axis);
                    f32 main_size = max(item.resolved_outer_main - main_margin_begin - main_margin_end, 0.0f);
                    f32 cross_available = max(line.cross_size - cross_margin_begin - cross_margin_end, 0.0f);
                    f32 cross_size = axis_value(item.measured.desired, cross_axis);
                    if(desc.cross_alignment == FlexAlignment::stretch)
                    {
                        cross_size = cross_available;
                    }
                    cross_size = min(max(cross_size, axis_value(item.measured.minimum, cross_axis)),
                        axis_value(item.measured.maximum, cross_axis));
                    f32 cross_extra = max(cross_available - cross_size, 0.0f);
                    f32 cross_offset = 0.0f;
                    if(desc.cross_alignment == FlexAlignment::center)
                    {
                        cross_offset = cross_extra * 0.5f;
                    }
                    else if(desc.cross_alignment == FlexAlignment::end)
                    {
                        cross_offset = cross_extra;
                    }

                    f32 child_main = rect_offset(content_rect, main_axis) + main_cursor + main_margin_begin;
                    f32 child_cross = rect_offset(content_rect, cross_axis) + line_cross_position + cross_margin_begin + cross_offset;
                    RectF child_rect;
                    if(main_axis == LayoutAxis::x)
                    {
                        child_rect = RectF(child_main, child_cross, main_size, cross_size);
                    }
                    else
                    {
                        child_rect = RectF(child_cross, child_main, cross_size, main_size);
                    }
                    LayoutResult result;
                    result.rect = child_rect;
                    result.clip_rect = desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip;
                    result.content_size = Float2U(child_rect.width, child_rect.height);
                    context->set_layout_result(ElementHandle { child->id, item.element_index, element.generation }, result);

                    main_cursor += item.resolved_outer_main;
                    if(visual_item + 1 < line.item_count)
                    {
                        main_cursor += main_gap;
                    }
                }
                measured_main = max(measured_main, line.resolved_main);
                cross_cursor += line.cross_size;
                if(line_index + 1 < lines.size())
                {
                    cross_cursor += line_gap;
                }
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

        LUNA_GUICORE_API MeasureResult measure_grid(IContext* context, const ElementHandle& element,
            const Float2U& available_content_size, void* userdata)
        {
            MeasureResult result;
            if(!context || !userdata) return result;
            const Element* parent = context->get_element(element.index);
            if(!parent || parent->id != element.id) return result;
            const GridLayoutDesc& desc = *reinterpret_cast<const GridLayoutDesc*>(userdata);
            Vector<u32> children = collect_children(context, *parent);
            if(children.empty()) return result;
            f32 cell_width = max(desc.cell_size.x, 0.0f);
            f32 cell_height = max(desc.cell_size.y, 0.0f);
            u32 columns = max(desc.column_count, 1U);
            if(desc.mode == GridLayoutMode::fixed_cell_size)
            {
                f32 stride = cell_width + desc.gap.x;
                columns = stride > 0.0f && available_content_size.x < F32_MAX * 0.5f ?
                    max((u32)((available_content_size.x + desc.gap.x) / stride), 1U) : (u32)children.size();
                columns = min(columns, (u32)children.size());
            }
            else
            {
                f32 total_gap = desc.gap.x * (f32)(columns - 1);
                cell_width = max((available_content_size.x - total_gap) / (f32)columns, 0.0f);
            }
            u32 rows = ((u32)children.size() + columns - 1) / columns;
            result.minimum = Float2U(cell_width, cell_height);
            result.desired = Float2U(cell_width * (f32)columns + desc.gap.x * (f32)(columns - 1),
                cell_height * (f32)rows + desc.gap.y * (f32)(rows - 1));
            return result;
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
            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
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
                MeasureResult measured = measure_element_by_index(context, child_index, Float2U(available_width, available_height));

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
                    width = resolve_measured_axis_size(*child, LayoutAxis::x, available_width, measured);
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
                    height = resolve_measured_axis_size(*child, LayoutAxis::y, available_height, measured);
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
            RectF content_rect = inset_rect(rect, parent->layout.padding);
            Vector<u32> children = collect_children(context, *parent);
            RectF parent_clip = inherited_layout_clip(*parent, rect);
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, parent_clip) : parent_clip;
            f32 measured_width = 0.0f;
            f32 measured_height = 0.0f;
            for(u32 child_index : children)
            {
                const Element* child = context->get_element(child_index);
                f32 available_width = max(content_rect.width - child->layout.margin.x - child->layout.margin.z, 0.0f);
                f32 available_height = max(content_rect.height - child->layout.margin.y - child->layout.margin.w, 0.0f);
                MeasureResult measured = measure_element_by_index(context, child_index, Float2U(available_width, available_height));
                const SizeValue& requested_width = requested_size(child->layout, LayoutAxis::x);
                const SizeValue& requested_height = requested_size(child->layout, LayoutAxis::y);
                f32 width = requested_width.kind == SizeKind::fit ?
                    measured.desired.x :
                    resolve_measured_axis_size(*child, LayoutAxis::x, available_width, measured);
                f32 height = requested_height.kind == SizeKind::fit ?
                    measured.desired.y :
                    resolve_measured_axis_size(*child, LayoutAxis::y, available_height, measured);
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

            Ref<ScrollViewportHistoryState> history = find_scroll_viewport_history(context, parent->id);
            if(!history)
            {
                history = new_object<ScrollViewportHistoryState>();
            }
            history->visible_rect = RectF(desc.scroll_offset.x, desc.scroll_offset.y,
                content_rect.width, content_rect.height);
            history->layout_generation = context->generation();
            RV state_result = context->set_state(make_state_id<ScrollViewportHistoryState>(parent->id),
                history.object(), StateLifetime::next_frame);
            if(failed(state_result))
            {
                return state_result;
            }
            return ok;
        }

        LUNA_GUICORE_API RectF get_scroll_viewport_visible_rect(IContext* context, const ElementHandle& element)
        {
            if(!context || element.generation != context->generation())
            {
                return RectF(0.0f, 0.0f, 0.0f, 0.0f);
            }
            const Element* viewport = context->get_element(element.index);
            if(!viewport || viewport->id != element.id)
            {
                return RectF(0.0f, 0.0f, 0.0f, 0.0f);
            }

            Ref<ScrollViewportHistoryState> history = find_scroll_viewport_history(context, viewport->id);
            if(history && context->generation() - history->layout_generation == 1)
            {
                return history->visible_rect;
            }

            FrameDesc frame_desc = context->get_frame_desc();
            return RectF(0.0f, 0.0f, max(frame_desc.screen_size.x, 0.0f),
                max(frame_desc.screen_size.y, 0.0f));
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
