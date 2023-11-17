/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "DeviceMemory.hpp"
#include "Device.hpp"
#include "TextureView.hpp"
#include <Luna/Runtime/SpinLock.hpp>
namespace Luna
{
    namespace RHI
    {
        struct Buffer : IBuffer
        {
            lustruct("RHI::Buffer", "{ab58c9db-7ddc-42b6-ad2d-d73449632aa6}");
            luiimpl();

            Ref<Device> m_device;

            BufferDesc m_desc;
            NSPtr<MTL::Buffer> m_buffer;
            Ref<DeviceMemory> m_memory;

            RV init_as_committed(MemoryType memory_type, const BufferDesc& desc);
            RV init_as_aliasing(IDeviceMemory* memory, const BufferDesc& desc);
            ~Buffer();

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_buffer.get(), name); }
            virtual IDeviceMemory* get_memory() override { return m_memory; }
            virtual BufferDesc get_desc() override { return m_desc; }
			virtual RV map(usize read_begin, usize read_end, void** data) override;
			virtual void unmap(usize write_begin, usize write_end) override {}
        };
        struct Texture : ITexture
        {
            lustruct("RHI::Texture", "{ff44d364-6802-4cd9-8916-04cd5e2439e8}");
            luiimpl();

            Ref<Device> m_device;

            TextureDesc m_desc;
            NSPtr<MTL::Texture> m_texture;
            Ref<DeviceMemory> m_memory;

            // Texture views.
			Vector<Pair<TextureViewDesc, Ref<TextureView>>> m_texture_views;
			SpinLock m_texture_views_lock;

            RV init_as_committed(MemoryType memory_type, const TextureDesc& desc);
            RV init_as_aliasing(IDeviceMemory* memory, const TextureDesc& desc);
            ~Texture();

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_texture.get(), name); }
            virtual IDeviceMemory* get_memory() override { return m_memory; }
            virtual TextureDesc get_desc() override { return m_desc; }

            virtual R<Ref<TextureView>> get_texture_view(const TextureViewDesc& validated_desc);
        };
    }
}
