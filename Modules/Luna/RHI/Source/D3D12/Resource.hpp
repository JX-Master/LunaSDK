/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.hpp
* @author JXMaster
* @date 2019/8/10
* @brief D3D12 implementation of Resource Object
*/
#pragma once
#include "D3D12Common.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include <Luna/Runtime/TSAssert.hpp>
namespace Luna
{
	namespace RHI
	{
		struct BufferResource : IBuffer
		{
			lustruct("RHI::BufferResource", "{A96361DD-C552-4C1C-8E4B-D50D52828626}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			ComPtr<ID3D12Resource> m_res;
			Ref<DeviceMemory> m_memory;
			BufferDesc m_desc;
			Name m_name;

			RV init_as_committed(MemoryType memory_type, const BufferDesc& desc);
			RV init_as_aliasing(const BufferDesc& desc, DeviceMemory* memory);

			virtual IDevice* get_device() override { return m_device; }
			virtual void set_name(const c8* name) override { m_name = name; set_object_name(m_res.Get(), name); }
			virtual IDeviceMemory* get_memory() override { return m_memory; }
			virtual BufferDesc get_desc() override { return m_desc; }
			virtual RV map(usize read_begin, usize read_end, void** data) override;
			virtual void unmap(usize write_begin, usize write_end) override;
		};
		struct TextureResource : ITexture
		{
			lustruct("RHI::TextureResource", "{5AC5B94D-5EAE-4672-98F3-7C4C557C9F01}");
			luiimpl();

			Ref<Device> m_device;
			ComPtr<ID3D12Resource> m_res;
			Ref<DeviceMemory> m_memory;
			TextureDesc m_desc;
			Vector<D3D12_RESOURCE_STATES> m_states;
			Name m_name;

			Vector<Pair<TextureViewDesc, ComPtr<ID3D12DescriptorHeap>>> m_rtvs;
			Vector<Pair<TextureViewDesc, ComPtr<ID3D12DescriptorHeap>>> m_dsvs;
			SpinLock m_views_lock;

			R<ID3D12DescriptorHeap*> get_rtv(const TextureViewDesc& desc);
			R<ID3D12DescriptorHeap*> get_dsv(const TextureViewDesc& desc);

			~TextureResource();

			u32 count_subresources() const
			{
				return m_desc.mip_levels * m_desc.array_size;
			}
			RV init_as_committed(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value);
			RV init_as_aliasing(const TextureDesc& desc, DeviceMemory* memory, const ClearValue* optimized_clear_value);
			void post_init();

			virtual IDevice* get_device() override { return m_device; }
			virtual void set_name(const c8* name) override { m_name = name; set_object_name(m_res.Get(), name); }
			virtual IDeviceMemory* get_memory() override { return m_memory; }
			virtual TextureDesc get_desc() override { return m_desc; }
		};
	}
}