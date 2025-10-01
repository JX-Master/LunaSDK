/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeBuffer.cpp
* @author JXMaster
* @date 2024/7/2
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeBuffer.hpp"
#include <Luna/RHI/Device.hpp>

namespace Luna
{
    namespace VG
    {
        R<RHI::IBuffer*> ShapeBuffer::build(RHI::IDevice* device)
        {
            lutry
            {
                using namespace RHI;
                if(!m_shape_points.empty() && (m_dirty || (m_buffer->get_device() != device)))
                {
                    // Recreate buffer if needed.
                    if (!m_shape_points.empty() && 
                    ((m_shape_buffer_capacity < m_shape_points.size()) || (m_buffer->get_device() != device)))
                    {
                        u64 shape_buffer_size = m_shape_points.size() * sizeof(f32);
                        luset(m_buffer, device->new_buffer(MemoryType::upload, BufferDesc(
                            BufferUsageFlag::read_buffer, shape_buffer_size)));
                        m_shape_buffer_capacity = m_shape_points.size();
                    }
                    // Upload data.
                    void* shape_data = nullptr;
                    luexp(m_buffer->map(0, 0, &shape_data));
                    memcpy(shape_data, m_shape_points.data(), m_shape_points.size() * sizeof(f32));
                    m_buffer->unmap(0, m_shape_points.size() * sizeof(f32));
                }
                m_dirty = false;
            }
            lucatchret;
            return m_buffer.get();
        }

        LUNA_VG_API Ref<IShapeBuffer> new_shape_buffer()
        {
            return new_object<ShapeBuffer>();
        }
    }
}