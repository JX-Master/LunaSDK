/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.cpp
* @author JXMaster
* @date 2022/7/13
*/
#include "Resource.hpp"
namespace Luna
{
    namespace RHI
    {
        RV Buffer::init_as_committed(MemoryType memory_type, const BufferDesc& desc)
        {
            lutry
            {
                if(test_flags(m_desc.flags, ResourceFlag::allow_aliasing))
                {
                    lulet(memory, m_device->allocate_memory(memory_type, {m_desc}, {}));
                    luexp(init_as_aliasing(memory, desc));
                }
                else
                {
                    m_desc = desc;
                    m_buffer = box(m_device->m_device->newBuffer(desc.size, encode_resource_options(memory_type)));
                    if(!m_buffer) return BasicError::bad_platform_call();
                    m_memory = new_object<DeviceMemory>();
                    m_memory->m_device = m_device;
                    m_memory->m_memory_type = memory_type;
                    m_memory->m_size = m_buffer->allocatedSize();
                }
            }
            lucatchret;
            return ok;
        }
        RV Buffer::init_as_aliasing(IDeviceMemory* memory, const BufferDesc& desc)
        {
            DeviceMemory* m = cast_object<DeviceMemory>(memory->get_object());
            if(!m->m_heap) return BasicError::not_supported();
            m_desc = desc;
            m_buffer = box(m->m_heap->newBuffer(desc.size, encode_resource_options(m->m_memory_type)));
            if(!m_buffer) return BasicError::bad_platform_call();
            m_memory = m;
            m_buffer->makeAliasable();
            return ok;
        }
        RV Buffer::map(usize read_begin, usize read_end, void** data)
        {
            void* d = m_buffer->contents();
            if(data) *data = d;
            return d ? ok : BasicError::not_supported();
        }
        RV Texture::init_as_committed(MemoryType memory_type, const TextureDesc& desc)
        {
            lutry
            {
                if(test_flags(desc.flags, ResourceFlag::allow_aliasing))
                {
                    lulet(memory, m_device->allocate_memory(memory_type, {}, {desc}));
                    luexp(init_as_aliasing(memory, desc));
                }
                else
                {
                    m_desc = desc;
                    luexp(validate_texture_desc(m_desc));
                    auto d = encode_texture_desc(memory_type, desc);
                    m_texture = box(m_device->m_device->newTexture(d.get()));
                    if(!m_texture) return BasicError::bad_platform_call();
                    m_memory = new_object<DeviceMemory>();
                    m_memory->m_device = m_device;
                    m_memory->m_memory_type = memory_type;
                    m_memory->m_size = m_texture->allocatedSize();
                }
            }
            lucatchret;
            return ok;
        }
        RV Texture::init_as_aliasing(IDeviceMemory* memory, const TextureDesc& desc)
        {
            lutry
            {
                DeviceMemory* m = cast_object<DeviceMemory>(memory->get_object());
                if(!m->m_heap) return BasicError::not_supported();
                m_desc = desc;
                luexp(validate_texture_desc(m_desc));
                auto d = encode_texture_desc(m->m_memory_type, desc);
                m_texture = box(m->m_heap->newTexture(d.get()));
                if(!m_texture) return BasicError::bad_platform_call();
                m_memory = m;
                m_texture->makeAliasable();
            }
            lucatchret;
            return ok;
        }
    }
}