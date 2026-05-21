/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        static GUIContext* g_current_context = nullptr;

        static constexpr u64 FNV_OFFSET = 14695981039346656037ull;
        static constexpr u64 FNV_PRIME = 1099511628211ull;

        static u64 hash_bytes(const void* data, usize size, u64 h = FNV_OFFSET)
        {
            const byte_t* p = (const byte_t*)data;
            for(usize i = 0; i < size; ++i)
            {
                h ^= (u64)p[i];
                h *= FNV_PRIME;
            }
            return h;
        }

        static u64 hash_cstr(const c8* str, u64 h)
        {
            if(!str) return h;
            while(*str)
            {
                h ^= (u64)(byte_t)*str;
                h *= FNV_PRIME;
                ++str;
            }
            return h;
        }

        static u64 hash_u64(u64 value, u64 h = FNV_OFFSET)
        {
            return hash_bytes(&value, sizeof(value), h);
        }

        static bool point_in_rect(const Float2U& p, const RectF& r)
        {
            return p.x >= r.offset_x && p.y >= r.offset_y &&
                p.x < r.offset_x + r.width && p.y < r.offset_y + r.height;
        }

        static RectF intersect_rect(const RectF& a, const RectF& b)
        {
            f32 min_x = max(a.offset_x, b.offset_x);
            f32 min_y = max(a.offset_y, b.offset_y);
            f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
            f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
            return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
        }

        static void pop_utf8_codepoint(String& value)
        {
            usize size = value.size();
            if(!size) return;
            usize begin = size - 1;
            while(begin > 0 && (((u8)value[begin]) & 0xC0) == 0x80)
            {
                --begin;
            }
            value.erase(begin, size - begin);
        }

        namespace GUIState
        {
            LUNA_GUI_API GUIStateKey<bool> clicked() { return {Name("gui.clicked"), false}; }
            LUNA_GUI_API GUIStateKey<bool> double_clicked() { return {Name("gui.double_clicked"), false}; }
            LUNA_GUI_API GUIStateKey<bool> hovered() { return {Name("gui.hovered"), false}; }
            LUNA_GUI_API GUIStateKey<bool> active() { return {Name("gui.active"), false}; }
            LUNA_GUI_API GUIStateKey<bool> focused() { return {Name("gui.focused"), false}; }
            LUNA_GUI_API GUIStateKey<bool> open() { return {Name("gui.open"), false}; }
            LUNA_GUI_API GUIStateKey<bool> value_changed() { return {Name("gui.value_changed"), false}; }
            LUNA_GUI_API GUIStateKey<RectF> rect() { return {Name("gui.rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
            LUNA_GUI_API GUIStateKey<RectF> clip_rect() { return {Name("gui.clip_rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
        }

        GUIContext::GUIContext()
        {
            m_shape_draw_list = VG::new_shape_draw_list(m_device);
            m_shape_renderer = VG::new_fill_shape_renderer();
            m_font_atlas = VG::new_font_atlas();
        }

        void GUIContext::begin_frame(const GUIFrameDesc& desc)
        {
            lutsassert();
            m_time += desc.delta_time;
            m_frame_desc = desc;
            m_submitted = false;
            m_last_results = m_current_results;
            m_current_results.clear();
            ++m_generation;
            m_build_desc = GUIDescription();
            m_build_desc.generation = m_generation;
            m_parent_stack.clear();
            m_id_stack.clear();
            m_child_ordinals.clear();

            GUINode root;
            root.id = 1;
            root.kind = GUINodeKind::root;
            root.parent = U32_MAX;
            root.depth = 0;
            root.requested_size = GUISize::fixed(desc.surface_size.x, desc.surface_size.y);
            m_build_desc.nodes.push_back(root);
            m_child_ordinals.push_back(0);
            m_parent_stack.push_back(0);
            m_id_stack.push_back(root.id);
            set_current_context(this);
        }

        void GUIContext::add_input_event(const GUIInputEvent& event)
        {
            lutsassert();
            m_input_events.push_back(event);
        }

        void GUIContext::add_input_events(Span<const GUIInputEvent> events)
        {
            lutsassert();
            m_input_events.insert(m_input_events.end(), events.begin(), events.end());
        }

        R<GUIDescription> GUIContext::end_build()
        {
            lutsassert();
            return m_build_desc;
        }

        GUIItemHandle GUIContext::add_node(GUINodeKind kind, const c8* text, bool interactive)
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            u32 ordinal = m_child_ordinals[parent]++;
            u64 h = hash_u64(m_build_desc.nodes[parent].id);
            h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back(), h);
            h = hash_u64((u64)kind, h);
            h = hash_u64((u64)ordinal, h);
            h = hash_cstr(text, h);

            GUINode node;
            node.id = h ? h : 1;
            node.kind = kind;
            node.parent = parent;
            node.depth = m_build_desc.nodes[parent].depth + 1;
            node.text = text ? text : "";
            node.interactive = interactive;

            u32 index = (u32)m_build_desc.nodes.size();
            m_build_desc.nodes.push_back(node);
            m_child_ordinals.push_back(0);

            GUINode& parent_node = m_build_desc.nodes[parent];
            if(parent_node.first_child == U32_MAX)
            {
                parent_node.first_child = index;
            }
            else
            {
                m_build_desc.nodes[parent_node.last_child].next_sibling = index;
            }
            parent_node.last_child = index;

            return GUIItemHandle{get_object(), node.id, m_generation};
        }

        void GUIContext::begin_container(GUINodeKind kind, const c8* label, const GUISize& size, GUIItemHandle* out_handle)
        {
            bool interactive = kind == GUINodeKind::scroll_view;
            GUIItemHandle handle = add_node(kind, label, interactive);
            u32 index = (u32)m_build_desc.nodes.size() - 1;
            m_build_desc.nodes[index].requested_size = size;
            m_parent_stack.push_back(index);
            m_id_stack.push_back(handle.id);
            if(out_handle) *out_handle = handle;
        }

        void GUIContext::end_container()
        {
            lutsassert();
            luassert(m_parent_stack.size() > 1);
            m_parent_stack.pop_back();
            m_id_stack.pop_back();
        }

        void GUIContext::push_id(GUIID id)
        {
            lutsassert();
            u64 h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back());
            h = hash_u64(id, h);
            m_id_stack.push_back(h);
        }

        void GUIContext::pop_id()
        {
            lutsassert();
            luassert(m_id_stack.size() > 1);
            m_id_stack.pop_back();
        }

        ItemResult* GUIContext::get_query_result(GUIItemHandle handle)
        {
            if(handle.context != get_object()) return nullptr;
            if(m_submitted)
            {
                if(handle.generation != m_generation) return nullptr;
                auto iter = m_current_results.find(handle.id);
                return iter == m_current_results.end() ? nullptr : &iter->second;
            }
            auto iter = m_last_results.find(handle.id);
            return iter == m_last_results.end() ? nullptr : &iter->second;
        }

        ItemResult& GUIContext::get_or_create_current_result(GUIID id)
        {
            auto iter = m_current_results.find(id);
            if(iter == m_current_results.end())
            {
                ItemResult result;
                result.generation = m_generation;
                iter = m_current_results.insert(make_pair(id, move(result))).first;
            }
            return iter->second;
        }

        PersistentItemState& GUIContext::get_or_create_persistent_state(GUIID id)
        {
            auto iter = m_persistent_states.find(id);
            if(iter == m_persistent_states.end())
            {
                PersistentItemState state;
                iter = m_persistent_states.insert(make_pair(id, state)).first;
            }
            return iter->second;
        }

        const Any* GUIContext::get_state(GUIItemHandle handle, const Name& key)
        {
            lutsassert();
            ItemResult* result = get_query_result(handle);
            if(!result) return nullptr;
            auto iter = result->states.find(key);
            return iter == result->states.end() ? nullptr : &iter->second;
        }

        void GUIContext::set_state(GUIItemHandle handle, const Name& key, const Any& value)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            ItemResult& result = get_or_create_current_result(handle.id);
            result.states.insert_or_assign(key, value);
        }

        void GUIContext::remove_state(GUIItemHandle handle, const Name& key)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            auto iter = m_current_results.find(handle.id);
            if(iter == m_current_results.end()) return;
            iter->second.states.erase(key);
        }

        Float2U GUIContext::measure_node(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            f32 font_size = 16.0f * m_frame_desc.dpi_scale;
            f32 text_width = (f32)node.text.size() * font_size * 0.52f;
            switch(node.kind)
            {
            case GUINodeKind::text:
                return Float2U(max(text_width, 1.0f), font_size + 4.0f);
            case GUINodeKind::button:
                return Float2U(max(text_width + 24.0f, 72.0f), 30.0f);
            case GUINodeKind::checkbox:
                return Float2U(max(text_width + 30.0f, 80.0f), 26.0f);
            case GUINodeKind::input_text:
                return Float2U(node.requested_size.width > 0.0f ? node.requested_size.width : 240.0f, 30.0f);
            case GUINodeKind::image:
                return Float2U(max(node.requested_size.width, 1.0f), max(node.requested_size.height, 1.0f));
            case GUINodeKind::collapsing_header:
                return Float2U(max(text_width + 32.0f, 120.0f), 30.0f);
            case GUINodeKind::row:
            {
                f32 w = 0.0f;
                f32 h = 0.0f;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    Float2U child_size = measure_node(child);
                    if(w > 0.0f) w += 6.0f;
                    w += child_size.x;
                    h = max(h, child_size.y);
                }
                return Float2U(w, max(h, 1.0f));
            }
            default:
            {
                if(node.requested_size.width > 0.0f && node.requested_size.height > 0.0f)
                {
                    return Float2U(node.requested_size.width, node.requested_size.height);
                }
                f32 w = 0.0f;
                f32 h = 0.0f;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    Float2U child_size = measure_node(child);
                    w = max(w, child_size.x);
                    h += child_size.y + 6.0f;
                }
                if(h > 0.0f) h -= 6.0f;
                return Float2U(max(w, 1.0f), max(h, 1.0f));
            }
            }
        }

        RectF GUIContext::layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            RectF effective_clip = intersect_rect(rect, clip_rect);
            m_layouts[node_index].rect = rect;
            m_layouts[node_index].clip_rect = effective_clip;

            if(node.kind != GUINodeKind::root)
            {
                ItemResult& result = get_or_create_current_result(node.id);
                result.states.insert_or_assign(Name("gui.rect"), Any(rect));
                result.states.insert_or_assign(Name("gui.clip_rect"), Any(effective_clip));
            }

            if(node.first_child == U32_MAX) return rect;

            if(node.kind == GUINodeKind::row)
            {
                f32 cursor_x = rect.offset_x;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    Float2U child_size = measure_node(child);
                    RectF child_rect(cursor_x, rect.offset_y, child_size.x, rect.height);
                    layout_node(child, child_rect, effective_clip);
                    cursor_x += child_size.x + 6.0f;
                }
            }
            else
            {
                f32 cursor_y = rect.offset_y;
                f32 content_x = rect.offset_x;
                f32 content_w = rect.width;
                if(node.kind == GUINodeKind::window || node.kind == GUINodeKind::scroll_view)
                {
                    content_x += 8.0f;
                    cursor_y += 8.0f;
                    content_w = max(content_w - 16.0f, 1.0f);
                }
                if(node.kind == GUINodeKind::scroll_view)
                {
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    cursor_y -= persistent.scroll_y;
                }
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    Float2U child_size = measure_node(child);
                    f32 child_width = min(max(child_size.x, content_w), content_w);
                    if(m_submitted_desc.nodes[child].kind == GUINodeKind::row ||
                        m_submitted_desc.nodes[child].kind == GUINodeKind::column ||
                        m_submitted_desc.nodes[child].kind == GUINodeKind::scroll_view ||
                        m_submitted_desc.nodes[child].kind == GUINodeKind::window)
                    {
                        child_width = content_w;
                    }
                    RectF child_rect(content_x, cursor_y, child_width, child_size.y);
                    layout_node(child, child_rect, effective_clip);
                    cursor_y += child_size.y + 6.0f;
                }
            }
            return rect;
        }

        GUIID GUIContext::hit_test(const Float2U& pos) const
        {
            GUIID ret = 0;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(!node.interactive) continue;
                const RectF& rect = m_layouts[i].rect;
                const RectF& clip = m_layouts[i].clip_rect;
                if(point_in_rect(pos, rect) && point_in_rect(pos, clip))
                {
                    ret = node.id;
                }
            }
            return ret;
        }

        GUIID GUIContext::hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const
        {
            GUIID ret = 0;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.kind != kind) continue;
                const RectF& rect = m_layouts[i].rect;
                const RectF& clip = m_layouts[i].clip_rect;
                if(point_in_rect(pos, rect) && point_in_rect(pos, clip))
                {
                    ret = node.id;
                }
            }
            return ret;
        }

        void GUIContext::process_input_events()
        {
            for(const GUIInputEvent& e : m_input_events)
            {
                if(e.type == GUIInputEventType::pointer_enter)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                }
                else if(e.type == GUIInputEventType::pointer_leave)
                {
                    m_pointer_inside = false;
                    m_hovered_id = 0;
                }
                else if(e.type == GUIInputEventType::pointer_move)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                }
                else if(e.type == GUIInputEventType::pointer_down)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID target = hit_test(e.position);
                    m_active_id = target;
                    m_focused_id = target;
                    if(target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(target);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                    }
                }
                else if(e.type == GUIInputEventType::pointer_up)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID target = hit_test(e.position);
                    if(target && target == m_active_id)
                    {
                        ItemResult& result = get_or_create_current_result(target);
                        result.states.insert_or_assign(Name("gui.clicked"), Any(true));
                        PersistentItemState& state = get_or_create_persistent_state(target);
                        bool dbl = (m_time - state.last_click_time) <= 0.4;
                        result.states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                        state.last_click_time = m_time;
                        for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                        {
                            GUINode& node = m_submitted_desc.nodes[i];
                            if(node.id != target) continue;
                            if(node.kind == GUINodeKind::checkbox && node.bool_value)
                            {
                                *node.bool_value = !*node.bool_value;
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            else if(node.kind == GUINodeKind::collapsing_header)
                            {
                                state.open = !state.open;
                                result.states.insert_or_assign(Name("gui.open"), Any(state.open));
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            break;
                        }
                    }
                    if(m_active_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_id);
                        state.pointer_down = false;
                        state.active = false;
                    }
                    m_active_id = 0;
                }
                else if(e.type == GUIInputEventType::pointer_wheel)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID scroll_target = hit_test_node_kind(e.position, GUINodeKind::scroll_view);
                    if(scroll_target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(scroll_target);
                        state.scroll_y = max(0.0f, state.scroll_y - e.wheel_delta.y * 24.0f);
                        ItemResult& result = get_or_create_current_result(scroll_target);
                        result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    }
                }
                else if(e.type == GUIInputEventType::text_utf8)
                {
                    if(!m_focused_id) continue;
                    for(GUINode& node : m_submitted_desc.nodes)
                    {
                        if(node.id == m_focused_id && node.kind == GUINodeKind::input_text && node.string_value)
                        {
                            node.string_value->append(e.text);
                            ItemResult& result = get_or_create_current_result(node.id);
                            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            break;
                        }
                    }
                }
                else if(e.type == GUIInputEventType::key_down)
                {
                    if(e.key == GUIKey::backspace && m_focused_id)
                    {
                        for(GUINode& node : m_submitted_desc.nodes)
                        {
                            if(node.id == m_focused_id && node.kind == GUINodeKind::input_text && node.string_value && !node.string_value->empty())
                            {
                                pop_utf8_codepoint(*node.string_value);
                                ItemResult& result = get_or_create_current_result(node.id);
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                break;
                            }
                        }
                    }
                }
                else if(e.type == GUIInputEventType::blur)
                {
                    m_focused_id = 0;
                    m_active_id = 0;
                }
            }
            m_input_events.clear();

            if(m_pointer_inside)
            {
                m_hovered_id = hit_test(m_pointer_pos);
            }
            else
            {
                m_hovered_id = 0;
            }
        }

        RV GUIContext::submit(const GUIDescription& desc)
        {
            lutsassert();
            lutry
            {
                m_submitted_desc = desc;
                m_layouts.clear();
                m_layouts.resize(m_submitted_desc.nodes.size());
                HashSet<GUIID> ids;
                for(const GUINode& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    auto r = ids.insert(node.id);
                    luassert_msg(r.second, "Duplicate GUI item ID detected.");
                    ItemResult& result = get_or_create_current_result(node.id);
                    result.generation = m_generation;
                    result.states.insert_or_assign(Name("gui.clicked"), Any(false));
                    result.states.insert_or_assign(Name("gui.double_clicked"), Any(false));
                    result.states.insert_or_assign(Name("gui.hovered"), Any(false));
                    result.states.insert_or_assign(Name("gui.active"), Any(false));
                    result.states.insert_or_assign(Name("gui.focused"), Any(false));
                    result.states.insert_or_assign(Name("gui.value_changed"), Any(false));
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    if(node.kind == GUINodeKind::collapsing_header)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                }
                RectF root_rect(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
                layout_node(0, root_rect, root_rect);
                process_input_events();
                for(const GUINode& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    ItemResult& result = get_or_create_current_result(node.id);
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    result.states.insert_or_assign(Name("gui.hovered"), Any(node.id == m_hovered_id));
                    result.states.insert_or_assign(Name("gui.active"), Any(node.id == m_active_id || persistent.active));
                    result.states.insert_or_assign(Name("gui.focused"), Any(node.id == m_focused_id));
                    if(node.kind == GUINodeKind::collapsing_header)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                }
                m_submitted = true;
            }
            lucatchret;
            return ok;
        }

        RectF GUIContext::to_vg_rect(const RectF& rect) const
        {
            return RectF(rect.offset_x, m_frame_desc.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
        }

        void GUIContext::render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture)
        {
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            m_shape_draw_list->set_shape_buffer(nullptr);
            m_shape_draw_list->set_texture(texture);
            m_shape_draw_list->set_clip_rect(c);
            auto& points = m_shape_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 begin = (u32)points.size();
            if(radius > 0.0f)
            {
                VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height, min(radius, min(r.width, r.height) * 0.5f));
            }
            else
            {
                VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height);
            }
            u32 end = (u32)points.size();
            m_shape_draw_list->draw_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_shape_draw_list->set_texture(nullptr);
        }

        void GUIContext::render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color)
        {
            if(!text || !text[0]) return;
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            VG::TextArrangeSection section;
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            section.font_size = font_size;
            section.color = color;
            section.num_chars = strlen(text);
            auto arranged = VG::arrange_text(text, section.num_chars, {&section, 1}, r, VG::TextAlignment::center, VG::TextAlignment::begin);
            m_shape_draw_list->set_clip_rect(c);
            VG::commit_text_arrange_result(arranged, {&section, 1}, m_font_atlas, m_shape_draw_list);
            m_shape_draw_list->set_clip_rect(RectF(0.0f, 0.0f, 0.0f, 0.0f));
        }

        void GUIContext::render_node(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            bool hovered = false;
            bool active = false;
            auto iter = m_current_results.find(node.id);
            if(iter != m_current_results.end())
            {
                auto h = iter->second.states.find(Name("gui.hovered"));
                hovered = h != iter->second.states.end() && h->second.as<bool>() && *h->second.as<bool>();
                auto a = iter->second.states.find(Name("gui.active"));
                active = a != iter->second.states.end() && a->second.as<bool>() && *a->second.as<bool>();
            }

            switch(node.kind)
            {
            case GUINodeKind::window:
            case GUINodeKind::scroll_view:
                render_rect(rect, clip, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
                break;
            case GUINodeKind::button:
                render_rect(rect, clip, active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : (hovered ? Float4U(0.26f, 0.43f, 0.72f, 1.0f) : Float4U(0.18f, 0.28f, 0.45f, 1.0f)), 5.0f);
                render_text(rect, clip, node.text.c_str(), 16.0f * m_frame_desc.dpi_scale, Color::white());
                break;
            case GUINodeKind::text:
                render_text(rect, clip, node.text.c_str(), 16.0f * m_frame_desc.dpi_scale, Color::white());
                break;
            case GUINodeKind::checkbox:
            {
                RectF box(rect.offset_x + 2.0f, rect.offset_y + 4.0f, 18.0f, 18.0f);
                render_rect(box, clip, node.bool_value && *node.bool_value ? Float4U(0.22f, 0.55f, 0.32f, 1.0f) : Float4U(0.18f, 0.20f, 0.23f, 1.0f), 3.0f);
                RectF label(rect.offset_x + 28.0f, rect.offset_y, max(rect.width - 28.0f, 1.0f), rect.height);
                render_text(label, clip, node.text.c_str(), 16.0f * m_frame_desc.dpi_scale, Color::white());
                break;
            }
            case GUINodeKind::input_text:
                render_rect(rect, clip, node.id == m_focused_id ? Float4U(0.12f, 0.16f, 0.22f, 1.0f) : Float4U(0.08f, 0.10f, 0.13f, 1.0f), 4.0f);
                if(node.string_value)
                {
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    render_text(text_rect, clip, node.string_value->c_str(), 16.0f * m_frame_desc.dpi_scale, Color::white());
                }
                break;
            case GUINodeKind::image:
                render_rect(rect, clip, Color::white(), 0.0f, node.texture);
                break;
            case GUINodeKind::collapsing_header:
                render_rect(rect, clip, hovered ? Float4U(0.22f, 0.27f, 0.34f, 1.0f) : Float4U(0.16f, 0.19f, 0.24f, 1.0f), 4.0f);
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, rect.width - 8.0f, rect.height), clip, node.text.c_str(), 16.0f * m_frame_desc.dpi_scale, Color::white());
                break;
            default:
                break;
            }

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                render_node(child);
            }
        }

        RV GUIContext::render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target)
        {
            lutsassert();
            if(!render_target || m_submitted_desc.nodes.empty()) return ok;
            lutry
            {
                m_shape_draw_list->reset();
                render_node(0);
                luexp(m_shape_draw_list->compile());
                luexp(m_shape_renderer->begin(render_target));
                Float4x4 mat = ProjectionMatrix::make_orthographic_off_center(0.0f, m_frame_desc.surface_size.x, 0.0f, m_frame_desc.surface_size.y, 0.0f, 1.0f);
                Float4x4U umat(mat);
                m_shape_renderer->draw(m_shape_draw_list->get_vertex_buffer(), m_shape_draw_list->get_index_buffer(), m_shape_draw_list->get_draw_calls(), &umat);
                luexp(m_shape_renderer->end());
                m_shape_renderer->submit(cmdbuf);
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_API Ref<IGUIContext> new_context(RHI::IDevice* device)
        {
            Ref<GUIContext> ctx = new_object<GUIContext>();
            ctx->m_device = device ? device : RHI::get_main_device();
            ctx->m_shape_draw_list = VG::new_shape_draw_list(ctx->m_device);
            return Ref<IGUIContext>(ctx);
        }

        LUNA_GUI_API void set_current_context(IGUIContext* context)
        {
            g_current_context = context ? (GUIContext*)context->get_object() : nullptr;
        }

        LUNA_GUI_API IGUIContext* get_current_context()
        {
            return g_current_context;
        }

        static GUIContext* require_current_context()
        {
            luassert_msg(g_current_context, "No current GUI context. Call IGUIContext::begin_frame first.");
            return g_current_context;
        }

        LUNA_GUI_API void PushID(u64 id)
        {
            require_current_context()->push_id(id);
        }

        LUNA_GUI_API void PushID(const void* ptr)
        {
            require_current_context()->push_id((u64)(usize)ptr);
        }

        LUNA_GUI_API void PushID(const c8* str)
        {
            u64 h = hash_cstr(str ? str : "", FNV_OFFSET);
            require_current_context()->push_id(h);
        }

        LUNA_GUI_API void PopID()
        {
            require_current_context()->pop_id();
        }

        LUNA_GUI_API GUIItemHandle BeginColumn(const c8* label)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::column, label ? label : "Column", GUISize(), &handle);
            return handle;
        }

        LUNA_GUI_API void EndColumn()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginRow(const c8* label)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::row, label ? label : "Row", GUISize(), &handle);
            return handle;
        }

        LUNA_GUI_API void EndRow()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginScrollView(const c8* label, const GUISize& size)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::scroll_view, label ? label : "ScrollView", size, &handle);
            return handle;
        }

        LUNA_GUI_API void EndScrollView()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, const GUISize& size)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::window, label ? label : "Window", size, &handle);
            return handle;
        }

        LUNA_GUI_API void EndWindow()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle Button(const c8* label)
        {
            return require_current_context()->add_node(GUINodeKind::button, label ? label : "", true);
        }

        LUNA_GUI_API GUIItemHandle Text(const c8* text)
        {
            return require_current_context()->add_node(GUINodeKind::text, text ? text : "", false);
        }

        LUNA_GUI_API GUIItemHandle Checkbox(const c8* label, bool* value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::checkbox, label ? label : "", true);
            ctx->m_build_desc.nodes.back().bool_value = value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle InputText(const c8* label, String& value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::input_text, label ? label : "", true);
            ctx->m_build_desc.nodes.back().string_value = &value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Image(RHI::ITexture* texture, const GUISize& size)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::image, "Image", false);
            ctx->m_build_desc.nodes.back().texture = texture;
            ctx->m_build_desc.nodes.back().requested_size = size;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle CollapsingHeader(const c8* label)
        {
            return require_current_context()->add_node(GUINodeKind::collapsing_header, label ? label : "", true);
        }

        LUNA_GUI_API const Any* get_item_state_any(GUIItemHandle handle, const Name& key)
        {
            if(!handle.context) return nullptr;
            GUIContext* ctx = (GUIContext*)handle.context;
            return ctx->get_state(handle, key);
        }

        LUNA_GUI_API void set_item_state_any(GUIItemHandle handle, const Name& key, const Any& value)
        {
            if(!handle.context) return;
            GUIContext* ctx = (GUIContext*)handle.context;
            ctx->set_state(handle, key, value);
        }

        LUNA_GUI_API void remove_item_state(GUIItemHandle handle, const Name& key)
        {
            if(!handle.context) return;
            GUIContext* ctx = (GUIContext*)handle.context;
            ctx->remove_state(handle, key);
        }

        LUNA_GUI_API bool IsItemClicked(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::clicked());
        }

        LUNA_GUI_API bool IsItemDoubleClicked(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::double_clicked());
        }

        LUNA_GUI_API bool IsItemHovered(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::hovered());
        }

        LUNA_GUI_API bool IsItemActive(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::active());
        }

        LUNA_GUI_API bool IsItemFocused(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::focused());
        }

        struct GUIModule : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                register_boxed_type<GUIContext>();
                impl_interface_for_type<GUIContext, IGUIContext>();
                return ok;
            }
            virtual void on_close() override {}
        };
    }

    namespace GUI
    {
        LUNA_GUI_API Module* module_gui()
        {
            static GUIModule m;
            return &m;
        }
    }
}
