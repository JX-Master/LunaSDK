/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Resource.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "DeviceMemory.hpp"
#include "ImageView.hpp"

namespace Luna
{
	namespace RHI
	{
		struct BufferResource : IBuffer
		{
			lustruct("RHI::BufferResource", "{2CE2F6F7-9CCB-4DD5-848A-DBE27F8A8B7A}");
			luiimpl();

			Ref<Device> m_device;

			BufferDesc m_desc;
			VkBuffer m_buffer = VK_NULL_HANDLE;
			Ref<DeviceMemory> m_memory;

			u32 m_owning_queue_family_index = U32_MAX;

			RV init_as_committed(MemoryType memory_type, const BufferDesc& desc);
			RV init_as_aliasing(const BufferDesc& desc, DeviceMemory* memory);
			~BufferResource();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override;
			virtual IDeviceMemory* get_memory() override { return m_memory; }
			virtual BufferDesc get_desc() override { return m_desc; }
			virtual RV map(usize read_begin, usize read_end, void** data) override;
			virtual void unmap(usize write_begin, usize write_end) override;
		};

		struct ImageGlobalState
		{
			VkImageLayout m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			u32 m_owning_queue_family_index = U32_MAX;
		};

		struct ImageResource : ITexture
		{
			lustruct("RHI::ImageResource", "{731F1D3C-2864-44A4-B380-CF03CBB7AFED}");
			luiimpl();
		
			Ref<Device> m_device;

			TextureDesc m_desc;
			VkImage m_image = VK_NULL_HANDLE;
			// This may be `nullptr` if the image memory is managed by external entities, 
			// for example, swap chains.
			Ref<DeviceMemory> m_memory;

			// Global state.
			Vector<ImageGlobalState> m_global_states;

			// Image views.
			Vector<Pair<TextureViewDesc, Ref<ImageView>>> m_image_views;
			SpinLock m_image_views_lock;

			// true if this is a swap chain resource.
			bool m_is_image_externally_managed = false;

			R<ImageView*> get_image_view(const TextureViewDesc& create_info);

			void post_init();
			RV init_as_committed(MemoryType memory_type, const TextureDesc& desc);
			RV init_as_aliasing(const TextureDesc& desc, DeviceMemory* memory);
			~ImageResource();

			u32 count_subresources() const
			{
				return m_desc.mip_levels * m_desc.array_size;
			}

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override;
			virtual IDeviceMemory* get_memory() override { return m_memory; }
			virtual TextureDesc get_desc() override { return m_desc; }
		};
	}
}