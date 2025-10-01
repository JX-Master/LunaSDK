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
#include "ShapeDrawList.hpp"
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/RHI.hpp>

namespace Luna
{
    namespace VG
    {
        ShapeDrawCall& ShapeDrawList::get_current_draw_call()
        {
            if (m_state_dirty || m_draw_calls.empty())
            {
                new_draw_call();
                m_state_dirty = false;
            }
            return m_draw_calls.back();
        }
        void ShapeDrawList::reset()
        {
            lutsassert();
            m_draw_calls.clear();
            m_draw_call_buffers.clear();
            m_vertices.clear();
            m_indices.clear();
            m_internal_shape_buffer->get_shape_points(true).clear();
            m_shape_buffer.reset();
            m_texture.reset();
            m_sampler = get_default_sampler();
            m_transform = Float4x4::identity();
            m_clip_rect = RectF{0, 0, 0, 0};
            m_state_dirty = false;
        }
        void ShapeDrawList::draw_shape_raw(Span<const Vertex> vertices, Span<const u32> indices)
        {
            lutsassert();
            auto& dc = get_current_draw_call();
            u32 idx_offset = (u32)m_vertices.size();
            m_vertices.insert(m_vertices.end(), vertices);
            m_indices.reserve(m_indices.size() + indices.size());
            for(u32 i : indices)
            {
                m_indices.push_back(idx_offset + i);
            }
            dc.num_indices += (u32)indices.size();
        }
        void ShapeDrawList::draw_shape(u32 begin_command, u32 num_commands,
            const Float2U& min_position, const Float2U& max_position,
            const Float2U& min_shapecoord, const Float2U& max_shapecoord, const Float4U& color,
            const Float2U& min_texcoord, const Float2U& max_texcoord)
        {
            lutsassert();
            auto& dc = get_current_draw_call();
            u32 idx_offset = (u32)m_vertices.size();
            Vertex v[4];
            u32 indices[6];
            get_rect_shape_draw_vertices(v, indices, begin_command, num_commands, 
                min_position, max_position, min_shapecoord, max_shapecoord,
                color, min_texcoord, max_texcoord);
            m_vertices.insert(m_vertices.end(), Span<Vertex>(v, 4));
            indices[0] += idx_offset;
            indices[1] += idx_offset;
            indices[2] += idx_offset;
            indices[3] += idx_offset;
            indices[4] += idx_offset;
            indices[5] += idx_offset;
            m_indices.insert(m_indices.end(), Span<u32>(indices, 6));
            dc.num_indices += 6;
        }
        RV ShapeDrawList::compile()
        {
            lutsassert();
            lutry
            {
                // Pack data.
                u32 num_vertices = (u32)m_vertices.size();
                u32 num_indices = (u32)m_indices.size();
                u32 num_internal_buffer_points = (u32)m_internal_shape_buffer->get_shape_points().size();
                if (m_vertex_buffer_capacity < num_vertices)
                {
                    // Recreate vertex buffer.
                    luset(m_vertex_buffer, m_device->new_buffer(RHI::MemoryType::upload, RHI::BufferDesc(
                        RHI::BufferUsageFlag::vertex_buffer, num_vertices * sizeof(Vertex))));
                    m_vertex_buffer_capacity = num_vertices;
                }
                m_vertex_buffer_size = num_vertices;
                if (m_index_buffer_capacity < num_indices)
                {
                    // Recreate index buffer.
                    luset(m_index_buffer, m_device->new_buffer(RHI::MemoryType::upload, RHI::BufferDesc(
                        RHI::BufferUsageFlag::index_buffer, num_indices * sizeof(u32))));
                    m_index_buffer_capacity = num_indices;
                }
                m_index_buffer_size = num_indices;
                if(m_vertex_buffer)
                {
                    Vertex* vertex_data = nullptr;
                    luexp(m_vertex_buffer->map(0, 0, (void**)&vertex_data));
                    memcpy(vertex_data, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
                    m_vertex_buffer->unmap(0, sizeof(Vertex) * m_vertices.size());
                }
                if(m_index_buffer)
                {
                    u32* index_data = nullptr;
                    luexp(m_index_buffer->map(0, 0, (void**)&index_data));
                    memcpy(index_data, m_indices.data(), m_indices.size() * sizeof(u32));
                    m_index_buffer->unmap(0, sizeof(u32) * m_indices.size());
                }
                for(usize i = 0; i < m_draw_calls.size(); ++i)
                {
                    if(!m_draw_call_buffers[i])
                    {
                        luset(m_draw_calls[i].shape_buffer, m_internal_shape_buffer->build(m_device));
                    }
                    else
                    {
                        luset(m_draw_calls[i].shape_buffer, m_draw_call_buffers[i]->build(m_device));
                    }
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_VG_API void get_rect_shape_draw_vertices(Vertex out_vertices[4], u32 out_indices[6],
            u32 begin_command, u32 num_commands,
            const Float2U& min_position, const Float2U& max_position,
            const Float2U& min_shapecoord, const Float2U& max_shapecoord,
            const Float4U& color,
            const Float2U& min_texcoord, const Float2U& max_texcoord)
        {
            out_vertices[0].position = min_position;
            //out_vertices[1].position = Float2U(min_position.x, max_position.y);
            out_vertices[1].position.x = min_position.x;
            out_vertices[1].position.y = max_position.y;
            out_vertices[2].position = max_position;
            //out_vertices[3].position = Float2U(max_position.x, min_position.y);
            out_vertices[3].position.x = max_position.x;
            out_vertices[3].position.y = min_position.y;
            out_vertices[0].shapecoord = min_shapecoord;
            //out_vertices[1].shapecoord = Float2U(min_shapecoord.x, max_shapecoord.y);
            out_vertices[1].shapecoord.x = min_shapecoord.x;
            out_vertices[1].shapecoord.y = max_shapecoord.y;
            out_vertices[2].shapecoord = max_shapecoord;
            //out_vertices[3].shapecoord = Float2U(max_shapecoord.x, min_shapecoord.y);
            out_vertices[3].shapecoord.x = max_shapecoord.x;
            out_vertices[3].shapecoord.y = min_shapecoord.y;
            out_vertices[0].texcoord = min_texcoord;
            //out_vertices[1].texcoord = Float2U(min_texcoord.x, max_texcoord.y);
            out_vertices[1].texcoord.x = min_texcoord.x;
            out_vertices[1].texcoord.y = max_texcoord.y;
            out_vertices[2].texcoord = max_texcoord;
            //out_vertices[3].texcoord = Float2U(max_texcoord.x, min_texcoord.y);
            out_vertices[3].texcoord.x = max_texcoord.x;
            out_vertices[3].texcoord.y = min_texcoord.y;
            out_vertices[0].color = out_vertices[1].color = out_vertices[2].color = out_vertices[3].color = color;
            out_vertices[0].begin_command = out_vertices[1].begin_command = out_vertices[2].begin_command = out_vertices[3].begin_command = begin_command;
            out_vertices[0].num_commands = out_vertices[1].num_commands = out_vertices[2].num_commands = out_vertices[3].num_commands = num_commands;
            out_indices[0] = 0;
            out_indices[1] = 1;
            out_indices[2] = 2;
            out_indices[3] = 0;
            out_indices[4] = 2;
            out_indices[5] = 3;
        }
        LUNA_VG_API Ref<IShapeDrawList> new_shape_draw_list(RHI::IDevice* device)
        {
            auto dl = new_object<ShapeDrawList>();
            dl->m_device = device ? device : RHI::get_main_device();
            return dl;
        }
    }
}