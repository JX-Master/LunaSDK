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

            RectF merge_clip_rect(const RectF& a, const RectF& b)
            {
                if(has_clip(a) && has_clip(b))
                {
                    return intersect_rect(a, b);
                }
                return has_clip(a) ? a : b;
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
            m_layout_callback_configs.clear();
            m_navigation_configs.clear();
            m_hit_test_configs.clear();
            m_draw_configs.clear();
            m_recorded_draw_commands.clear();
            m_layer_draw_operations.clear();
            m_draw_commands.clear();
            m_input_events.clear();
            m_pointer_delta = Float2U(0.0f);
            m_layer_stack.clear();
            m_element_stack.clear();
            m_style_stack.clear();
            m_data_scope_stack.clear();
            m_data_scope_stack.push_back(DEFAULT_DATA_SCOPE);
            m_element_indices.clear();
            m_text_input_request = TextInputRequest();
            m_hovered_elements.clear();
            m_draw_generation_layer = INVALID_LAYER;
            m_draw_generation_element = INVALID_ELEMENT;
            m_generating_draw_commands = false;
            m_draw_commands_generated = false;
            m_counters = PerformanceCounters();
            m_counters.frame_generation = m_generation;
            m_counters.state_gc_ms = perf_elapsed_ms(gc_begin, gc_end);
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

        Span<const InputEvent> Context::get_input_events() const
        {
            lutsassert();
            return m_input_events.cspan();
        }

        void Context::push_layer(id_t id, const Float2U& screen_position)
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "Layers cannot be built while GUI Core draw callbacks are running.");
            for(const Layer& layer : m_layers)
            {
                luassert_msg(layer.id != id, "Duplicate GUI Core layer ID in one frame.");
            }
            Layer layer;
            layer.id = id;
            layer.screen_position = screen_position;
            m_layers.push_back(move(layer));
            m_layer_draw_operations.push_back(Vector<DrawOperation>());
            m_layer_stack.push_back((u32)m_layers.size() - 1);
            m_draw_commands_generated = false;
        }

        void Context::pop_layer()
        {
            lutsassert();
            luassert(!m_layer_stack.empty());
            luassert(m_element_stack.empty() || m_elements[m_element_stack.back()].layer != m_layer_stack.back());
            m_layer_stack.pop_back();
        }

        Span<const Layer> Context::get_layers() const
        {
            lutsassert();
            return m_layers.cspan();
        }

        bool Context::set_layer_screen_position(id_t id, const Float2U& screen_position)
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "Layer positions cannot change while GUI Core draw callbacks are running.");
            for(Layer& layer : m_layers)
            {
                if(layer.id == id)
                {
                    layer.screen_position = screen_position;
                    m_draw_commands_generated = false;
                    return true;
                }
            }
            return false;
        }

        bool Context::bring_layer_to_front(id_t id)
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "Layers cannot be reordered while GUI Core draw callbacks are running.");
            u32 source = U32_MAX;
            for(u32 i = 0; i < m_layers.size(); ++i)
            {
                if(m_layers[i].id == id)
                {
                    source = i;
                    break;
                }
            }
            if(source == U32_MAX)
            {
                return false;
            }
            if(source + 1 == m_layers.size())
            {
                return true;
            }

            Layer layer = move(m_layers[source]);
            Vector<DrawOperation> operations = move(m_layer_draw_operations[source]);
            m_layers.erase(m_layers.begin() + source);
            m_layer_draw_operations.erase(m_layer_draw_operations.begin() + source);
            m_layers.push_back(move(layer));
            m_layer_draw_operations.push_back(move(operations));
            u32 destination = (u32)m_layers.size() - 1;

            for(Element& element : m_elements)
            {
                if(element.layer == source) element.layer = destination;
                else if(element.layer > source) --element.layer;
            }
            for(DrawCommand& command : m_recorded_draw_commands)
            {
                if(command.layer == source) command.layer = destination;
                else if(command.layer > source) --command.layer;
            }
            for(u32& layer_index : m_layer_stack)
            {
                if(layer_index == source) layer_index = destination;
                else if(layer_index > source) --layer_index;
            }
            m_draw_commands_generated = false;
            return true;
        }

        void Context::set_layer_debug_name(id_t id, const Name& name)
        {
            lutsassert();
            for(Layer& layer : m_layers)
            {
                if(layer.id == id)
                {
                    layer.debug_name = name;
                    return;
                }
            }
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

        Span<const id_t> Context::get_data_scope_stack() const
        {
            lutsassert();
            return m_data_scope_stack.cspan();
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

        ElementHandle Context::begin_element(id_t id)
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "Elements cannot be built while GUI Core draw callbacks are running.");
            luassert(!m_layer_stack.empty());
            luassert_msg(m_element_indices.find(id) == m_element_indices.end(), "Duplicate GUI Core element ID in one frame.");
            u32 layer_index = m_layer_stack.back();
            u32 index = (u32)m_elements.size();
            Element element;
            element.id = id;
            element.layer = layer_index;
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
            m_layer_draw_operations[layer_index].push_back(DrawOperation { DrawOperationType::begin_element, index });
            m_draw_commands_generated = false;
            return ElementHandle { id, index, m_generation };
        }

        void Context::end_element()
        {
            lutsassert();
            luassert(!m_element_stack.empty());
            luassert_msg(!m_generating_draw_commands, "Elements cannot be built while GUI Core draw callbacks are running.");
            u32 element_index = m_element_stack.back();
            u32 layer_index = m_elements[element_index].layer;
            m_layer_draw_operations[layer_index].push_back(DrawOperation { DrawOperationType::end_element, element_index });
            m_element_stack.pop_back();
            m_draw_commands_generated = false;
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
                m_draw_commands_generated = false;
            }
        }

        void Context::set_layout_callback_config(const ElementHandle& element, const LayoutCallbackConfig& config)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e)
            {
                return;
            }
            if(e->layout_callback_config == U32_MAX)
            {
                e->layout_callback_config = (u32)m_layout_callback_configs.size();
                m_layout_callback_configs.push_back(config);
            }
            else
            {
                m_layout_callback_configs[e->layout_callback_config] = config;
            }
            m_draw_commands_generated = false;
        }

        LayoutCallbackConfig Context::get_layout_callback_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return LayoutCallbackConfig();
            }
            const Element& e = m_elements[element.index];
            if(e.id != element.id || e.layout_callback_config >= m_layout_callback_configs.size())
            {
                return LayoutCallbackConfig();
            }
            return m_layout_callback_configs[e.layout_callback_config];
        }

        void Context::set_layout_result(const ElementHandle& element, const LayoutResult& result)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->layout_result = result;
                m_draw_commands_generated = false;
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

        MeasureResult Context::measure_element(const ElementHandle& element, const Float2U& available_size)
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size() ||
                m_elements[element.index].id != element.id)
            {
                return MeasureResult();
            }
            const Element& e = m_elements[element.index];
            LayoutCallbackConfig callbacks;
            if(e.layout_callback_config < m_layout_callback_configs.size())
            {
                callbacks = m_layout_callback_configs[e.layout_callback_config];
            }
            MeasureResult content;
            if(callbacks.measure_callback)
            {
                Float2U available_content(
                    max(available_size.x - e.layout.margin.x - e.layout.margin.z - e.layout.padding.x - e.layout.padding.z, 0.0f),
                    max(available_size.y - e.layout.margin.y - e.layout.margin.w - e.layout.padding.y - e.layout.padding.w, 0.0f));
                content = callbacks.measure_callback(this, element, available_content, callbacks.userdata);
            }
            MeasureResult result;
            for(LayoutAxis axis : { LayoutAxis::x, LayoutAxis::y })
            {
                const SizeValue& size = axis == LayoutAxis::x ? e.layout.width : e.layout.height;
                f32 available_axis = axis == LayoutAxis::x ? available_size.x : available_size.y;
                f32 padding = axis == LayoutAxis::x ?
                    e.layout.padding.x + e.layout.padding.z :
                    e.layout.padding.y + e.layout.padding.w;
                f32 content_min = (axis == LayoutAxis::x ? content.minimum.x : content.minimum.y) + padding;
                f32 content_desired = (axis == LayoutAxis::x ? content.desired.x : content.desired.y) + padding;
                f32 content_max = axis == LayoutAxis::x ? content.maximum.x : content.maximum.y;
                content_max = content_max >= F32_MAX * 0.5f ? F32_MAX : content_max + padding;
                f32 minimum = size.kind == SizeKind::fit ? max(size.min, content_min) : size.min;
                f32 maximum = size.max >= 0.0f ? size.max : F32_MAX;
                if(size.kind == SizeKind::fit)
                {
                    maximum = min(maximum, content_max);
                }
                maximum = max(maximum, minimum);
                f32 desired = 0.0f;
                switch(size.kind)
                {
                case SizeKind::fixed:
                    desired = size.value;
                    break;
                case SizeKind::percent:
                    desired = available_axis * size.value;
                    break;
                case SizeKind::fit:
                    desired = content_desired;
                    break;
                default:
                    break;
                }
                desired = min(max(desired, minimum), maximum);
                if(axis == LayoutAxis::x)
                {
                    result.minimum.x = minimum;
                    result.desired.x = desired;
                    result.maximum.x = maximum;
                }
                else
                {
                    result.minimum.y = minimum;
                    result.desired.y = desired;
                    result.maximum.y = maximum;
                }
            }
            return result;
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
            LayoutCallbackConfig config = get_layout_callback_config(element);
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
            LayoutCallbackConfig config = get_layout_callback_config(element);
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
                m_draw_commands_generated = false;
            }
        }

        void Context::set_navigation_config(const ElementHandle& element, const NavigationConfig& navigation)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e)
            {
                return;
            }
            if(e->navigation_config == U32_MAX)
            {
                e->navigation_config = (u32)m_navigation_configs.size();
                m_navigation_configs.push_back(navigation);
            }
            else
            {
                m_navigation_configs[e->navigation_config] = navigation;
            }
            m_draw_commands_generated = false;
        }

        NavigationConfig Context::get_navigation_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return NavigationConfig();
            }
            const Element& e = m_elements[element.index];
            if(e.id != element.id || e.navigation_config >= m_navigation_configs.size())
            {
                return NavigationConfig();
            }
            return m_navigation_configs[e.navigation_config];
        }

        void Context::set_hit_test_config(const ElementHandle& element, const ElementHitTestConfig& hit_test)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e)
            {
                return;
            }
            if(e->hit_test_config == U32_MAX)
            {
                e->hit_test_config = (u32)m_hit_test_configs.size();
                m_hit_test_configs.push_back(hit_test);
            }
            else
            {
                m_hit_test_configs[e->hit_test_config] = hit_test;
            }
            m_draw_commands_generated = false;
        }

        ElementHitTestConfig Context::get_hit_test_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return ElementHitTestConfig();
            }
            const Element& e = m_elements[element.index];
            if(e.id != element.id || e.hit_test_config >= m_hit_test_configs.size())
            {
                return ElementHitTestConfig();
            }
            return m_hit_test_configs[e.hit_test_config];
        }

        void Context::bind_style(const ElementHandle& element, const Name& style)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->style = style;
                m_draw_commands_generated = false;
            }
        }

        const Element* Context::get_element(u32 index) const
        {
            lutsassert();
            return index < m_elements.size() ? &m_elements[index] : nullptr;
        }

        Span<const Element> Context::get_elements() const
        {
            lutsassert();
            return m_elements.cspan();
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

        void Context::set_element_debug_name(const ElementHandle& element, const Name& name)
        {
            lutsassert();
            if(Element* e = mutable_element(element))
            {
                e->debug_name = name;
            }
        }

        void Context::set_draw_config(const ElementHandle& element, const DrawConfig& config)
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "Draw configuration cannot change while callbacks are running.");
            Element* e = mutable_element(element);
            if(!e)
            {
                return;
            }
            if(e->draw_config == U32_MAX)
            {
                e->draw_config = (u32)m_draw_configs.size();
                m_draw_configs.push_back(config);
            }
            else
            {
                m_draw_configs[e->draw_config] = config;
            }
            m_draw_commands_generated = false;
        }

        DrawConfig Context::get_draw_config(const ElementHandle& element) const
        {
            lutsassert();
            if(!element.id || element.generation != m_generation || element.index >= m_elements.size())
            {
                return DrawConfig();
            }
            const Element& e = m_elements[element.index];
            if(e.id != element.id || e.draw_config >= m_draw_configs.size())
            {
                return DrawConfig();
            }
            return m_draw_configs[e.draw_config];
        }

        Span<const DrawCommand> Context::get_draw_commands() const
        {
            lutsassert();
            return m_draw_commands.cspan();
        }

        void Context::append_draw_command(u32 layer_index, u32 element_index, const DrawCommand& command)
        {
            DrawCommand cmd = command;
            cmd.layer = layer_index;
            cmd.element = element_index;
            u32 command_index = (u32)m_draw_commands.size();
            Layer& layer = m_layers[layer_index];
            layer.draw_command_indices.push_back(command_index);
            m_draw_commands.push_back(move(cmd));
        }

        void Context::record_static_draw_command(u32 layer_index, u32 element_index, const DrawCommand& command)
        {
            DrawCommand recorded = command;
            recorded.layer = layer_index;
            recorded.element = element_index;
            u32 command_index = (u32)m_recorded_draw_commands.size();
            m_recorded_draw_commands.push_back(move(recorded));
            m_layer_draw_operations[layer_index].push_back(DrawOperation { DrawOperationType::static_command, command_index });
            m_draw_commands_generated = false;
        }

        void Context::reset_generated_draw_commands()
        {
            m_draw_commands.clear();
            for(Layer& layer : m_layers)
            {
                layer.draw_command_indices.clear();
            }
        }

        RV Context::invoke_draw_callback(u32 layer_index, u32 element_index, DrawPhase phase)
        {
            if(element_index >= m_elements.size())
            {
                return ok;
            }
            const Element& element = m_elements[element_index];
            if(element.draw_config >= m_draw_configs.size())
            {
                return ok;
            }
            const DrawConfig& config = m_draw_configs[element.draw_config];
            DrawPhaseFlag required_phase = phase == DrawPhase::before_children ?
                DrawPhaseFlag::before_children : DrawPhaseFlag::after_children;
            if(!config.callback || !test_flags(config.phases, required_phase))
            {
                return ok;
            }
            m_draw_generation_layer = layer_index;
            m_draw_generation_element = element_index;
            ++m_counters.draw_callback_count;
            return config.callback(this, ElementHandle { element.id, element_index, m_generation }, phase, config.userdata);
        }

        RV Context::generate_draw_commands()
        {
            lutsassert();
            luassert_msg(!m_generating_draw_commands, "GUI Core draw command generation is not reentrant.");
            luassert_msg(m_element_stack.empty() && m_layer_stack.empty(),
                "GUI Core draw commands can only be generated after all element and layer scopes are closed.");
            u64 generate_begin = get_ticks();
            reset_generated_draw_commands();
            m_counters.draw_callback_count = 0;
            m_generating_draw_commands = true;
            m_draw_commands_generated = false;
            for(u32 layer_index = 0; layer_index < m_layer_draw_operations.size(); ++layer_index)
            {
                for(const DrawOperation& operation : m_layer_draw_operations[layer_index])
                {
                    RV result = ok;
                    switch(operation.type)
                    {
                    case DrawOperationType::begin_element:
                        result = invoke_draw_callback(layer_index, operation.index, DrawPhase::before_children);
                        break;
                    case DrawOperationType::static_command:
                        if(operation.index < m_recorded_draw_commands.size())
                        {
                            const DrawCommand& command = m_recorded_draw_commands[operation.index];
                            append_draw_command(command.layer, command.element, command);
                        }
                        break;
                    case DrawOperationType::end_element:
                        result = invoke_draw_callback(layer_index, operation.index, DrawPhase::after_children);
                        break;
                    }
                    if(failed(result))
                    {
                        m_generating_draw_commands = false;
                        m_draw_generation_layer = INVALID_LAYER;
                        m_draw_generation_element = INVALID_ELEMENT;
                        m_counters.draw_generate_ms = perf_elapsed_ms(generate_begin, get_ticks());
                        return result;
                    }
                }
            }
            m_generating_draw_commands = false;
            m_draw_generation_layer = INVALID_LAYER;
            m_draw_generation_element = INVALID_ELEMENT;
            m_draw_commands_generated = true;
            m_counters.draw_generate_ms = perf_elapsed_ms(generate_begin, get_ticks());
            return ok;
        }

        void Context::draw(const DrawCommand& command)
        {
            lutsassert();
            if(m_generating_draw_commands)
            {
                luassert(m_draw_generation_layer < m_layers.size());
                append_draw_command(m_draw_generation_layer, m_draw_generation_element, command);
                return;
            }
            luassert(!m_layer_stack.empty());
            u32 layer_index = m_layer_stack.back();
            u32 element_index = m_element_stack.empty() ? INVALID_ELEMENT : m_element_stack.back();
            record_static_draw_command(layer_index, element_index, command);
        }

        void Context::draw_for_element(const ElementHandle& element, const DrawCommand& command)
        {
            lutsassert();
            Element* e = mutable_element(element);
            if(!e || e->layer >= m_layers.size())
            {
                return;
            }
            if(m_generating_draw_commands)
            {
                luassert_msg(e->layer == m_draw_generation_layer,
                    "A draw callback cannot emit commands into a different layer.");
                append_draw_command(e->layer, element.index, command);
                return;
            }
            record_static_draw_command(e->layer, element.index, command);
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
            if(!m_draw_commands_generated)
            {
                RV result = generate_draw_commands();
                if(failed(result))
                {
                    return result;
                }
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
                if(layer.draw_command_indices.empty())
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
                    RectF element_clip;
                    if(command.element != INVALID_ELEMENT && command.element < m_elements.size())
                    {
                        element_clip = to_screen_rect(layer_index, m_elements[command.element].layout_result.clip_rect);
                    }
                    if(command.type == DrawCommandType::push_clip)
                    {
                        RectF screen_clip = to_screen_rect(layer_index, resolved_rect);
                        screen_clip = merge_clip_rect(screen_clip, element_clip);
                        screen_clip = clip_stack.empty() ? screen_clip : intersect_rect(clip_stack.back(), screen_clip);
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

                    RectF clip_rect = clip_stack.empty() ? element_clip : merge_clip_rect(clip_stack.back(), element_clip);
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
                            u32 indices[6];
                            VG::get_rect_shape_draw_vertices(vertices, indices, begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y), Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f, 0.0f), Float2U(vg_rect.width, vg_rect.height),
                                command.color, command.min_texcoord, command.max_texcoord);
                            vertices[0].color = command.color_bottom_left;
                            vertices[1].color = command.color;
                            vertices[2].color = command.color_top_right;
                            vertices[3].color = command.color_bottom_right;
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
            const ElementHitTestConfig* hit_test = element.hit_test_config < m_hit_test_configs.size() ?
                &m_hit_test_configs[element.hit_test_config] : nullptr;
            if(hit_test && hit_test->mode == ElementHitTestMode::callback)
            {
                if(!hit_test->callback)
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
                return hit_test->callback(this, request, hit_test->userdata);
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
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
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
            NavigationConfig navigation;
            if(element && element->navigation_config < m_navigation_configs.size())
            {
                navigation = m_navigation_configs[element->navigation_config];
            }
            NavigationMode mode = get_navigation_mode(navigation, request);
            switch(mode)
            {
            case NavigationMode::automatic:
                return navigate_default(request);
            case NavigationMode::none:
                return true;
            case NavigationMode::callback:
                return element && navigation.callback ?
                    navigation.callback(this, request, navigation.userdata) : false;
            default:
                return false;
            }
        }

        void Context::route_input()
        {
            lutsassert();
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
            Style& style = get_or_create_style(name);
            if(!parent.empty())
            {
                style.parent = parent;
            }
        }

        void Context::set_style_parent(const Name& name, const Name& parent)
        {
            lutsassert();
            m_draw_commands_generated = false;
            get_or_create_style(name).parent = parent;
        }

        void Context::set_style_value(const Name& style_name, const Name& entry, const StyleValue& value)
        {
            lutsassert();
            m_draw_commands_generated = false;
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
            m_draw_commands_generated = false;
            Style& style = get_or_create_style(style_name);
            style.entries.erase(entry);
        }

        void Context::unset_style_entry(const Name& style_name, const Name& entry)
        {
            lutsassert();
            m_draw_commands_generated = false;
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

        const Style* Context::get_style(const Name& name) const
        {
            lutsassert();
            auto iter = m_styles.find(name);
            return iter == m_styles.end() ? nullptr : &iter->second;
        }

        const HashMap<Name, Style>& Context::get_styles() const
        {
            lutsassert();
            return m_styles;
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
        }
    }
}
