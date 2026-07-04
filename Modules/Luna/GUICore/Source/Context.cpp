/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUICORE_API LUNA_EXPORT
#include "GUICore.hpp"
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include <Luna/Runtime/Time.hpp>

namespace Luna
{
    namespace GUICore
    {
        namespace
        {
            bool color_visible(const Float4U& color)
            {
                return color.w > 0.0f;
            }

            bool command_color_visible(const DrawCommand& command)
            {
                if(command.type == DrawCommandType::gradient_rect)
                {
                    return color_visible(command.color) ||
                        color_visible(command.color_top_right) ||
                        color_visible(command.color_bottom_right) ||
                        color_visible(command.color_bottom_left);
                }
                return color_visible(command.color);
            }

            bool rect_visible(const RectF& rect)
            {
                return rect.width > 0.0f && rect.height > 0.0f;
            }

            RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 min_x = max(a.offset_x, b.offset_x);
                f32 min_y = max(a.offset_y, b.offset_y);
                f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            bool has_clip(const RectF& clip_rect)
            {
                return clip_rect.width > 0.0f || clip_rect.height > 0.0f;
            }

            bool contains_name(Span<const Name> names, const Name& name)
            {
                for(const Name& candidate : names)
                {
                    if(candidate == name)
                    {
                        return true;
                    }
                }
                return false;
            }

            bool contains_id(Span<const id_t> ids, id_t id)
            {
                for(id_t candidate : ids)
                {
                    if(candidate == id)
                    {
                        return true;
                    }
                }
                return false;
            }

            Float2U element_screen_center(Span<const Layer> layers, const Element& element)
            {
                const RectF& rect = element.layout_result.rect;
                Float2U center(rect.offset_x + rect.width * 0.5f, rect.offset_y + rect.height * 0.5f);
                if(element.layer < layers.size())
                {
                    center.x += layers[element.layer].screen_position.x;
                    center.y += layers[element.layer].screen_position.y;
                }
                return center;
            }

            f64 perf_elapsed_ms(u64 begin, u64 end)
            {
                return (f64)(end - begin) * 1000.0 / get_ticks_per_second();
            }
        }

        void Context::begin_frame(const FrameDesc& desc)
        {
            lutsassert();
            ++m_generation;
            m_elapsed_time += max(desc.delta_time, 0.0f);
            m_frame_desc = desc;
            u64 gc_begin = get_ticks();
            gc_states();
            u64 gc_end = get_ticks();
            m_layers.clear();
            m_elements.clear();
            m_draw_commands.clear();
            m_input_events.clear();
            m_debug_issues.clear();
            m_debug_passes.clear();
            m_pointer_delta = Float2U(0.0f);
            m_layer_stack.clear();
            m_element_stack.clear();
            m_style_stack.clear();
            m_data_scope_stack.clear();
            m_data_scope_stack.push_back(DEFAULT_DATA_SCOPE);
            m_element_indices.clear();
            m_drag_drop.deliveries.clear();
            m_text_input_request = TextInputRequest();
            m_hovered_elements.clear();
            m_counters = PerformanceCounters();
            m_counters.frame_generation = m_generation;
            m_counters.state_gc_ms = perf_elapsed_ms(gc_begin, gc_end);
            log_debug_pass(DebugPassKind::frame, Name("begin_frame"), Name("host_frame"), 0, nullptr, 0.0);
            log_debug_pass(DebugPassKind::state, Name("gc_states"), Name("begin_frame"), 0, nullptr, m_counters.state_gc_ms);
        }

        u32 Context::generation() const
        {
            lutsassert();
            return m_generation;
        }

        FrameDesc Context::get_frame_desc() const
        {
            lutsassert();
            return m_frame_desc;
        }

        Float2U Context::get_pointer_position() const
        {
            lutsassert();
            return m_pointer_position;
        }

        Float2U Context::get_pointer_delta() const
        {
            lutsassert();
            return m_pointer_delta;
        }

        bool Context::is_pointer_inside() const
        {
            lutsassert();
            return m_pointer_inside;
        }

        bool Context::is_pointer_button_down(PointerButton button) const
        {
            lutsassert();
            u32 index = (u32)button;
            return index < 5 ? m_pointer_down[index] : false;
        }

        bool Context::is_key_down(KeyCode key) const
        {
            lutsassert();
            u32 index = (u32)key;
            return index < 256 ? m_key_down[index] : false;
        }

        KeyModifierFlag Context::get_key_modifiers() const
        {
            lutsassert();
            return m_key_modifiers;
        }

        void Context::add_input_event(const InputEvent& event)
        {
            lutsassert();
            m_input_events.push_back(event);
        }

        void Context::add_input_events(Span<const InputEvent> events)
        {
            lutsassert();
            m_input_events.insert(m_input_events.end(), events.begin(), events.end());
        }

        void Context::push_layer(id_t id, const Float2U& screen_position, const Name& debug_name)
        {
            lutsassert();
            for(const Layer& layer : m_layers)
            {
                luassert_msg(layer.id != id, "Duplicate GUI Core layer ID in one frame.");
            }
            Layer layer;
            layer.id = id;
            layer.screen_position = screen_position;
            layer.debug_name = debug_name;
            m_layers.push_back(move(layer));
            m_layer_stack.push_back((u32)m_layers.size() - 1);
        }

        void Context::pop_layer()
        {
            lutsassert();
            luassert(!m_layer_stack.empty());
            luassert(m_element_stack.empty() || m_elements[m_element_stack.back()].layer != m_layer_stack.back());
            m_layer_stack.pop_back();
        }

        void Context::push_data_scope(id_t id)
        {
            lutsassert();
            luassert_msg(id, "GUI Core data scope ID must not be zero.");
            m_data_scope_stack.push_back(make_scoped_id(current_data_scope(), id));
        }

        void Context::pop_data_scope()
        {
            lutsassert();
            luassert(m_data_scope_stack.size() > 1);
            m_data_scope_stack.pop_back();
        }

        id_t Context::current_data_scope() const
        {
            lutsassert();
            return m_data_scope_stack.empty() ? DEFAULT_DATA_SCOPE : m_data_scope_stack.back();
        }

        id_t Context::make_id(id_t local_id) const
        {
            lutsassert();
            return make_scoped_id(current_data_scope(), local_id);
        }

        id_t Context::make_id(const c8* local_name) const
        {
            lutsassert();
            return make_scoped_id(current_data_scope(), local_name);
        }

        ElementHandle Context::begin_element(id_t id, const Name& debug_name)
        {
            lutsassert();
            luassert(!m_layer_stack.empty());
            luassert_msg(m_element_indices.find(id) == m_element_indices.end(), "Duplicate GUI Core element ID in one frame.");
            u32 layer_index = m_layer_stack.back();
            u32 index = (u32)m_elements.size();
            Element element;
            element.id = id;
            element.layer = layer_index;
            element.debug_name = debug_name;
            element.style = current_style();
            if(!m_element_stack.empty() && m_elements[m_element_stack.back()].layer == layer_index)
            {
                u32 parent_index = m_element_stack.back();
                element.parent = parent_index;
                element.depth = m_elements[parent_index].depth + 1;
                Element& parent = m_elements[parent_index];
                if(parent.first_child == INVALID_ELEMENT)
                {
                    parent.first_child = index;
                }
                if(parent.last_child != INVALID_ELEMENT)
                {
                    Element& prev = m_elements[parent.last_child];
                    prev.next_sibling = index;
                    element.prev_sibling = parent.last_child;
                }
                parent.last_child = index;
            }
            else if(m_layers[layer_index].root == INVALID_ELEMENT)
            {
                m_layers[layer_index].root = index;
            }
            else
            {
                luassert_msg(false, "A GUI Core layer can only have one root element.");
            }
            m_elements.push_back(move(element));
            m_element_indices.insert(make_pair(id, index));
            m_element_stack.push_back(index);
            return ElementHandle { id, index, m_generation };
        }

        void Context::end_element()
        {
            lutsassert();
            luassert(!m_element_stack.empty());
            m_element_stack.pop_back();
        }

        Element* Context::mutable_element(const ElementHandle& element)
        {
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return nullptr;
            }
            Element& e = m_elements[element.index];
            return e.id == element.id ? &e : nullptr;
        }

        void Context::set_layout_config(const ElementHandle& element, const LayoutConfig& config)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->layout = config;
            }
        }

        void Context::set_layout_result(const ElementHandle& element, const LayoutResult& result)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->layout_result = result;
            }
        }

        RV Context::apply_layout(const ElementHandle& root, const RectF& rect)
        {
            lutsassert();
            if(!root.id || root.generation != m_generation || root.index >= m_elements.size() ||
                m_elements[root.index].id != root.id)
            {
                return BasicError::bad_arguments();
            }
            LayoutResult root_result;
            root_result.rect = rect;
            root_result.clip_rect = rect;
            root_result.content_size = Float2U(rect.width, rect.height);
            set_layout_result(root, root_result);
            return apply_layout_subtree(root);
        }

        RV Context::apply_element_layout(const ElementHandle& element)
        {
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return BasicError::bad_arguments();
            }
            Element* e = mutable_element(element);
            if(!e)
            {
                return BasicError::bad_arguments();
            }
            const LayoutConfig& config = e->layout;
            RectF rect = e->layout_result.rect;
            if(config.callback)
            {
                return config.callback(this, element, rect, config.userdata);
            }
            return ok;
        }

        RV Context::apply_layout_subtree(const ElementHandle& element)
        {
            RV r = apply_element_layout(element);
            if(failed(r))
            {
                return r;
            }
            const Element* e = get_element(element.index);
            if(!e || e->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            Vector<u32> children;
            for(u32 child = e->first_child; child != INVALID_ELEMENT;)
            {
                const Element* child_element = get_element(child);
                if(!child_element)
                {
                    break;
                }
                children.push_back(child);
                child = child_element->next_sibling;
            }
            for(u32 child_index : children)
            {
                const Element* child = get_element(child_index);
                if(!child)
                {
                    continue;
                }
                r = apply_layout_subtree(ElementHandle { child->id, child_index, m_generation });
                if(failed(r))
                {
                    return r;
                }
            }
            e = get_element(element.index);
            if(!e || e->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            const LayoutConfig& config = e->layout;
            if(config.finalize_callback)
            {
                return config.finalize_callback(this, element, e->layout_result.rect, config.userdata);
            }
            return ok;
        }

        void Context::set_interactable(const ElementHandle& element, const Interactable& interactable)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->interactable = interactable;
            }
        }

        void Context::set_navigation_config(const ElementHandle& element, const NavigationConfig& navigation)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->navigation = navigation;
            }
        }

        NavigationConfig Context::get_navigation_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return NavigationConfig();
            }
            const Element& e = m_elements[element.index];
            return e.id == element.id ? e.navigation : NavigationConfig();
        }

        void Context::set_hit_test_config(const ElementHandle& element, const ElementHitTestConfig& hit_test)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->hit_test = hit_test;
            }
        }

        ElementHitTestConfig Context::get_hit_test_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return ElementHitTestConfig();
            }
            const Element& e = m_elements[element.index];
            return e.id == element.id ? e.hit_test : ElementHitTestConfig();
        }

        void Context::set_drag_drop_source_types(const ElementHandle& element, Span<const Name> types)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->interactable.drag_source_types.assign(types.begin(), types.end());
            }
        }

        void Context::set_drag_drop_target_types(const ElementHandle& element, Span<const Name> types)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->interactable.drag_target_types.assign(types.begin(), types.end());
            }
        }

        void Context::bind_style(const ElementHandle& element, const Name& style)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->style = style;
            }
        }

        const Element* Context::get_element(u32 index) const
        {
            lutsassert();
            return index < m_elements.size() ? &m_elements[index] : nullptr;
        }

        const Element* Context::find_element(id_t id) const
        {
            lutsassert();
            auto iter = m_element_indices.find(id);
            return iter == m_element_indices.end() ? nullptr : get_element(iter->second);
        }

        ElementHandle Context::find_element_handle(id_t id) const
        {
            lutsassert();
            auto iter = m_element_indices.find(id);
            if(iter == m_element_indices.end())
            {
                return ElementHandle();
            }
            return ElementHandle { id, iter->second, m_generation };
        }

        Span<const DrawCommand> Context::get_draw_commands() const
        {
            lutsassert();
            return m_draw_commands.cspan();
        }

        void Context::record_draw_command(u32 layer_index, u32 element_index, const DrawCommand& command)
        {
            DrawCommand cmd = command;
            cmd.layer = layer_index;
            cmd.element = element_index;
            u32 command_index = (u32)m_draw_commands.size();
            if(element_index != INVALID_ELEMENT)
            {
                Element& e = m_elements[element_index];
                if(e.first_draw_command == U32_MAX)
                {
                    e.first_draw_command = command_index;
                }
                ++e.draw_command_count;
            }
            Layer& layer = m_layers[layer_index];
            if(layer.first_draw_command == U32_MAX)
            {
                layer.first_draw_command = command_index;
            }
            ++layer.draw_command_count;
            layer.draw_command_indices.push_back(command_index);
            m_draw_commands.push_back(move(cmd));
        }

        void Context::draw(const DrawCommand& command)
        {
            lutsassert();
            luassert(!m_layer_stack.empty());
            u32 layer_index = m_layer_stack.back();
            u32 element_index = m_element_stack.empty() ? INVALID_ELEMENT : m_element_stack.back();
            record_draw_command(layer_index, element_index, command);
        }

        void Context::draw_for_element(const ElementHandle& element, const DrawCommand& command)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e || e->layer >= m_layers.size())
            {
                return;
            }
            record_draw_command(e->layer, element.index, command);
        }

        RectF Context::to_screen_rect(u32 layer_index, const RectF& rect) const
        {
            if(layer_index >= m_layers.size())
            {
                return rect;
            }
            const Float2U& layer_position = m_layers[layer_index].screen_position;
            return RectF(layer_position.x + rect.offset_x, layer_position.y + rect.offset_y, rect.width, rect.height);
        }

        RectF Context::to_vg_rect(const RectF& screen_rect) const
        {
            return RectF(screen_rect.offset_x, m_frame_desc.screen_size.y - screen_rect.offset_y - screen_rect.height,
                screen_rect.width, screen_rect.height);
        }

        RectF resolve_draw_rect(const DrawCommand& command, Span<const Element> elements)
        {
            if(command.rect_reference != DrawCommandRectReference::element ||
                command.element == INVALID_ELEMENT ||
                command.element >= elements.size())
            {
                return command.rect;
            }
            const RectF& element_rect = elements[command.element].layout_result.rect;
            RectF ret;
            ret.offset_x = element_rect.offset_x + command.rect.offset_x + element_rect.width * command.rect_layout_scale.x;
            ret.offset_y = element_rect.offset_y + command.rect.offset_y + element_rect.height * command.rect_layout_scale.y;
            f32 scaled_width = command.rect.width + element_rect.width * command.rect_layout_scale.z;
            f32 scaled_height = command.rect.height + element_rect.height * command.rect_layout_scale.w;
            ret.width = scaled_width > 0.0f ?
                scaled_width :
                max(element_rect.width - command.rect.offset_x + command.rect.width, 1.0f);
            ret.height = scaled_height > 0.0f ?
                scaled_height :
                max(element_rect.height - command.rect.offset_y + command.rect.height, 1.0f);
            return ret;
        }

        Float2U resolve_draw_point0(const DrawCommand& command, Span<const Element> elements)
        {
            if(command.rect_reference != DrawCommandRectReference::element ||
                command.element == INVALID_ELEMENT ||
                command.element >= elements.size())
            {
                return Float2U(command.rect.offset_x, command.rect.offset_y);
            }
            const RectF& element_rect = elements[command.element].layout_result.rect;
            return Float2U(
                element_rect.offset_x + command.rect.offset_x + element_rect.width * command.rect_layout_scale.x,
                element_rect.offset_y + command.rect.offset_y + element_rect.height * command.rect_layout_scale.y);
        }

        Float2U resolve_draw_point1(const DrawCommand& command, Span<const Element> elements)
        {
            if(command.rect_reference != DrawCommandRectReference::element ||
                command.element == INVALID_ELEMENT ||
                command.element >= elements.size())
            {
                return command.point1;
            }
            const RectF& element_rect = elements[command.element].layout_result.rect;
            return Float2U(
                element_rect.offset_x + command.point1.x + element_rect.width * command.rect_layout_scale.z,
                element_rect.offset_y + command.point1.y + element_rect.height * command.rect_layout_scale.w);
        }

        RV Context::compile_draw_commands(VG::IShapeDrawList* draw_list)
        {
            lutsassert();
            if(!draw_list)
            {
                return BasicError::bad_arguments();
            }
            u64 compile_begin = get_ticks();
            draw_list->reset();
            Vector<RectF> clip_stack;
            RHI::SamplerDesc nearest_sampler_desc;
            nearest_sampler_desc.min_filter = RHI::Filter::nearest;
            nearest_sampler_desc.mag_filter = RHI::Filter::nearest;
            nearest_sampler_desc.mip_filter = RHI::Filter::nearest;
            nearest_sampler_desc.address_u = RHI::TextureAddressMode::clamp;
            nearest_sampler_desc.address_v = RHI::TextureAddressMode::clamp;
            nearest_sampler_desc.address_w = RHI::TextureAddressMode::clamp;
            for(u32 layer_index = 0; layer_index < m_layers.size(); ++layer_index)
            {
                const Layer& layer = m_layers[layer_index];
                if(layer.first_draw_command == U32_MAX || layer.draw_command_count == 0)
                {
                    continue;
                }
                clip_stack.clear();
                for(u32 command_index : layer.draw_command_indices)
                {
                    if(command_index >= m_draw_commands.size())
                    {
                        continue;
                    }
                    const DrawCommand& command = m_draw_commands[command_index];
                    RectF resolved_rect = resolve_draw_rect(command, Span<const Element>(m_elements.data(), m_elements.size()));
                    if(command.type == DrawCommandType::push_clip)
                    {
                        RectF screen_clip = to_screen_rect(layer_index, resolved_rect);
                        if(!clip_stack.empty())
                        {
                            screen_clip = intersect_rect(clip_stack.back(), screen_clip);
                        }
                        clip_stack.push_back(screen_clip);
                        continue;
                    }
                    if(command.type == DrawCommandType::pop_clip)
                    {
                        if(!clip_stack.empty())
                        {
                            clip_stack.pop_back();
                        }
                        continue;
                    }

                    RectF clip_rect = clip_stack.empty() ? RectF(0.0f, 0.0f, 0.0f, 0.0f) : clip_stack.back();
                    RectF vg_clip = has_clip(clip_rect) ? to_vg_rect(clip_rect) : RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    draw_list->set_clip_rect(vg_clip);
                    draw_list->set_texture(nullptr);
                    draw_list->set_shape_buffer(nullptr);
                    draw_list->set_sampler(nullptr);

                    switch(command.type)
                    {
                    case DrawCommandType::rect:
                    case DrawCommandType::gradient_rect:
                    case DrawCommandType::rounded_rect:
                    case DrawCommandType::image:
                    {
                        RectF screen_rect = to_screen_rect(layer_index, resolved_rect);
                        if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect)))
                        {
                            break;
                        }
                        if(!rect_visible(screen_rect) || !command_color_visible(command))
                        {
                            break;
                        }
                        RectF vg_rect = to_vg_rect(screen_rect);
                        draw_list->set_texture(command.type == DrawCommandType::image ? command.texture : nullptr);
                        if(command.type == DrawCommandType::image && command.nearest_sampler)
                        {
                            draw_list->set_sampler(&nearest_sampler_desc);
                        }
                        auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
                        u32 begin = (u32)points.size();
                        if(command.type == DrawCommandType::rounded_rect && command.radius > 0.0f)
                        {
                            VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0.0f, 0.0f, vg_rect.width, vg_rect.height,
                                min(command.radius, min(vg_rect.width, vg_rect.height) * 0.5f));
                        }
                        else
                        {
                            VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f, vg_rect.width, vg_rect.height);
                        }
                        u32 end = (u32)points.size();
                        if(command.type == DrawCommandType::gradient_rect)
                        {
                            VG::Vertex vertices[4];
                            vertices[0].position = Float2U(vg_rect.offset_x, vg_rect.offset_y);
                            vertices[0].shapecoord = Float2U(0.0f, 0.0f);
                            vertices[0].texcoord = command.min_texcoord;
                            vertices[0].begin_command = begin;
                            vertices[0].num_commands = end - begin;
                            vertices[0].color = command.color_bottom_left;
                            vertices[1].position = Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y);
                            vertices[1].shapecoord = Float2U(vg_rect.width, 0.0f);
                            vertices[1].texcoord = Float2U(command.max_texcoord.x, command.min_texcoord.y);
                            vertices[1].begin_command = begin;
                            vertices[1].num_commands = end - begin;
                            vertices[1].color = command.color_bottom_right;
                            vertices[2].position = Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height);
                            vertices[2].shapecoord = Float2U(vg_rect.width, vg_rect.height);
                            vertices[2].texcoord = command.max_texcoord;
                            vertices[2].begin_command = begin;
                            vertices[2].num_commands = end - begin;
                            vertices[2].color = command.color_top_right;
                            vertices[3].position = Float2U(vg_rect.offset_x, vg_rect.offset_y + vg_rect.height);
                            vertices[3].shapecoord = Float2U(0.0f, vg_rect.height);
                            vertices[3].texcoord = Float2U(command.min_texcoord.x, command.max_texcoord.y);
                            vertices[3].begin_command = begin;
                            vertices[3].num_commands = end - begin;
                            vertices[3].color = command.color;
                            u32 indices[] = {0, 1, 2, 0, 2, 3};
                            draw_list->draw_shape_raw(Span<const VG::Vertex>(vertices, 4), Span<const u32>(indices, 6));
                        }
                        else
                        {
                            draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y), Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f, 0.0f), Float2U(vg_rect.width, vg_rect.height),
                                command.color, command.min_texcoord, command.max_texcoord);
                        }
                        break;
                    }
                    case DrawCommandType::line:
                    {
                        if(!color_visible(command.color) || command.line_width <= 0.0f)
                        {
                            break;
                        }
                        Float2U resolved_p0 = resolve_draw_point0(command, Span<const Element>(m_elements.data(), m_elements.size()));
                        Float2U resolved_p1 = resolve_draw_point1(command, Span<const Element>(m_elements.data(), m_elements.size()));
                        Float2U p0(m_layers[layer_index].screen_position.x + resolved_p0.x,
                            m_layers[layer_index].screen_position.y + resolved_p0.y);
                        Float2U p1(m_layers[layer_index].screen_position.x + resolved_p1.x,
                            m_layers[layer_index].screen_position.y + resolved_p1.y);
                        f32 margin = max(command.line_width, 1.0f);
                        f32 dx = p1.x > p0.x ? p1.x - p0.x : p0.x - p1.x;
                        f32 dy = p1.y > p0.y ? p1.y - p0.y : p0.y - p1.y;
                        RectF bounds(min(p0.x, p1.x) - margin, min(p0.y, p1.y) - margin,
                            max(dx + margin * 2.0f, 1.0f), max(dy + margin * 2.0f, 1.0f));
                        if(has_clip(clip_rect) && !rect_visible(intersect_rect(bounds, clip_rect)))
                        {
                            break;
                        }
                        RectF vg_rect = to_vg_rect(bounds);
                        auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
                        u32 begin = (u32)points.size();
                        Float2U local_p0(p0.x - bounds.offset_x, bounds.height - (p0.y - bounds.offset_y));
                        Float2U local_p1(p1.x - bounds.offset_x, bounds.height - (p1.y - bounds.offset_y));
                        VG::ShapeBuilder::add_line(points, local_p0.x, local_p0.y, local_p1.x, local_p1.y, command.line_width);
                        u32 end = (u32)points.size();
                        draw_list->draw_shape(begin, end - begin,
                            Float2U(vg_rect.offset_x, vg_rect.offset_y), Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                            Float2U(0.0f, 0.0f), Float2U(vg_rect.width, vg_rect.height),
                            command.color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
                        break;
                    }
                    case DrawCommandType::text:
                    {
                        if(command.text.empty() || command.font_size <= 0.0f || !color_visible(command.color))
                        {
                            break;
                        }
                        RectF screen_rect = to_screen_rect(layer_index, resolved_rect);
                        if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect)))
                        {
                            break;
                        }
                        if(!rect_visible(screen_rect))
                        {
                            break;
                        }
                        if(!m_font_atlas)
                        {
                            m_font_atlas = VG::new_font_atlas();
                        }
                        FontDesc font = resolve_font(command.font);
                        VG::TextArrangeSection section;
                        section.font_file = font.font;
                        section.font_index = font.font_index;
                        section.font_size = command.font_size;
                        section.color = command.color;
                        section.num_chars = command.text.size();
                        RectF vg_rect = to_vg_rect(screen_rect);
                        VG::TextArrangeResult arranged = VG::arrange_text(command.text.c_str(), command.text.size(),
                            Span<const VG::TextArrangeSection>(&section, 1), vg_rect,
                            command.vertical_alignment, command.horizontal_alignment);
                        Vector<VG::Vertex> vertices;
                        Vector<u32> indices;
                        VG::generate_text_arrange_result_draw_vertices(arranged,
                            Span<const VG::TextArrangeSection>(&section, 1), m_font_atlas, vertices, indices);
                        draw_list->set_shape_buffer(m_font_atlas->get_shape_buffer());
                        draw_list->set_texture(nullptr);
                        draw_list->draw_shape_raw(vertices.cspan(), indices.cspan());
                        break;
                    }
                    case DrawCommandType::shape:
                    {
                        if(!command.shape.buffer || command.shape.num_commands == 0 || !rect_visible(command.shape.bounds) ||
                            !rect_visible(resolved_rect) || !color_visible(command.color))
                        {
                            break;
                        }
                        RectF screen_rect = to_screen_rect(layer_index, resolved_rect);
                        if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect)))
                        {
                            break;
                        }
                        RectF vg_rect = to_vg_rect(screen_rect);
                        draw_list->set_shape_buffer(command.shape.buffer);
                        draw_list->set_texture(command.shape.texture);
                        if(command.shape.texture && command.nearest_sampler)
                        {
                            draw_list->set_sampler(&nearest_sampler_desc);
                        }
                        Float2U shape_min(command.shape.bounds.offset_x, command.shape.bounds.offset_y + command.shape.bounds.height);
                        Float2U shape_max(command.shape.bounds.offset_x + command.shape.bounds.width, command.shape.bounds.offset_y);
                        draw_list->draw_shape(command.shape.first_command, command.shape.num_commands,
                            Float2U(vg_rect.offset_x, vg_rect.offset_y), Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                            shape_min, shape_max, command.color, command.min_texcoord, command.max_texcoord);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            draw_list->set_texture(nullptr);
            draw_list->set_shape_buffer(nullptr);
            draw_list->set_sampler(nullptr);
            draw_list->set_clip_rect(RectF(0.0f, 0.0f, 0.0f, 0.0f));
            RV r = draw_list->compile();
            m_counters.draw_compile_ms = perf_elapsed_ms(compile_begin, get_ticks());
            log_debug_pass(DebugPassKind::render, Name("compile_draw_commands"), Name("explicit_compile"), 0, nullptr,
                m_counters.draw_compile_ms);
            return r;
        }

        RV Context::register_font(const Name& id, Font::IFontFile* font, u32 font_index)
        {
            lutsassert();
            if(id.empty() || !font || font_index >= font->get_num_fonts())
            {
                return BasicError::bad_arguments();
            }
            if(m_fonts.find(id) != m_fonts.end())
            {
                return BasicError::already_exists();
            }
            FontResource resource;
            resource.font = font;
            resource.font_index = font_index;
            m_fonts.insert(make_pair(id, move(resource)));
            return ok;
        }

        FontDesc Context::get_font(const Name& id)
        {
            lutsassert();
            auto iter = m_fonts.find(id);
            if(iter == m_fonts.end())
            {
                return FontDesc();
            }
            FontDesc ret;
            ret.font = iter->second.font.get();
            ret.font_index = iter->second.font_index;
            return ret;
        }

        void Context::set_clipboard_io(const ClipboardIO& io)
        {
            lutsassert();
            m_clipboard_io = io;
        }

        ClipboardIO Context::get_clipboard_io()
        {
            lutsassert();
            return m_clipboard_io;
        }

        void Context::request_text_input(const ElementHandle& element, i32 cursor)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e)
            {
                return;
            }
            m_text_input_request.element_id = e->id;
            m_text_input_request.cursor = cursor;
        }

        TextInputState Context::get_text_input_state()
        {
            lutsassert();
            TextInputState ret;
            if(!m_text_input_request.element_id)
            {
                return ret;
            }
            const Element* element = find_element(m_text_input_request.element_id);
            if(!element || element->layer >= m_layers.size())
            {
                return ret;
            }
            ret.active = true;
            ret.rect = to_screen_rect(element->layer, element->layout_result.rect);
            ret.cursor = m_text_input_request.cursor;
            return ret;
        }

        FontDesc Context::resolve_font(const Name& id) const
        {
            auto iter = id.empty() ? m_fonts.end() : m_fonts.find(id);
            if(iter != m_fonts.end())
            {
                FontDesc ret;
                ret.font = iter->second.font.get();
                ret.font_index = iter->second.font_index;
                return ret;
            }
            FontDesc ret;
            ret.font = Font::get_default_font();
            ret.font_index = 0;
            return ret;
        }

        InteractionState& Context::get_or_create_interaction(id_t id)
        {
            auto iter = m_interactions.find(id);
            if(iter == m_interactions.end())
            {
                InteractionState state;
                iter = m_interactions.insert(make_pair(id, state)).first;
            }
            return iter->second;
        }

        static Float2U element_local_position(const Element& element, const Layer& layer, const Float2U& screen_position)
        {
            return Float2U(
                screen_position.x - layer.screen_position.x - element.layout_result.rect.offset_x,
                screen_position.y - layer.screen_position.y - element.layout_result.rect.offset_y);
        }

        static bool pointer_event_has_position(InputEventType type)
        {
            return type == InputEventType::pointer_enter ||
                type == InputEventType::pointer_leave ||
                type == InputEventType::pointer_move ||
                type == InputEventType::pointer_down ||
                type == InputEventType::pointer_up ||
                type == InputEventType::pointer_wheel;
        }

        void Context::mark_subtree_interaction(id_t id, bool hovered, bool active, bool focused, bool clicked, bool double_clicked)
        {
            const Element* element = find_element(id);
            while(element)
            {
                InteractionState& state = get_or_create_interaction(element->id);
                state.subtree_hovered = state.subtree_hovered || hovered;
                state.subtree_active = state.subtree_active || active;
                state.subtree_focused = state.subtree_focused || focused;
                state.subtree_clicked = state.subtree_clicked || clicked;
                state.subtree_double_clicked = state.subtree_double_clicked || double_clicked;
                if(element->parent == INVALID_ELEMENT)
                {
                    break;
                }
                element = get_element(element->parent);
            }
        }

        bool Context::point_hits_element(const Element& element, const Float2U& screen_position) const
        {
            if(element.interactable.pointer_hit_behavior == PointerHitBehavior::none ||
                has_flags(element.interactable, InteractableFlag::disabled) || element.layer >= m_layers.size())
            {
                return false;
            }
            const Layer& layer = m_layers[element.layer];
            Float2U p(screen_position.x - layer.screen_position.x, screen_position.y - layer.screen_position.y);
            const RectF& rect = element.layout_result.rect;
            if(p.x < rect.offset_x || p.y < rect.offset_y ||
                p.x >= rect.offset_x + rect.width || p.y >= rect.offset_y + rect.height)
            {
                return false;
            }
            const RectF& clip = element.layout_result.clip_rect;
            if(clip.width > 0.0f || clip.height > 0.0f)
            {
                if(p.x < clip.offset_x || p.y < clip.offset_y ||
                    p.x >= clip.offset_x + clip.width || p.y >= clip.offset_y + clip.height)
                {
                    return false;
                }
            }
            if(element.hit_test.mode == ElementHitTestMode::callback)
            {
                if(!element.hit_test.callback)
                {
                    return false;
                }
                ElementHitTestRequest request;
                request.source = element.id;
                request.screen_position = screen_position;
                request.element_position = Float2U(p.x - rect.offset_x, p.y - rect.offset_y);
                request.element_rect = rect;
                request.element_clip_rect = clip;
                request.screen_rect = RectF(
                    layer.screen_position.x + rect.offset_x,
                    layer.screen_position.y + rect.offset_y,
                    rect.width,
                    rect.height);
                request.screen_clip_rect = RectF(
                    layer.screen_position.x + clip.offset_x,
                    layer.screen_position.y + clip.offset_y,
                    clip.width,
                    clip.height);
                return element.hit_test.callback(this, request, element.hit_test.userdata);
            }
            return true;
        }

        ElementHandle Context::hit_test(const Float2U& screen_position, HitTestCallback* callback, void* userdata) const
        {
            lutsassert();
            for(usize layer_reverse_index = m_layers.size(); layer_reverse_index > 0; --layer_reverse_index)
            {
                u32 layer_index = (u32)(layer_reverse_index - 1);
                for(usize element_reverse_index = m_elements.size(); element_reverse_index > 0; --element_reverse_index)
                {
                    u32 element_index = (u32)(element_reverse_index - 1);
                    const Element& element = m_elements[element_index];
                    if(element.layer != layer_index)
                    {
                        continue;
                    }
                    if(!point_hits_element(element, screen_position))
                    {
                        continue;
                    }
                    ElementHandle handle { element.id, element_index, m_generation };
                    PointerHitBehavior hit_behavior = element.interactable.pointer_hit_behavior;
                    bool is_event_target = hit_behavior == PointerHitBehavior::target ||
                        hit_behavior == PointerHitBehavior::pass_through;
                    bool is_routing_stop = hit_behavior == PointerHitBehavior::target || hit_behavior == PointerHitBehavior::block;
                    if(callback)
                    {
                        HitTestVisit visit;
                        visit.element = handle;
                        visit.element_data = &element;
                        visit.event_target = is_event_target;
                        visit.routing_stop = is_routing_stop;
                        visit.pointer_hit_behavior = hit_behavior;
                        callback(visit, userdata);
                    }
                    if(is_routing_stop)
                    {
                        return handle;
                    }
                }
            }
            return ElementHandle();
        }

        InteractionState Context::get_interaction_state(id_t id) const
        {
            lutsassert();
            auto iter = m_interactions.find(id);
            return iter == m_interactions.end() ? InteractionState() : iter->second;
        }

        Span<const InputEvent> Context::get_delivered_input_events(id_t id)
        {
            lutsassert();
            auto iter = m_input_deliveries.find(id);
            return iter == m_input_deliveries.end() ? Span<const InputEvent>() : iter->second.cspan();
        }

        Span<const RoutedInputEvent> Context::get_routed_input_events(id_t id)
        {
            lutsassert();
            auto iter = m_routed_input_deliveries.find(id);
            return iter == m_routed_input_deliveries.end() ? Span<const RoutedInputEvent>() : iter->second.cspan();
        }

        void Context::deliver_input_event(id_t id, const InputEvent& event)
        {
            if(!id)
            {
                return;
            }
            auto element_iter = m_element_indices.find(id);
            if(element_iter == m_element_indices.end())
            {
                return;
            }
            auto iter = m_input_deliveries.find(id);
            if(iter == m_input_deliveries.end())
            {
                Vector<InputEvent> events;
                iter = m_input_deliveries.insert(make_pair(id, move(events))).first;
            }
            iter->second.push_back(event);

            auto routed_iter = m_routed_input_deliveries.find(id);
            if(routed_iter == m_routed_input_deliveries.end())
            {
                Vector<RoutedInputEvent> events;
                routed_iter = m_routed_input_deliveries.insert(make_pair(id, move(events))).first;
            }
            RoutedInputEvent routed_event;
            routed_event.event = event;
            if(pointer_event_has_position(event.type))
            {
                const Element& element = m_elements[element_iter->second];
                if(element.layer < m_layers.size())
                {
                    routed_event.has_element_position = true;
                    routed_event.element_position = element_local_position(element, m_layers[element.layer], event.position);
                    InteractionState& state = get_or_create_interaction(id);
                    state.pointer_screen_position = event.position;
                    state.pointer_element_position = routed_event.element_position;
                    state.pointer_element_rect = element.layout_result.rect;
                }
            }
            routed_iter->second.push_back(move(routed_event));
        }

        bool Context::element_can_focus(const Element& element) const
        {
            return has_flags(element.interactable, InteractableFlag::focusable) && !has_flags(element.interactable, InteractableFlag::disabled);
        }

        bool Context::element_has_drag_source_type(const Element& element, const Name& payload_type) const
        {
            return !payload_type.empty() && contains_name(element.interactable.drag_source_types.cspan(), payload_type);
        }

        bool Context::element_has_drag_target_type(const Element& element, const Name& payload_type) const
        {
            return !payload_type.empty() && contains_name(element.interactable.drag_target_types.cspan(), payload_type);
        }

        id_t Context::focus_scope_of(id_t element_id) const
        {
            const Element* element = find_element(element_id);
            while(element)
            {
                if(element->interactable.focus_scope)
                {
                    return element->interactable.focus_scope;
                }
                if(element->parent == INVALID_ELEMENT)
                {
                    break;
                }
                element = get_element(element->parent);
            }
            return 0;
        }

        id_t Context::scroll_target_of(id_t element_id) const
        {
            const Element* element = find_element(element_id);
            while(element)
            {
                if(has_flags(element->interactable, InteractableFlag::scrollable) && !has_flags(element->interactable, InteractableFlag::disabled))
                {
                    return element->id;
                }
                if(element->parent == INVALID_ELEMENT)
                {
                    break;
                }
                element = get_element(element->parent);
            }
            return 0;
        }

        void Context::focus_element(id_t id)
        {
            lutsassert();
            if(!id)
            {
                m_focused_element = 0;
                return;
            }
            const Element* element = find_element(id);
            if(element && element_can_focus(*element))
            {
                m_focused_element = id;
            }
        }

        id_t Context::focused_element() const
        {
            lutsassert();
            return m_focused_element;
        }

        void Context::capture_pointer(id_t id)
        {
            lutsassert();
            if(!id)
            {
                m_pointer_capture_element = 0;
                m_active_elements.clear();
                return;
            }
            const Element* element = find_element(id);
            if(element && has_flags(element->interactable, InteractableFlag::activatable) && !has_flags(element->interactable, InteractableFlag::disabled) && !has_flags(element->interactable, InteractableFlag::read_only))
            {
                m_pointer_capture_element = id;
                m_active_elements.clear();
                m_active_elements.push_back(id);
            }
        }

        void Context::release_pointer_capture(id_t id)
        {
            lutsassert();
            if(!id || m_pointer_capture_element == id)
            {
                m_pointer_capture_element = 0;
                m_active_elements.clear();
                return;
            }
        }

        id_t Context::captured_element() const
        {
            lutsassert();
            return m_pointer_capture_element;
        }

        RV Context::start_drag_drop(const ElementHandle& source, const Name& payload_type, const void* data, usize data_size)
        {
            lutsassert();
            const Element* element = get_element(source.index);
            if(!element || element->id != source.id || source.generation != m_generation ||
                !element_has_drag_source_type(*element, payload_type) || (data_size && !data))
            {
                return BasicError::bad_arguments();
            }
            m_drag_drop.active = true;
            m_drag_drop.source_id = source.id;
            m_drag_drop.type = payload_type;
            m_drag_drop.data.resize(data_size);
            if(data_size)
            {
                memcpy(m_drag_drop.data.data(), data, data_size);
            }
            return ok;
        }

        void Context::clear_drag_drop()
        {
            lutsassert();
            m_drag_drop.active = false;
            m_drag_drop.source_id = 0;
            m_drag_drop.type.reset();
            m_drag_drop.data.clear();
        }

        bool Context::is_drag_drop_active() const
        {
            lutsassert();
            return m_drag_drop.active;
        }

        const DragDropPayload* Context::get_drag_drop_payload()
        {
            lutsassert();
            if(!m_drag_drop.active)
            {
                return nullptr;
            }
            m_drag_drop.payload_view.type = m_drag_drop.type;
            m_drag_drop.payload_view.data = m_drag_drop.data.empty() ? nullptr : m_drag_drop.data.data();
            m_drag_drop.payload_view.data_size = m_drag_drop.data.size();
            auto iter = m_element_indices.find(m_drag_drop.source_id);
            m_drag_drop.payload_view.source = iter == m_element_indices.end() ?
                ElementHandle() : ElementHandle { m_drag_drop.source_id, iter->second, m_generation };
            m_drag_drop.payload_view.target = ElementHandle();
            m_drag_drop.payload_view.delivery = false;
            return &m_drag_drop.payload_view;
        }

        ElementHandle Context::hit_test_drag_drop_target(const Name& payload_type, const Float2U& screen_position) const
        {
            lutsassert();
            if(payload_type.empty())
            {
                return ElementHandle();
            }
            for(usize layer_reverse_index = m_layers.size(); layer_reverse_index > 0; --layer_reverse_index)
            {
                u32 layer_index = (u32)(layer_reverse_index - 1);
                for(usize element_reverse_index = m_elements.size(); element_reverse_index > 0; --element_reverse_index)
                {
                    u32 element_index = (u32)(element_reverse_index - 1);
                    const Element& element = m_elements[element_index];
                    if(element.layer != layer_index || element.id == m_drag_drop.source_id)
                    {
                        continue;
                    }
                    if(point_hits_element(element, screen_position))
                    {
                        if(element_has_drag_target_type(element, payload_type))
                        {
                            return ElementHandle { element.id, element_index, m_generation };
                        }
                        if(element.interactable.pointer_hit_behavior == PointerHitBehavior::target ||
                            element.interactable.pointer_hit_behavior == PointerHitBehavior::block)
                        {
                            return ElementHandle();
                        }
                    }
                }
            }
            return ElementHandle();
        }

        const DragDropPayload* Context::make_drag_drop_payload_view(const DragDropPayloadStorage& storage)
        {
            m_drag_drop.payload_view.type = storage.type;
            m_drag_drop.payload_view.data = storage.data.empty() ? nullptr : storage.data.data();
            m_drag_drop.payload_view.data_size = storage.data.size();
            m_drag_drop.payload_view.source = storage.source;
            m_drag_drop.payload_view.target = storage.target;
            m_drag_drop.payload_view.delivery = storage.delivery;
            return &m_drag_drop.payload_view;
        }

        void Context::deliver_drag_drop_payload(const ElementHandle& target)
        {
            if(!m_drag_drop.active || !target.id)
            {
                return;
            }
            DragDropPayloadStorage storage;
            storage.type = m_drag_drop.type;
            storage.data = m_drag_drop.data;
            auto source_iter = m_element_indices.find(m_drag_drop.source_id);
            storage.source = source_iter == m_element_indices.end() ?
                ElementHandle() : ElementHandle { m_drag_drop.source_id, source_iter->second, m_generation };
            storage.target = target;
            storage.delivery = true;
            m_drag_drop.deliveries.insert_or_assign(target.id, move(storage));
        }

        const DragDropPayload* Context::get_drag_drop_delivery(const ElementHandle& target, const Name& payload_type)
        {
            lutsassert();
            const Element* element = get_element(target.index);
            if(!element || element->id != target.id || target.generation != m_generation || payload_type.empty())
            {
                return nullptr;
            }
            auto iter = m_drag_drop.deliveries.find(target.id);
            if(iter == m_drag_drop.deliveries.end() || iter->second.type != payload_type)
            {
                return nullptr;
            }
            return make_drag_drop_payload_view(iter->second);
        }

        bool Context::move_focus(bool reverse)
        {
            id_t old_focus = m_focused_element;
            id_t scope = focus_scope_of(m_focused_element);
            Vector<id_t> candidates;
            for(const Element& element : m_elements)
            {
                if(element_can_focus(element) && focus_scope_of(element.id) == scope)
                {
                    candidates.push_back(element.id);
                }
            }
            if(candidates.empty())
            {
                m_focused_element = 0;
                return old_focus != m_focused_element;
            }
            usize current_index = USIZE_MAX;
            for(usize i = 0; i < candidates.size(); ++i)
            {
                if(candidates[i] == m_focused_element)
                {
                    current_index = i;
                    break;
                }
            }
            if(current_index == USIZE_MAX)
            {
                m_focused_element = reverse ? candidates.back() : candidates.front();
                return old_focus != m_focused_element;
            }
            if(reverse)
            {
                m_focused_element = current_index == 0 ? candidates.back() : candidates[current_index - 1];
            }
            else
            {
                m_focused_element = candidates[(current_index + 1) % candidates.size()];
            }
            return old_focus != m_focused_element;
        }

        bool Context::move_focus_spatial(NavigationDirection direction)
        {
            const Element* current = find_element(m_focused_element);
            if(!current || !element_can_focus(*current))
            {
                id_t old_focus = m_focused_element;
                move_focus(false);
                return m_focused_element != old_focus;
            }

            id_t scope = focus_scope_of(m_focused_element);
            Float2U current_center = element_screen_center(m_layers.cspan(), *current);
            id_t best_id = 0;
            f32 best_score = F32_MAX;
            f32 best_primary = F32_MAX;
            for(const Element& candidate : m_elements)
            {
                if(candidate.id == m_focused_element || !element_can_focus(candidate) || focus_scope_of(candidate.id) != scope)
                {
                    continue;
                }
                Float2U candidate_center = element_screen_center(m_layers.cspan(), candidate);
                f32 dx = candidate_center.x - current_center.x;
                f32 dy = candidate_center.y - current_center.y;
                f32 primary = 0.0f;
                f32 secondary = 0.0f;
                switch(direction)
                {
                case NavigationDirection::left:
                    if(dx >= 0.0f) continue;
                    primary = -dx;
                    secondary = dy >= 0.0f ? dy : -dy;
                    break;
                case NavigationDirection::right:
                    if(dx <= 0.0f) continue;
                    primary = dx;
                    secondary = dy >= 0.0f ? dy : -dy;
                    break;
                case NavigationDirection::up:
                    if(dy >= 0.0f) continue;
                    primary = -dy;
                    secondary = dx >= 0.0f ? dx : -dx;
                    break;
                case NavigationDirection::down:
                    if(dy <= 0.0f) continue;
                    primary = dy;
                    secondary = dx >= 0.0f ? dx : -dx;
                    break;
                default:
                    return false;
                }
                f32 score = primary * primary + secondary * secondary * 4.0f;
                if(score < best_score || (score == best_score && primary < best_primary))
                {
                    best_id = candidate.id;
                    best_score = score;
                    best_primary = primary;
                }
            }
            if(!best_id)
            {
                return false;
            }
            m_focused_element = best_id;
            return true;
        }

        static NavigationMode get_navigation_mode(const NavigationConfig& navigation, const NavigationRequest& request)
        {
            switch(request.event_type)
            {
            case InputEventType::navigation_dpad:
                switch(request.direction)
                {
                case NavigationDirection::left: return navigation.left;
                case NavigationDirection::right: return navigation.right;
                case NavigationDirection::up: return navigation.up;
                case NavigationDirection::down: return navigation.down;
                default: return NavigationMode::automatic;
                }
            case InputEventType::navigation_move:
                return request.move == NavigationMove::backward ? navigation.backward : navigation.forward;
            case InputEventType::navigation_confirm:
                return navigation.confirm;
            case InputEventType::navigation_back:
                return navigation.back;
            default:
                return NavigationMode::automatic;
            }
        }

        bool Context::navigate_default(const NavigationRequest& request)
        {
            lutsassert();
            switch(request.event_type)
            {
            case InputEventType::navigation_dpad:
                return move_focus_spatial(request.direction);
            case InputEventType::navigation_move:
                return move_focus(request.move == NavigationMove::backward);
            case InputEventType::navigation_confirm:
            {
                id_t target = request.source ? request.source : m_focused_element;
                deliver_input_event(target, request.event);
                const Element* element = find_element(target);
                if(element && element_can_focus(*element) &&
                    has_flags(element->interactable, InteractableFlag::activatable) &&
                    !has_flags(element->interactable, InteractableFlag::read_only))
                {
                    InteractionState& state = get_or_create_interaction(target);
                    state.clicked = true;
                    state.clicked_screen_position = element_screen_center(m_layers.cspan(), *element);
                    if(element->layer < m_layers.size())
                    {
                        state.clicked_element_position = element_local_position(
                            *element, m_layers[element->layer], state.clicked_screen_position);
                        state.clicked_element_rect = element->layout_result.rect;
                    }
                    return true;
                }
                return target != 0;
            }
            case InputEventType::navigation_back:
            {
                id_t target = request.source ? request.source : m_focused_element;
                deliver_input_event(target, request.event);
                return target != 0;
            }
            default:
                return false;
            }
        }

        bool Context::navigate(const NavigationRequest& request)
        {
            lutsassert();
            const Element* element = find_element(request.source);
            NavigationMode mode = element ? get_navigation_mode(element->navigation, request) : NavigationMode::automatic;
            switch(mode)
            {
            case NavigationMode::automatic:
                return navigate_default(request);
            case NavigationMode::none:
                return true;
            case NavigationMode::callback:
                return element && element->navigation.callback ?
                    element->navigation.callback(this, request, element->navigation.userdata) : false;
            default:
                return false;
            }
        }

        void Context::route_input()
        {
            lutsassert();
            u64 route_begin = get_ticks();
            m_input_deliveries.clear();
            m_routed_input_deliveries.clear();
            for(auto iter = m_interactions.begin(); iter != m_interactions.end();)
            {
                if(m_element_indices.find(iter->first) == m_element_indices.end())
                {
                    iter = m_interactions.erase(iter);
                }
                else
                {
                    iter->second.hovered = false;
                    iter->second.clicked = false;
                    iter->second.double_clicked = false;
                    iter->second.pointer_screen_position = Float2U(0.0f);
                    iter->second.pointer_element_position = Float2U(0.0f);
                    iter->second.pointer_element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    iter->second.clicked_screen_position = Float2U(0.0f);
                    iter->second.clicked_element_position = Float2U(0.0f);
                    iter->second.clicked_element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    iter->second.active = false;
                    iter->second.focused = false;
                    iter->second.subtree_hovered = false;
                    iter->second.subtree_active = false;
                    iter->second.subtree_focused = false;
                    iter->second.subtree_clicked = false;
                    iter->second.subtree_double_clicked = false;
                    ++iter;
                }
            }
            auto element_can_activate = [&](id_t id) -> bool
            {
                const Element* element = find_element(id);
                return element && has_flags(element->interactable, InteractableFlag::activatable) &&
                    !has_flags(element->interactable, InteractableFlag::disabled) &&
                    !has_flags(element->interactable, InteractableFlag::read_only);
            };
            if(m_pointer_capture_element && !element_can_activate(m_pointer_capture_element))
            {
                m_pointer_capture_element = 0;
            }
            for(auto iter = m_active_elements.begin(); iter != m_active_elements.end();)
            {
                if(!element_can_activate(*iter))
                {
                    iter = m_active_elements.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
            if(m_active_elements.empty())
            {
                if(element_can_activate(m_pointer_capture_element))
                {
                    m_active_elements.push_back(m_pointer_capture_element);
                }
                else
                {
                    m_pointer_capture_element = 0;
                }
            }
            if(m_element_indices.find(m_focused_element) == m_element_indices.end() ||
                (m_focused_element && !element_can_focus(*find_element(m_focused_element))))
            {
                m_focused_element = 0;
            }

            Vector<ElementHandle> pointer_targets;
            auto collect_pointer_targets = [&](Vector<ElementHandle>& targets)
            {
                targets.clear();
                if(!m_pointer_inside)
                {
                    return;
                }
                hit_test(m_pointer_position, [](const HitTestVisit& visit, void* userdata) {
                    if(visit.event_target)
                    {
                        Vector<ElementHandle>* targets = (Vector<ElementHandle>*)userdata;
                        targets->push_back(visit.element);
                    }
                }, &targets);
            };

            auto deliver_to_pointer_targets = [&](Span<const ElementHandle> targets, const InputEvent& event)
            {
                for(const ElementHandle& target : targets)
                {
                    deliver_input_event(target.id, event);
                }
            };

            auto update_hover = [&]()
            {
                m_hovered_elements.clear();
                if(!m_pointer_inside)
                {
                    return;
                }
                collect_pointer_targets(pointer_targets);
                for(const ElementHandle& hit : pointer_targets)
                {
                    const Element* element = get_element(hit.index);
                    if(element && has_flags(element->interactable, InteractableFlag::hoverable))
                    {
                        m_hovered_elements.push_back(hit.id);
                    }
                }
            };

            auto update_pointer_inside = [&]()
            {
                m_pointer_inside = m_pointer_position.x >= 0.0f && m_pointer_position.y >= 0.0f &&
                    m_pointer_position.x < m_frame_desc.screen_size.x && m_pointer_position.y < m_frame_desc.screen_size.y;
            };

            auto update_pointer_position = [&](const Float2U& position)
            {
                Float2U delta = position - m_pointer_position;
                m_pointer_delta.x += delta.x;
                m_pointer_delta.y += delta.y;
                m_pointer_position = position;
            };

            for(const InputEvent& event : m_input_events)
            {
                switch(event.type)
                {
                case InputEventType::pointer_enter:
                    m_key_modifiers = event.modifiers;
                    m_pointer_inside = true;
                    update_pointer_position(event.position);
                    update_hover();
                    break;
                case InputEventType::pointer_leave:
                    m_key_modifiers = event.modifiers;
                    m_pointer_inside = false;
                    update_pointer_position(event.position);
                    m_hovered_elements.clear();
                    break;
                case InputEventType::pointer_move:
                    m_key_modifiers = event.modifiers;
                    update_pointer_position(event.position);
                    update_pointer_inside();
                    update_hover();
                    if(!m_active_elements.empty())
                    {
                        for(id_t active : m_active_elements)
                        {
                            deliver_input_event(active, event);
                        }
                    }
                    else
                    {
                        collect_pointer_targets(pointer_targets);
                        deliver_to_pointer_targets(pointer_targets.cspan(), event);
                    }
                    break;
                case InputEventType::pointer_down:
                    m_key_modifiers = event.modifiers;
                    update_pointer_position(event.position);
                    update_pointer_inside();
                    update_hover();
                    if((u32)event.button < 5)
                    {
                        m_pointer_down[(u32)event.button] = true;
                    }
                    collect_pointer_targets(pointer_targets);
                    deliver_to_pointer_targets(pointer_targets.cspan(), event);
                    if(event.button == PointerButton::left)
                    {
                        m_active_elements.clear();
                        m_pointer_capture_element = 0;
                        bool focused_set = false;
                        for(const ElementHandle& hit : pointer_targets)
                        {
                            const Element* element = get_element(hit.index);
                            if(!element)
                            {
                                continue;
                            }
                            if(has_flags(element->interactable, InteractableFlag::activatable) && !has_flags(element->interactable, InteractableFlag::read_only))
                            {
                                if(!m_pointer_capture_element)
                                {
                                    m_pointer_capture_element = hit.id;
                                }
                                m_active_elements.push_back(hit.id);
                            }
                            if(!focused_set && has_flags(element->interactable, InteractableFlag::focusable))
                            {
                                m_focused_element = hit.id;
                                focused_set = true;
                            }
                        }
                        if(!focused_set)
                        {
                            m_focused_element = 0;
                        }
                    }
                    break;
                case InputEventType::pointer_up:
                    m_key_modifiers = event.modifiers;
                    update_pointer_position(event.position);
                    update_pointer_inside();
                    update_hover();
                    if((u32)event.button < 5)
                    {
                        m_pointer_down[(u32)event.button] = false;
                    }
                    if(event.button == PointerButton::left && m_drag_drop.active)
                    {
                        ElementHandle target = hit_test_drag_drop_target(m_drag_drop.type, m_pointer_position);
                        deliver_input_event(target.id, event);
                        deliver_drag_drop_payload(target);
                        clear_drag_drop();
                        m_pointer_capture_element = 0;
                        m_active_elements.clear();
                        break;
                    }
                    if(event.button == PointerButton::left && !m_active_elements.empty())
                    {
                        collect_pointer_targets(pointer_targets);
                        Vector<id_t> pointer_target_ids;
                        for(const ElementHandle& target : pointer_targets)
                        {
                            pointer_target_ids.push_back(target.id);
                        }
                        id_t clicked_representative = 0;
                        id_t previous_clicked_element = m_last_clicked_element;
                        f32 previous_clicked_time = m_last_clicked_time;
                        Float2U previous_clicked_position = m_last_clicked_position;
                        for(id_t active : m_active_elements)
                        {
                            const Element* active_element = find_element(active);
                            deliver_input_event(active, event);
                            if(contains_id(pointer_target_ids.cspan(), active) && active_element && element_can_activate(active))
                            {
                                InteractionState& state = get_or_create_interaction(active);
                                state.clicked = true;
                                f32 click_dx = m_pointer_position.x - previous_clicked_position.x;
                                f32 click_dy = m_pointer_position.y - previous_clicked_position.y;
                                constexpr f32 DOUBLE_CLICK_MAX_INTERVAL = 0.35f;
                                constexpr f32 DOUBLE_CLICK_MAX_DISTANCE = 6.0f;
                                state.double_clicked =
                                    previous_clicked_element == active &&
                                    (m_elapsed_time - previous_clicked_time) <= DOUBLE_CLICK_MAX_INTERVAL &&
                                    (click_dx * click_dx + click_dy * click_dy) <= DOUBLE_CLICK_MAX_DISTANCE * DOUBLE_CLICK_MAX_DISTANCE;
                                state.clicked_screen_position = m_pointer_position;
                                if(active_element->layer < m_layers.size())
                                {
                                    state.clicked_element_position = element_local_position(*active_element, m_layers[active_element->layer], m_pointer_position);
                                    state.clicked_element_rect = active_element->layout_result.rect;
                                }
                                if(!clicked_representative)
                                {
                                    clicked_representative = active;
                                }
                            }
                        }
                        if(clicked_representative)
                        {
                            m_last_clicked_element = clicked_representative;
                            m_last_clicked_time = m_elapsed_time;
                            m_last_clicked_position = m_pointer_position;
                        }
                        m_pointer_capture_element = 0;
                        m_active_elements.clear();
                    }
                    else
                    {
                        collect_pointer_targets(pointer_targets);
                        deliver_to_pointer_targets(pointer_targets.cspan(), event);
                    }
                    break;
                case InputEventType::pointer_wheel:
                {
                    m_key_modifiers = event.modifiers;
                    update_pointer_position(event.position);
                    update_pointer_inside();
                    update_hover();
                    collect_pointer_targets(pointer_targets);
                    id_t scroll_target = 0;
                    for(const ElementHandle& target : pointer_targets)
                    {
                        scroll_target = scroll_target_of(target.id);
                        if(scroll_target)
                        {
                            break;
                        }
                    }
                    if(scroll_target)
                    {
                        deliver_input_event(scroll_target, event);
                    }
                    else
                    {
                        deliver_to_pointer_targets(pointer_targets.cspan(), event);
                    }
                    break;
                }
                case InputEventType::key_down:
                {
                    m_key_modifiers = event.modifiers;
                    u32 key_index = (u32)event.key;
                    if(key_index < 256)
                    {
                        m_key_down[key_index] = true;
                    }
                    deliver_input_event(m_focused_element, event);
                    break;
                }
                case InputEventType::key_up:
                {
                    m_key_modifiers = event.modifiers;
                    u32 key_index = (u32)event.key;
                    if(key_index < 256)
                    {
                        m_key_down[key_index] = false;
                    }
                    deliver_input_event(m_focused_element, event);
                    break;
                }
                case InputEventType::text_utf8:
                    deliver_input_event(m_focused_element, event);
                    break;
                case InputEventType::navigation_dpad:
                {
                    m_key_modifiers = event.modifiers;
                    NavigationRequest request;
                    request.source = m_focused_element;
                    request.event_type = event.type;
                    request.direction = event.navigation_direction;
                    request.move = event.navigation_move;
                    request.event = event;
                    navigate(request);
                    break;
                }
                case InputEventType::navigation_move:
                {
                    m_key_modifiers = event.modifiers;
                    NavigationRequest request;
                    request.source = m_focused_element;
                    request.event_type = event.type;
                    request.direction = event.navigation_direction;
                    request.move = event.navigation_move;
                    request.event = event;
                    navigate(request);
                    break;
                }
                case InputEventType::navigation_confirm:
                {
                    m_key_modifiers = event.modifiers;
                    NavigationRequest request;
                    request.source = m_focused_element;
                    request.event_type = event.type;
                    request.direction = event.navigation_direction;
                    request.move = event.navigation_move;
                    request.event = event;
                    navigate(request);
                    break;
                }
                case InputEventType::navigation_back:
                {
                    m_key_modifiers = event.modifiers;
                    NavigationRequest request;
                    request.source = m_focused_element;
                    request.event_type = event.type;
                    request.direction = event.navigation_direction;
                    request.move = event.navigation_move;
                    request.event = event;
                    navigate(request);
                    break;
                }
                case InputEventType::focus:
                    break;
                case InputEventType::blur:
                    m_pointer_inside = false;
                    m_hovered_elements.clear();
                    m_pointer_capture_element = 0;
                    m_active_elements.clear();
                    m_focused_element = 0;
                    m_last_clicked_element = 0;
                    m_last_clicked_time = -1000.0f;
                    m_key_modifiers = KeyModifierFlag::none;
                    for(bool& key : m_key_down)
                    {
                        key = false;
                    }
                    for(bool& button : m_pointer_down)
                    {
                        button = false;
                    }
                    clear_drag_drop();
                    break;
                default:
                    break;
                }
            }

            update_hover();
            for(id_t hovered : m_hovered_elements)
            {
                get_or_create_interaction(hovered).hovered = true;
            }
            for(id_t active : m_active_elements)
            {
                get_or_create_interaction(active).active = true;
            }
            if(m_focused_element)
            {
                get_or_create_interaction(m_focused_element).focused = true;
            }

            Vector<id_t> hovered_ids;
            Vector<id_t> active_ids;
            Vector<id_t> focused_ids;
            Vector<id_t> clicked_ids;
            Vector<id_t> double_clicked_ids;
            for(auto& pair : m_interactions)
            {
                if(pair.second.hovered)
                {
                    hovered_ids.push_back(pair.first);
                }
                if(pair.second.active)
                {
                    active_ids.push_back(pair.first);
                }
                if(pair.second.focused)
                {
                    focused_ids.push_back(pair.first);
                }
                if(pair.second.clicked)
                {
                    clicked_ids.push_back(pair.first);
                }
                if(pair.second.double_clicked)
                {
                    double_clicked_ids.push_back(pair.first);
                }
            }
            for(id_t id : hovered_ids)
            {
                mark_subtree_interaction(id, true, false, false, false, false);
            }
            for(id_t id : active_ids)
            {
                mark_subtree_interaction(id, false, true, false, false, false);
            }
            for(id_t id : focused_ids)
            {
                mark_subtree_interaction(id, false, false, true, false, false);
            }
            for(id_t id : clicked_ids)
            {
                mark_subtree_interaction(id, false, false, false, true, false);
            }
            for(id_t id : double_clicked_ids)
            {
                mark_subtree_interaction(id, false, false, false, false, true);
            }
            m_counters.input_route_ms = perf_elapsed_ms(route_begin, get_ticks());
            log_debug_pass(DebugPassKind::input, Name("route_input"),
                m_input_events.empty() ? Name("no_queued_input") : Name("queued_input_events"), 0, nullptr,
                m_counters.input_route_ms);
        }

        object_t Context::get_state(id_t id)
        {
            lutsassert();
            auto iter = m_states.find(id);
            return iter == m_states.end() ? nullptr : iter->second.data.get();
        }

        RV Context::set_state(id_t id, object_t data, StateLifetime lifetime)
        {
            lutsassert();
            StateRecord record;
            record.data = data;
            record.lifetime = lifetime;
            record.last_touched_generation = m_generation;
            auto iter = m_states.find(id);
            if(iter == m_states.end())
            {
                m_states.insert(make_pair(id, move(record)));
            }
            else
            {
                iter->second = move(record);
            }
            return ok;
        }

        void Context::clear_state(id_t id)
        {
            lutsassert();
            m_states.erase(id);
        }

        Style& Context::get_or_create_style(const Name& name)
        {
            auto iter = m_styles.find(name);
            if(iter == m_styles.end())
            {
                Style style;
                iter = m_styles.insert(make_pair(name, move(style))).first;
            }
            return iter->second;
        }

        void Context::define_style(const Name& name, const Name& parent)
        {
            lutsassert();
            Style& style = get_or_create_style(name);
            if(!parent.empty())
            {
                style.parent = parent;
            }
        }

        void Context::set_style_parent(const Name& name, const Name& parent)
        {
            lutsassert();
            get_or_create_style(name).parent = parent;
        }

        void Context::set_style_value(const Name& style_name, const Name& entry, const StyleValue& value)
        {
            lutsassert();
            StyleEntry style_entry;
            style_entry.mode = StyleEntryMode::set;
            style_entry.value = value;
            Style& style = get_or_create_style(style_name);
            auto iter = style.entries.find(entry);
            if(iter == style.entries.end())
            {
                style.entries.insert(make_pair(entry, move(style_entry)));
            }
            else
            {
                iter->second = move(style_entry);
            }
        }

        void Context::inherit_style_entry(const Name& style_name, const Name& entry)
        {
            lutsassert();
            Style& style = get_or_create_style(style_name);
            style.entries.erase(entry);
        }

        void Context::unset_style_entry(const Name& style_name, const Name& entry)
        {
            lutsassert();
            StyleEntry style_entry;
            style_entry.mode = StyleEntryMode::unset;
            Style& style = get_or_create_style(style_name);
            auto iter = style.entries.find(entry);
            if(iter == style.entries.end())
            {
                style.entries.insert(make_pair(entry, move(style_entry)));
            }
            else
            {
                iter->second = move(style_entry);
            }
        }

        void Context::push_style(const Name& style)
        {
            lutsassert();
            m_style_stack.push_back(style);
        }

        void Context::pop_style()
        {
            lutsassert();
            luassert(!m_style_stack.empty());
            m_style_stack.pop_back();
        }

        Name Context::current_style() const
        {
            lutsassert();
            return m_style_stack.empty() ? Name() : m_style_stack.back();
        }

        StyleValue Context::get_style_value(const Name& style_name, const Name& entry, const StyleValue& default_value)
        {
            lutsassert();
            Name current = style_name;
            u32 depth = 0;
            while(!current.empty() && depth < 64)
            {
                auto style_iter = m_styles.find(current);
                if(style_iter == m_styles.end())
                {
                    break;
                }
                auto entry_iter = style_iter->second.entries.find(entry);
                if(entry_iter != style_iter->second.entries.end())
                {
                    if(entry_iter->second.mode == StyleEntryMode::set)
                    {
                        return entry_iter->second.value;
                    }
                    if(entry_iter->second.mode == StyleEntryMode::unset)
                    {
                        return default_value;
                    }
                }
                current = style_iter->second.parent;
                ++depth;
            }
            return default_value;
        }

        void Context::register_style_entry_schema(const StyleEntrySchema& schema)
        {
            lutsassert();
            luassert(!schema.entry.empty());
            for(StyleEntrySchema& existing : m_style_schemas)
            {
                if(existing.owner == schema.owner && existing.entry == schema.entry)
                {
                    existing = schema;
                    return;
                }
            }
            m_style_schemas.push_back(schema);
        }

        Span<const StyleEntrySchema> Context::get_style_entry_schemas()
        {
            lutsassert();
            return Span<const StyleEntrySchema>(m_style_schemas.data(), m_style_schemas.size());
        }

        PerformanceCounters Context::get_performance_counters()
        {
            lutsassert();
            refresh_counters();
            return m_counters;
        }

        DebugInfo Context::dump_debug_info()
        {
            lutsassert();
            u64 debug_begin = get_ticks();
            refresh_counters();
            DebugInfo info;
            info.counters = m_counters;
            info.input_events = m_input_events;
            info.issues = m_debug_issues;
            info.passes = m_debug_passes;
            info.draw_commands = m_draw_commands;
            info.data_scope_stack = m_data_scope_stack;
            auto resolve_debug_style_value = [this](const Name& style_name, const Name& entry, const StyleValue& default_value, bool& defaulted) -> StyleValue
            {
                Name current = style_name;
                u32 depth = 0;
                while(!current.empty() && depth < 64)
                {
                    auto style_iter = m_styles.find(current);
                    if(style_iter == m_styles.end())
                    {
                        break;
                    }
                    auto entry_iter = style_iter->second.entries.find(entry);
                    if(entry_iter != style_iter->second.entries.end())
                    {
                        if(entry_iter->second.mode == StyleEntryMode::set)
                        {
                            defaulted = false;
                            return entry_iter->second.value;
                        }
                        if(entry_iter->second.mode == StyleEntryMode::unset)
                        {
                            break;
                        }
                    }
                    current = style_iter->second.parent;
                    ++depth;
                }
                defaulted = true;
                return default_value;
            };
            for(const Layer& layer : m_layers)
            {
                DebugLayerInfo layer_info;
                layer_info.id = layer.id;
                layer_info.root = layer.root;
                layer_info.first_draw_command = layer.first_draw_command;
                layer_info.draw_command_count = layer.draw_command_count;
                layer_info.draw_command_indices = layer.draw_command_indices;
                layer_info.screen_position = layer.screen_position;
                layer_info.debug_name = layer.debug_name;
                info.layers.push_back(move(layer_info));
            }
            for(u32 i = 0; i < m_elements.size(); ++i)
            {
                const Element& element = m_elements[i];
                DebugElementInfo element_info;
                element_info.id = element.id;
                element_info.index = i;
                element_info.layer = element.layer;
                element_info.parent = element.parent;
                element_info.first_child = element.first_child;
                element_info.last_child = element.last_child;
                element_info.next_sibling = element.next_sibling;
                element_info.prev_sibling = element.prev_sibling;
                element_info.depth = element.depth;
                element_info.style = element.style;
                element_info.debug_name = element.debug_name;
                element_info.layout = element.layout;
                element_info.rect = element.layout_result.rect;
                element_info.clip_rect = element.layout_result.clip_rect;
                element_info.content_size = element.layout_result.content_size;
                element_info.pointer_hit_behavior = element.interactable.pointer_hit_behavior;
                element_info.hit_test_mode = element.hit_test.mode;
                element_info.has_hit_test_callback = element.hit_test.callback != nullptr;
                element_info.hoverable = has_flags(element.interactable, InteractableFlag::hoverable);
                element_info.activatable = has_flags(element.interactable, InteractableFlag::activatable);
                element_info.focusable = has_flags(element.interactable, InteractableFlag::focusable);
                element_info.scrollable = has_flags(element.interactable, InteractableFlag::scrollable);
                element_info.disabled = has_flags(element.interactable, InteractableFlag::disabled);
                element_info.read_only = has_flags(element.interactable, InteractableFlag::read_only);
                element_info.focus_scope = element.interactable.focus_scope;
                element_info.drag_source_types = element.interactable.drag_source_types;
                element_info.drag_target_types = element.interactable.drag_target_types;
                for(const StyleEntrySchema& schema : m_style_schemas)
                {
                    DebugResolvedStyleEntryInfo resolved_style;
                    resolved_style.owner = schema.owner;
                    resolved_style.entry = schema.entry;
                    resolved_style.value = resolve_debug_style_value(element.style, schema.entry, schema.default_value, resolved_style.defaulted);
                    element_info.resolved_style.push_back(move(resolved_style));
                }
                InteractionState interaction = get_interaction_state(element.id);
                element_info.hovered = interaction.hovered;
                element_info.active = interaction.active;
                element_info.pointer_captured = element.id == m_pointer_capture_element;
                element_info.focused = interaction.focused;
                element_info.clicked = interaction.clicked;
                element_info.double_clicked = interaction.double_clicked;
                element_info.pointer_screen_position = interaction.pointer_screen_position;
                element_info.pointer_element_position = interaction.pointer_element_position;
                element_info.pointer_element_rect = interaction.pointer_element_rect;
                element_info.clicked_screen_position = interaction.clicked_screen_position;
                element_info.clicked_element_position = interaction.clicked_element_position;
                element_info.clicked_element_rect = interaction.clicked_element_rect;
                element_info.subtree_hovered = interaction.subtree_hovered;
                element_info.subtree_active = interaction.subtree_active;
                element_info.subtree_focused = interaction.subtree_focused;
                element_info.subtree_clicked = interaction.subtree_clicked;
                element_info.subtree_double_clicked = interaction.subtree_double_clicked;
                element_info.first_draw_command = element.first_draw_command;
                element_info.draw_command_count = element.draw_command_count;
                info.elements.push_back(move(element_info));
            }
            for(auto& pair : m_styles)
            {
                DebugStyleInfo style_info;
                style_info.name = pair.first;
                style_info.parent = pair.second.parent;
                style_info.entry_count = (u32)pair.second.entries.size();
                for(auto& entry_pair : pair.second.entries)
                {
                    DebugStyleEntryInfo entry_info;
                    entry_info.entry = entry_pair.first;
                    entry_info.mode = entry_pair.second.mode;
                    entry_info.value = entry_pair.second.value;
                    style_info.entries.push_back(move(entry_info));
                }
                info.styles.push_back(move(style_info));
            }
            info.style_schemas = m_style_schemas;
            for(auto& pair : m_input_deliveries)
            {
                DebugInputDeliveryInfo delivery_info;
                delivery_info.element_id = pair.first;
                delivery_info.events = pair.second;
                auto routed_iter = m_routed_input_deliveries.find(pair.first);
                if(routed_iter != m_routed_input_deliveries.end())
                {
                    delivery_info.routed_events = routed_iter->second;
                }
                info.input_deliveries.push_back(move(delivery_info));
            }
            info.hovered_elements = m_hovered_elements;
            info.active_elements = m_active_elements;
            info.pointer_capture_element = m_pointer_capture_element;
            info.focused_element = m_focused_element;
            info.focused_scope = focus_scope_of(m_focused_element);
            info.drag_drop_active = m_drag_drop.active;
            info.drag_drop_source = m_drag_drop.source_id;
            info.drag_drop_type = m_drag_drop.type;
            m_counters.debug_dump_ms = perf_elapsed_ms(debug_begin, get_ticks());
            info.counters = m_counters;
            return info;
        }

        void Context::log_debug_issue(DebugIssueSeverity severity, const Name& category, const c8* message, id_t element)
        {
            lutsassert();
            DebugIssueInfo issue;
            issue.severity = severity;
            issue.category = category;
            issue.element = element;
            issue.message = message ? message : "";
            m_debug_issues.push_back(move(issue));
            m_counters.debug_issue_count = (u32)m_debug_issues.size();
        }

        void Context::log_debug_pass(DebugPassKind kind, const Name& name, const Name& reason, id_t element,
            const c8* detail, f64 duration_ms)
        {
            lutsassert();
            DebugPassInfo pass;
            pass.kind = kind;
            pass.name = name;
            pass.reason = reason;
            pass.element = element;
            pass.frame_generation = m_generation;
            pass.duration_ms = duration_ms;
            pass.detail = detail ? detail : "";
            m_debug_passes.push_back(move(pass));
            m_counters.debug_pass_count = (u32)m_debug_passes.size();
        }

        void Context::gc_states()
        {
            for(auto iter = m_states.begin(); iter != m_states.end();)
            {
                bool erase = false;
                if(iter->second.lifetime == StateLifetime::current_frame)
                {
                    erase = true;
                }
                else if(iter->second.lifetime == StateLifetime::next_frame &&
                    (m_generation - iter->second.last_touched_generation) > 1)
                {
                    erase = true;
                }
                if(erase)
                {
                    iter = m_states.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }

        void Context::refresh_counters()
        {
            m_counters.frame_generation = m_generation;
            m_counters.layer_count = (u32)m_layers.size();
            m_counters.element_count = (u32)m_elements.size();
            m_counters.input_event_count = (u32)m_input_events.size();
            m_counters.delivered_input_event_count = 0;
            for(auto& pair : m_input_deliveries)
            {
                m_counters.delivered_input_event_count += (u32)pair.second.size();
            }
            m_counters.interactable_count = 0;
            for(const Element& element : m_elements)
            {
                if(element.interactable.pointer_hit_behavior != PointerHitBehavior::none)
                {
                    ++m_counters.interactable_count;
                }
            }
            m_counters.draw_command_count = (u32)m_draw_commands.size();
            m_counters.state_count = (u32)m_states.size();
            m_counters.style_count = (u32)m_styles.size();
            m_counters.style_schema_count = (u32)m_style_schemas.size();
            m_counters.debug_issue_count = (u32)m_debug_issues.size();
            m_counters.debug_pass_count = (u32)m_debug_passes.size();
        }
    }
}
