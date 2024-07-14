/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DrawList.hpp
* @author JXMaster
* @date 2024/7/14
*/
#pragma once
#include "../DrawList.hpp"
#include <Luna/Runtime/TSAssert.hpp>

namespace Luna
{
    namespace GUI
    {
        struct DrawList : IDrawList
        {
            lustruct("GUI::DrawList", "e4b6dea6-a361-4746-a7c0-4163fb4fd08b");
            luiimpl();
            lutsassert_lock();

            Ref<VG::IShapeDrawList> m_draw_list;

            struct DrawBatch
            {
                DrawListState m_state;
                Vector<VG::Vertex> m_vertices;
                Vector<u32> m_indices;
            };

            Vector<DrawBatch> m_batches;
            u32 m_current_batch = 0;

            virtual void begin(VG::IShapeDrawList* draw_list) override;

            virtual DrawListState get_state() override;

            virtual u32 push_state(DrawListState* state, bool allow_merge) override;

            virtual void pop_state(u32 pop_id) override;

            virtual VG::IShapeBuffer* get_shape_buffer() override;

            virtual void add_shape_raw(Span<const VG::Vertex> vertices, Span<const u32> indices) override;

            virtual void add_shape(u32 begin_command, u32 num_commands, 
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                const Float4U& color,
                const Float2U& min_texcoord, const Float2U& max_texcoord) override;

            virtual void end() override;
        };
    }
}