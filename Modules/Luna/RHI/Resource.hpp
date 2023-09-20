/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.hpp
* @author JXMaster
* @date 2019/7/20
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include "DeviceMemory.hpp"
namespace Luna
{
	namespace RHI
	{
		enum class ResourceFlag : u16
		{
			none = 0,
			//! Specify this flag will allow multiple resources sharing the same memory with this resource.
			//! If this flag is not set when calling `new_buffer` or `new_texture`, the memory allocated along
			//! with the resource cannot be used for creating another resource by calling `new_aliasing_buffer`
			//! or `new_aliasing_texture`.
			//! 
			//! This flag is ignored and will be set for new resources created with `new_aliasing_buffer` or `new_aliasing_texture`, 
			//! since such resources are always aliased.
			allow_aliasing = 0x01,
		};

		struct SubresourceIndex
		{
			//! The mip index of the subresource.
			u32 mip_slice;
			//! The array index of the subresource.
			u32 array_slice;

			SubresourceIndex() = default;
			constexpr SubresourceIndex(u32 mip_slice, u32 array_slice) :
				mip_slice(mip_slice),
				array_slice(array_slice) {}
			bool operator==(const SubresourceIndex& rhs) const { return mip_slice == rhs.mip_slice && array_slice == rhs.array_slice; }
		};
	}
	template <>
	struct hash<RHI::SubresourceIndex>
	{
		usize operator()(const RHI::SubresourceIndex& value)
		{
			return memhash(&value, sizeof(RHI::SubresourceIndex));
		}
	};
	namespace RHI
	{

		//! @interface IResource
		//! Represents a memory region that can be accessed by GPU.
		struct IResource : virtual IDeviceChild
		{
			luiid("{D67C47CD-1FF3-4FA4-82FE-773EC5C8AD2A}");

			//! Gets the device memory object that holds memory of this resource.
			virtual IDeviceMemory* get_memory() = 0;
		};
	}
}
