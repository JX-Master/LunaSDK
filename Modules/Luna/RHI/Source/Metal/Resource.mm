/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.mm
* @author JXMaster
* @date 2023/7/13
*/
#include "Resource.h"
#include <Luna/Runtime/Profiler.hpp>
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
                    m_buffer = [m_device->m_device newBufferWithLength:desc.size options:encode_resource_options(memory_type)];
                    if(!m_buffer) return BasicError::bad_platform_call();
                    m_memory = new_object<DeviceMemory>();
                    m_memory->m_device = m_device;
                    m_memory->m_memory_type = memory_type;
                    m_memory->m_size = [m_buffer allocatedSize];
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                    memory_profiler_allocate((__bridge void*)m_buffer, m_memory->get_size());
                    memory_profiler_set_memory_domain((__bridge void*)m_buffer, "GPU", 3);
                    memory_profiler_set_memory_type((__bridge void*)m_buffer, "Buffer", 6);
#endif
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
            m_buffer = [m->m_heap newBufferWithLength:desc.size options:encode_resource_options(m->m_memory_type)];
            if(!m_buffer) return BasicError::bad_platform_call();
            m_memory = m;
            [m_buffer makeAliasable];
            return ok;
        }
        Buffer::~Buffer()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(!m_memory->m_heap) memory_profiler_deallocate((__bridge void*)m_buffer);
#endif
            m_buffer = nil;
        }
        void Buffer::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_buffer.label = label;
            }
        }
        RV Buffer::map(usize read_begin, usize read_end, void** data)
        {
            void* d = [m_buffer contents];
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
                    auto d = encode_texture_desc(memory_type, m_desc);
                    m_texture = [m_device->m_device newTextureWithDescriptor:d];
                    if(!m_texture) return BasicError::bad_platform_call();
                    m_memory = new_object<DeviceMemory>();
                    m_memory->m_device = m_device;
                    m_memory->m_memory_type = memory_type;
                    m_memory->m_size = [m_texture allocatedSize];
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                    memory_profiler_allocate((__bridge void*)m_texture, m_memory->get_size());
                    memory_profiler_set_memory_domain((__bridge void*)m_texture, "GPU", 3);
                    memory_profiler_set_memory_type((__bridge void*)m_texture, "Texture", 7);
#endif
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
                auto d = encode_texture_desc(m->m_memory_type, m_desc);
                m_texture = [m->m_heap newTextureWithDescriptor:d];
                if(!m_texture) return BasicError::bad_platform_call();
                m_memory = m;
                [m_texture makeAliasable];
            }
            lucatchret;
            return ok;
        }
        Texture::~Texture()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(m_memory && !m_memory->m_heap) memory_profiler_deallocate((__bridge void*)m_texture);
#endif
            m_texture = nil;
        }
        void Texture::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_texture.label = label;
            }
        }
        bool compare_texture_view_desc(const TextureViewDesc& lhs, const TextureViewDesc& rhs)
        {
            return
                lhs.texture == rhs.texture &&
                lhs.type == rhs.type &&
                lhs.format == rhs.format &&
                lhs.mip_slice == rhs.mip_slice &&
                lhs.mip_size == rhs.mip_size &&
                lhs.array_slice == rhs.array_slice &&
                lhs.array_size == rhs.array_size;
        }
        R<Ref<TextureView>> Texture::get_texture_view(const TextureViewDesc& validated_desc)
        {
            LockGuard guard(m_texture_views_lock);
            for (auto& v : m_texture_views)
            {
                if (compare_texture_view_desc(v.first, validated_desc))
                {
                    return v.second;
                }
            }
            // Create a new one.
            auto view = new_object<TextureView>();
            auto r = view->init(validated_desc);
            if (failed(r)) return r.errcode();
            m_texture_views.push_back(make_pair(validated_desc, view));
            return view;
        }
    }
}
