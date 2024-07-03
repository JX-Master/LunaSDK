/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeBuffer.hpp
* @author JXMaster
* @date 2024/7/2
*/
#pragma once
#include "../ShapeBuffer.hpp"

namespace Luna
{
    namespace VG
    {
        struct ShapeBuffer : IShapeBuffer
        {
            lustruct("VG::ShapeBuffer", "c8818774-b17f-4953-8820-1ff3543cd188");
            luiimpl();

            Vector<f32> m_shape_points;
            Ref<RHI::IBuffer> m_buffer;
            usize m_shape_buffer_capacity;
            bool m_dirty;

            ShapeBuffer() :
                m_shape_buffer_capacity(0),
                m_dirty(true) {}

            virtual Vector<f32>& get_shape_points(bool modify) override
            {
                if(modify) m_dirty = true;
                return m_shape_points;
            }
            virtual R<RHI::IBuffer*> build(RHI::IDevice* device) override;
        };
    }
}