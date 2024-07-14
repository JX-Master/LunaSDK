/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DrawList.cpp
* @author JXMaster
* @date 2024/7/14
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "DrawList.hpp"

namespace Luna
{
    namespace GUI
    {
        void DrawList::begin(VG::IShapeDrawList* draw_list)
        {
            lutsassert();
            m_draw_list = draw_list;
            // reset batch.
            m_batches.clear();
            m_current_batch = 0;
            auto iter = m_batches.emplace_back();
            auto& state = iter->m_state;
            state.texture = draw_list->get_texture();
            state.shape_buffer = draw_list->get_shape_buffer();
            state.sampler = draw_list->get_sampler();
            state.clip_rect = draw_list->get_clip_rect();
        }
        DrawListState DrawList::get_state()
        {
            lutsassert();
            return m_batches[m_current_batch].m_state;
        }
        u32 DrawList::push_state(DrawListState* state, bool allow_merge)
        {
            lutsassert();
            u32 current_batch = m_current_batch;
            DrawListState new_state = state ? *state : get_state();
            if(allow_merge)
            {
                // try to merge to a existing batch.
                for(u32 i = current_batch; i < m_batches.size(); ++i)
                {
                    if(m_batches[i].m_state == new_state)
                    {
                        m_current_batch = i;
                        return current_batch;
                    }
                }
            }
            // allocates a new batch.
            auto iter = m_batches.emplace_back();
            iter->m_state = new_state;
            m_current_batch = (u32)m_batches.size() - 1;
            return current_batch;
        }
        void DrawList::pop_state(u32 pop_id)
        {
            lutsassert();
            m_current_batch = pop_id;
        }
        VG::IShapeBuffer* DrawList::get_shape_buffer()
        {
            lutsassert();
            VG::IShapeBuffer* shape_buffer = m_batches[m_current_batch].m_state.shape_buffer;
            return shape_buffer ? shape_buffer : m_draw_list->get_shape_buffer();
        }
        void DrawList::add_shape_raw(Span<const VG::Vertex> vertices, Span<const u32> indices)
        {
            lutsassert();
            auto& batch = m_batches[m_current_batch];
            u32 idx_offset = (u32)batch.m_vertices.size();
            batch.m_vertices.insert(batch.m_vertices.end(), vertices);
            batch.m_indices.reserve(batch.m_indices.size() + indices.size());
            for (u32 i : indices)
            {
                batch.m_indices.push_back(idx_offset + i);
            }
        }
        void DrawList::add_shape(u32 begin_command, u32 num_commands, 
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                const Float4U& color,
                const Float2U& min_texcoord, const Float2U& max_texcoord)
        {
            lutsassert();
            auto& batch = m_batches[m_current_batch];
            u32 idx_offset = (u32)batch.m_vertices.size();
            VG::Vertex v[4];
            u32 indices[6];
            get_rect_shape_draw_vertices(v, indices, begin_command, num_commands, 
                min_position, max_position, min_shapecoord, max_shapecoord,
                color, min_texcoord, max_texcoord);
            batch.m_vertices.insert(batch.m_vertices.end(), Span<VG::Vertex>(v, 4));
            indices[0] += idx_offset;
            indices[1] += idx_offset;
            indices[2] += idx_offset;
            indices[3] += idx_offset;
            indices[4] += idx_offset;
            indices[5] += idx_offset;
            batch.m_indices.insert(batch.m_indices.end(), Span<u32>(indices, 6));
        }
        void DrawList::end()
        {
            for(auto& batch : m_batches)
            {
                if (!batch.m_vertices.empty() && !batch.m_indices.empty())
                {
                    auto& state = batch.m_state;
                    m_draw_list->set_texture(state.texture);
                    m_draw_list->set_sampler(&state.sampler);
                    m_draw_list->set_clip_rect(state.clip_rect);
                    m_draw_list->set_shape_buffer(state.shape_buffer);
                    m_draw_list->draw_shape_raw(batch.m_vertices.cspan(), batch.m_indices.cspan());
                }
            }
        }
        LUNA_GUI_API Ref<IDrawList> new_draw_list()
        {
            return new_object<DrawList>();
        }
    }
}