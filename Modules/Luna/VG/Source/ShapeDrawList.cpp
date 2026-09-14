/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeDrawList.cpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeDrawListImpl.hpp"
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/RHI.hpp>

namespace Luna
{
    namespace VG
    {
        static_assert(sizeof(ShapeState) == sizeof(f32) * 28,
            "ShapeState must match the structured-buffer layout used by FillVS.");

        ShapeDrawCall& ShapeDrawList::get_current_draw_call()
        {
            IShapeBuffer* shape_buffer = get_effective_shape_buffer();
            if (m_force_new_draw_call || m_draw_calls.empty() ||
                m_draw_call_buffers.back() != shape_buffer ||
                m_draw_calls.back().texture != m_texture ||
                !sampler_desc_equal(m_draw_calls.back().sampler, m_sampler))
            {
                new_draw_call(shape_buffer);
                m_force_new_draw_call = false;
            }
            return m_draw_calls.back();
        }
        u32 ShapeDrawList::resolve_current_state_index()
        {
            if (!m_state_index_dirty)
            {
                return m_current_state_index;
            }
            ShapeState state;
            state.transform = m_transform;
            state.clip_rect = Float4U(m_clip_rect.offset_x, m_clip_rect.offset_y,
                m_clip_rect.width, m_clip_rect.height);
            state.rounded_clip_rect = Float4U(m_rounded_clip_rect.offset_x, m_rounded_clip_rect.offset_y,
                m_rounded_clip_rect.width, m_rounded_clip_rect.height);
            state.rounded_clip_radii = m_rounded_clip_radii;
            for (usize i = 0; i < m_states.size(); ++i)
            {
                const ShapeState& existing_state = m_states[i];
                if (!memcmp(&existing_state, &state, sizeof(ShapeState)))
                {
                    m_current_state_index = (u32)i;
                    m_state_index_dirty = false;
                    return m_current_state_index;
                }
            }
            m_current_state_index = (u32)m_states.size();
            m_states.push_back(state);
            m_state_index_dirty = false;
            return m_current_state_index;
        }
        void ShapeDrawList::reset()
        {
            lutsassert();
            m_draw_calls.clear();
            m_draw_call_buffers.clear();
            m_instances.clear();
            m_states.clear();
            m_internal_shape_buffer->get_shape_points(true).clear();
            m_shape_buffer.reset();
            m_texture.reset();
            m_sampler = get_default_sampler();
            m_transform = Float4x4::identity();
            m_clip_rect = RectF{0, 0, 0, 0};
            m_rounded_clip_rect = RectF{0, 0, 0, 0};
            m_rounded_clip_radii = Float4U(0.0f);
            m_state_index_dirty = true;
            m_current_state_index = 0;
            m_force_new_draw_call = false;
        }
        void ShapeDrawList::draw_shape(u32 begin_command, u32 num_commands,
            const Float2U& min_position, const Float2U& max_position,
            const Float2U& min_shapecoord, const Float2U& max_shapecoord, const Float4U& color,
            const Float2U& min_texcoord, const Float2U& max_texcoord)
        {
            lutsassert();
            auto& dc = get_current_draw_call();
            ShapeInstance instance;
            instance.position_bounds = Float4U(min_position.x, min_position.y,
                max_position.x, max_position.y);
            instance.shapecoord_bounds = Float4U(min_shapecoord.x, min_shapecoord.y,
                max_shapecoord.x, max_shapecoord.y);
            instance.texcoord_bounds = Float4U(min_texcoord.x, min_texcoord.y,
                max_texcoord.x, max_texcoord.y);
            instance.command_range_and_state = UInt4U(begin_command, num_commands,
                resolve_current_state_index(), 0);
            instance.color = color;
            m_instances.push_back(instance);
            ++dc.num_instances;
        }
        RV ShapeDrawList::compile()
        {
            lutsassert();
            lutry
            {
                // Pack data.
                u32 num_instances = (u32)m_instances.size();
                if (m_instance_buffer_capacity < num_instances)
                {
                    luset(m_instance_buffer, m_device->new_buffer(RHI::MemoryType::upload, RHI::BufferDesc(
                        RHI::BufferUsageFlag::vertex_buffer, num_instances * sizeof(ShapeInstance))));
                    m_instance_buffer_capacity = num_instances;
                }
                m_instance_buffer_size = num_instances;
                if(m_instance_buffer)
                {
                    ShapeInstance* instance_data = nullptr;
                    luexp(m_instance_buffer->map(0, 0, (void**)&instance_data));
                    memcpy(instance_data, m_instances.data(), m_instances.size() * sizeof(ShapeInstance));
                    m_instance_buffer->unmap(0, sizeof(ShapeInstance) * m_instances.size());
                }
                u32 num_states = (u32)m_states.size();
                if (m_state_buffer_capacity < num_states)
                {
                    luset(m_state_buffer, m_device->new_buffer(RHI::MemoryType::upload, RHI::BufferDesc(
                        RHI::BufferUsageFlag::read_buffer, num_states * sizeof(ShapeState))));
                    m_state_buffer_capacity = num_states;
                }
                m_state_buffer_size = num_states;
                if (m_state_buffer)
                {
                    ShapeState* state_data = nullptr;
                    luexp(m_state_buffer->map(0, 0, (void**)&state_data));
                    memcpy(state_data, m_states.data(), m_states.size() * sizeof(ShapeState));
                    m_state_buffer->unmap(0, sizeof(ShapeState) * m_states.size());
                }
                for(usize i = 0; i < m_draw_calls.size(); ++i)
                {
                    luset(m_draw_calls[i].shape_buffer, m_draw_call_buffers[i]->build(m_device));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_VG_API Ref<IShapeDrawList> new_shape_draw_list(RHI::IDevice* device)
        {
            auto dl = new_object<ShapeDrawList>();
            dl->m_device = device ? device : RHI::get_main_device();
            return dl;
        }
    }
}
