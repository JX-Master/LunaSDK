/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Buffer.hpp
* @author JXMaster
* @date 2023/9/20
*/
#pragma once
#include "Resource.hpp"

namespace Luna
{
    namespace RHI
    {
        enum class BufferUsageFlag : u16
		{
			none = 0x00,
			//! Allows this resource to be bound as copy source.
			copy_source = 0x01,
			//! Allows this resource to be bound as copy destination.
			copy_dest = 0x02,
			//! Allows this resource to be bound to a uniform buffer view.
			uniform_buffer = 0x04,
			//! Allows this resource to be bound to a read buffer view.
			read_buffer = 0x08,
			//! Allows this resource to be bound to a read-write buffer view.
			read_write_buffer = 0x10,
			//! Allows this resource to be bound as a vertex buffer.
			vertex_buffer = 0x20,
			//! Allows this resource to be bound as a index buffer.
			index_buffer = 0x40,
			//! Allows this resource to be bound as a buffer providing indirect draw arguments.
			indirect_buffer = 0x80,
		};

		struct BufferDesc
		{
			//! The size of the buffer in bytes.
			u64 size;
			//! A combination of `BufferUsageFlag` flags to indicate all possible 
			//! usages of this buffer.
			BufferUsageFlag usages;
			//! The resource flags.
			ResourceFlag flags;

			BufferDesc() = default;
			BufferDesc(const BufferDesc&) = default;
			BufferDesc& operator=(const BufferDesc&) = default;
			BufferDesc(BufferUsageFlag usages, u64 size, ResourceFlag flags = ResourceFlag::none) :
				usages(usages),
				size(size) {}
		};

        struct IBuffer : virtual IResource
		{
			luiid("{548E82ED-947F-4F4C-95A0-DC0607C96C54}");

			virtual BufferDesc get_desc() = 0;

			//! Maps the resource data to system memory and enables CPU access to the resource data.
			//! Map/unmap operations are reference counted, for each `map` operation, you need to call `unmap` once to finally unmap the memory.
			//! @param[in] read_begin The offset, in bytes, of the beginning of the data range that will be read by CPU.
			//! @param[in] read_end The offset, in bytes, of the ending of the data range that will be read by CPU.
			//! @param[out] data Returns the pointer to the mapped memory. The returned pointer is not offsetted by `read_begin` and always points 
			//! to the beginning of the resource data, but only data in [pointer + read_begin, pointer + read_end) range is valid for reading from CPU.
			//! 
			//! If `read_end <= read_begin`, no data will be read by CPU, which is required if resource heap type is not `MemoryType::readback`.
			//! 
			//! If `read_end` is larger than the subresource size (like setting to `USIZE_MAX`), the read range will be clamped to [read_begin, resource_size). 
			//! 
			virtual RV map(usize read_begin, usize read_end, void** data) = 0;

			//! Invalidates the pointer to the mapped data, and synchronizes changed data with device when needed.
			//! Map/unmap operations are reference counted, for each `map` operation, you need to call `unmap` once to finally unmap the memory.
			//! @param[in] write_begin The offset, in bytes, of the beginning of the data range that is changed by CPU and should be synchronized. 
			//! @param[in] write_end The offset, in bytes, of the ending of the data range that is changed by CPU and should be synchronized.
			//! 
			//! If `write_begin <= write_end`, no data will be synchronized, which is required if resource heap type is not `MemoryType::upload`.
			//! 
			//! If `write_end` is larger than the subresource size (like setting to `USIZE_MAX`), the write range will be clamped to [write_begin, resource_size). 
			virtual void unmap(usize write_begin, usize write_end) = 0;
		};
    }
}