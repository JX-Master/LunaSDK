/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceHeap.hpp
* @author JXMaster
* @date 2022/12/2
*/
#pragma once
#include "Resource.hpp"
#include <Runtime/Ref.hpp>
namespace Luna
{
	namespace RHI
	{
		//! Describes resources that can be placed in the heap.
		enum class ResourceHeapUsageFlag : u8
		{
			none = 0,
			//! Allow buffer resources being created in the heap.
			buffer = 0x01,
			//! Allow textures without `ResourceUsage::render_target` and `ResourceUsage::depth_stencil`
			//! usages being created in this heap.
			texture_non_rt_ds = 0x02,
			//! Allow textures with `ResourceUsage::render_target` and `ResourceUsage::depth_stencil`
			//! usages being created in this heap.
			texture_rt_ds = 0x04,
			//! Allow MSAA textures being created in this heap.
			texture_msaa = 0x08,
		};

		struct ResourceHeapDesc
		{
			//! The type of the resource heap.
			ResourceHeapType type;
			//! The type of resources that can be created in this heap.
			ResourceHeapUsageFlag usages;
			//! The size of the resource heap in bytes.
			u64 size;
		};

		//! @interface IResourceHeap
		//! Represents one pre-allocated resource heap that can be used to allocate memory for resource, enabling fast resource creation and
		//! resources memory overlapping.
		struct IResourceHeap : virtual IDeviceChild
		{
			luiid("{323095DC-C2C0-4B6A-B3AE-D68781A0465F}");

			//! Gets the descriptor of the resource heap.
			virtual ResourceHeapDesc get_desc() = 0;

			//! Creates one resource in the heap.
			//! @param[in] heap_offset The memory address offset of the resource to be created from heap begin.
			//! The `heap_offset` must be a multiple of the resource's alignment, and `heap_offset` plus the resource size must be smaller than or equal to the heap size.
			//! @param[in] desc The resource descriptor object.
			//! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if this is a buffer
			//! resource or the resource does not have a optimized clear value.
			virtual R<Ref<IResource>> new_resource(u64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;
		};
	}
}