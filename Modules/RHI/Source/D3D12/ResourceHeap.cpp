/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceHeap.cpp
* @author JXMaster
* @date 2022/12/2
* @brief D3D12 implementation of Resource Heap Object
*/
#include "ResourceHeap.hpp"

#ifdef LUNA_RHI_D3D12

#include "Resource.hpp"
#include "../../RHI.hpp"

namespace Luna
{
	namespace RHI
	{
		RV ResourceHeap::init(const ResourceHeapDesc& desc)
		{
			D3D12_HEAP_DESC d;
			d.SizeInBytes = desc.size;
			d.Properties = encode_heap_properties((Device*)m_device.object(), desc.type);
			d.Alignment = desc.alignment;
			d.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
			if(test_flags(desc.child_types, ResourceHeapChildType::buffer))
			{
				d.Flags &= ~D3D12_HEAP_FLAG_DENY_BUFFERS;
			}
			if(test_flags(desc.child_types, ResourceHeapChildType::texture_rt_ds))
			{
				d.Flags &= ~D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
			}
			if(test_flags(desc.child_types, ResourceHeapChildType::texture_non_rt_ds))
			{
				d.Flags &= ~D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
			}
			if (FAILED(m_device->m_device->CreateHeap(&d, IID_PPV_ARGS(&m_heap))))
			{
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		R<Ref<IResource>> ResourceHeap::new_resource(u64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			lutsassert();
			Ref<Resource> obj = new_object<Resource>();
			obj->m_device = m_device;
			auto r = obj->init_as_placed(m_heap.Get(), heap_offset, desc, optimized_clear_value);
			if (failed(r))
			{
				return r.errcode();
			}
			return obj;
		}
	}
}

#endif