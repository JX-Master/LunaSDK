/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.cpp
* @author JXMaster
* @date 2023/7/13
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
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                    memory_profiler_allocate(m_buffer.get(), m_memory->get_size());
                    memory_profiler_set_memory_domain(m_buffer.get(), "GPU", 3);
                    memory_profiler_set_memory_type(m_buffer.get(), "Buffer", 6);
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
            m_buffer = box(m->m_heap->newBuffer(desc.size, encode_resource_options(m->m_memory_type)));
            if(!m_buffer) return BasicError::bad_platform_call();
            m_memory = m;
            m_buffer->makeAliasable();
            return ok;
        }
        Buffer::~Buffer()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(!m_memory->m_heap) memory_profiler_deallocate(m_buffer.get());
#endif
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
                    auto d = encode_texture_desc(memory_type, m_desc);
                    m_texture = box(m_device->m_device->newTexture(d.get()));
                    if(!m_texture) return BasicError::bad_platform_call();
                    m_memory = new_object<DeviceMemory>();
                    m_memory->m_device = m_device;
                    m_memory->m_memory_type = memory_type;
                    m_memory->m_size = m_texture->allocatedSize();
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                    memory_profiler_allocate(m_texture.get(), m_memory->get_size());
                    memory_profiler_set_memory_domain(m_texture.get(), "GPU", 3);
                    memory_profiler_set_memory_type(m_texture.get(), "Texture", 7);
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
                m_texture = box(m->m_heap->newTexture(d.get()));
                if(!m_texture) return BasicError::bad_platform_call();
                m_memory = m;
                m_texture->makeAliasable();
            }
            lucatchret;
            return ok;
        }
        Texture::~Texture()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(m_memory && !m_memory->m_heap) memory_profiler_deallocate(m_texture.get());
#endif
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
