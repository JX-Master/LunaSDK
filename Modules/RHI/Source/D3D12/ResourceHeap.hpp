/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceHeap.hpp
* @author JXMaster
* @date 2022/12/2
* @brief D3D12 implementation of Resource Heap Object
*/
#pragma once
#include "D3D12Common.hpp"
#include "Device.hpp"
#include <Runtime/TSAssert.hpp>

namespace Luna
{
	namespace RHI
	{
		struct ResourceHeap : IResourceHeap
		{
			lustruct("RHI::ResourceHeap", "{22D2523E-2E6C-4503-A02F-A95C3203632A}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			ComPtr<ID3D12Heap> m_heap;
			ResourceHeapDesc m_desc;

			RV init(const ResourceHeapDesc& desc);
			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			ResourceHeapDesc get_desc()
			{
				return m_desc;
			}
			void set_name(const Name& name) { set_object_name(m_heap.Get(), name); }
			R<Ref<IResource>> new_resource(u64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value);
		};
	}
}