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
#include <Runtime/Object.hpp>
#include <Runtime/Result.hpp>
#include <Runtime/Math/Math.hpp>
#include "DeviceChild.hpp"
namespace Luna
{
	namespace RHI
	{
		enum class ResourceType : u8
		{
			buffer,
			texture_1d,
			texture_2d,
			texture_3d
		};

		//! Specify the resource heaps types. See remarks for details.
		//! @remark The resource heap type determines the memory location and access policy for the heap.
		//! The system will choose the most suitable memory location to allocate the resource heap or resource
		//! based on the target platform and the target resource heap type.
		//! 
		//! The local heap type is allocated on memory that is visible only to GPU. Such memory gains maximum 
		//! GPU bandwidth, but cannot be accessed by CPU. On platforms with non-uniform memory architecture (NUMA),
		//! the local heap will be allocated on video memory, which cannot be accessed by CPU; in a platform with 
		//! uniform memory architecture (UMA), the local heap will be allocated on system memory. While it is 
		//! technically possible for CPU to access local heap on UMA, preventing such access gives the hardware and
		//! driver more rooms for optimizing GPU access efficiency.
		//!
		//! The upload heap type is allocated on system memory that is optimized for CPU writing. GPU cannot write to this heap and
		//! GPU reading from upload heap is slow. On NUMA platfroms, reading data from upload heap from GPU requires data transmission 
		//! through PCI-Express bus, which is much slower than reading data in local heap from GPU. We recommend using upload heap only 
		//! for uploading data to local heap or reading the data only once per CPU write.
		//! 
		//! The readback heap type is allocated on system memory that is optimized for CPU reading. GPU writing to read back heap 
		//! is slow, and the only operation allowed for GPU is to copy data to the heap. On NUMA platfroms, writing data to readback heap 
		//! from GPU requires data transmission through PCI-Express bus, which is a slow operation.
		//! 
		//! The user should choose the suitable heap type based on the use situation. Here are some basic principles:
		//! 1. If you need to create texture resources, use local heap. If you need to upload texture data from CPU side, use one upload heap 
		//! to copy data to the local heap.
		//! 2. If you don't need to access resource data from CPU, use local heap.
		//! 3. If you only need to upload data from CPU side once, like setting the initial data for static vertex and index buffers, use 
		//! one local heap to store the data, then use one temporary upload heap to copy data to the local heap.
		//! 4. If you need to upload data from CPU side multiple times, but the data is read by GPU only once per CPU update, use upload heap.
		//! 5. If you need to upload data from CPU side multiple times, and the data will be read by GPU multiple times per CPU update, use one 
		//! local heap resource for GPU access and one upload heap resource for CPU access, and copy data between two resources when needed.
		//! 6. If you need to read resource data from CPU side, use readback heap.
		enum class ResourceHeapType : u8
		{
			//! The resource heap can only be read and written by GPU. CPU access is not allowed.
			local = 0,

			//! The resource heap can only be written by CPU and read by GPU.
			//! Only buffer resources can be created on upload heap.
			upload = 1,

			//! The resource heap can only be read by CPU and written by GPU.
			//! Only buffer resources can be created on readback heap.
			//! The buffer resource can only be used as the copy destination for GPU copy commands.
			readback = 2,
		};

		enum class Format : u16
		{
			unknown = 0,
			// Ordinary 8-Bit Formats
			r8_unorm,
			r8_snorm,
			r8_uint,
			r8_sint,
			// Ordinary 16-Bit Formats
			r16_unorm,
			r16_snorm,
			r16_uint,
			r16_sint,
			r16_float,
			rg8_unorm,
			rg8_snorm,
			rg8_uint,
			rg8_sint,
			// Ordinary 32-Bit Formats
			r32_uint,
			r32_sint,
			r32_float,
			rg16_unorm,
			rg16_snorm,
			rg16_uint,
			rg16_sint,
			rg16_float,
			rgba8_unorm,
			rgba8_unorm_srgb,
			rgba8_snorm,
			rgba8_uint,
			rgba8_sint,
			bgra8_unorm,
			bgra8_unorm_srgb,
			// Ordinary 64-Bit Formats
			rg32_uint,
			rg32_sint,
			rg32_float,
			rgba16_unorm,
			rgba16_snorm,
			rgba16_uint,
			rgba16_sint,
			rgba16_float,
			// Ordinary 96-Bit Formats
			rgb32_uint,
			rgb32_sint,
			rgb32_float,
			// Ordinary 128-Bit Formats
			rgba32_uint,
			rgba32_sint,
			rgba32_float,
			// Packed 16-Bit Formats
			b5g6r5_unorm,
			bgr5a1_unorm,
			// Packed 32-Bit Formats
			rgb10a2_unorm,
			rgb10a2_uint,
			rg11b10_float,
			rgb9e5_float,
			// Depth-stencil
			d16_unorm,
			d32_float,
			d24_unorm_s8_uint,
			d32_float_s8_uint_x24,
			// Compressed formats
			bc1_rgba_unorm,   // DXT1
			bc1_rgba_unorm_srgb,
			bc2_rgba_unorm,   // DXT3
			bc2_rgba_unorm_srgb,
			bc3_rgba_unorm,   // DXT5
			bc3_rgba_unorm_srgb,
			bc4_r_unorm,   // RGTC Unsigned Red
			bc4_r_snorm,   // RGTC Signed Red
			bc5_rg_unorm,   // RGTC Unsigned RG
			bc5_rg_snorm,   // RGTC Signed RG
			bc6h_rgb_sfloat,
			bc6h_rgb_ufloat,
			bc7_rgba_unorm,
			bc7_rgba_unorm_srgb,

			count
		};

		//! Returns the size of one pixel in the specified format, in bits.
		inline usize bits_per_pixel(Format format)
		{
			switch (format)
			{
			case Format::r8_unorm:
			case Format::r8_snorm:
			case Format::r8_uint:
			case Format::r8_sint:
				return 8;
			case Format::r16_unorm:
			case Format::r16_snorm:
			case Format::r16_uint:
			case Format::r16_sint:
			case Format::r16_float:
			case Format::rg8_unorm:
			case Format::rg8_snorm:
			case Format::rg8_uint:
			case Format::rg8_sint:
			case Format::b5g6r5_unorm:
			case Format::bgr5a1_unorm:
			case Format::d16_unorm:
				return 16;
			case Format::r32_uint:
			case Format::r32_sint:
			case Format::r32_float:
			case Format::rg16_uint:
			case Format::rg16_sint:
			case Format::rg16_unorm:
			case Format::rg16_snorm:
			case Format::rg16_float:
			case Format::rgba8_unorm:
			case Format::rgba8_unorm_srgb:
			case Format::rgba8_snorm:
			case Format::rgba8_uint:
			case Format::rgba8_sint:
			case Format::bgra8_unorm:
			case Format::bgra8_unorm_srgb:
			case Format::rgb10a2_unorm:
			case Format::rgb10a2_uint:
			case Format::rg11b10_float:
			case Format::rgb9e5_float:
			case Format::d32_float:
			case Format::d24_unorm_s8_uint:
				return 32;
			case Format::rg32_uint:
			case Format::rg32_sint:
			case Format::rg32_float:
			case Format::rgba16_unorm:
			case Format::rgba16_snorm:
			case Format::rgba16_uint:
			case Format::rgba16_sint:
			case Format::rgba16_float:
			case Format::d32_float_s8_uint_x24:
				return 64;
			case Format::rgba32_uint:
			case Format::rgba32_sint:
			case Format::rgba32_float:
				return 128;
			case Format::bc1_rgba_unorm:
			case Format::bc1_rgba_unorm_srgb:
			case Format::bc4_r_snorm:
			case Format::bc4_r_unorm:
				return 4;
			case Format::bc2_rgba_unorm:
			case Format::bc2_rgba_unorm_srgb:
			case Format::bc3_rgba_unorm:
			case Format::bc3_rgba_unorm_srgb:
			case Format::bc5_rg_snorm:
			case Format::bc5_rg_unorm:
			case Format::bc6h_rgb_sfloat:
			case Format::bc6h_rgb_ufloat:
			case Format::bc7_rgba_unorm:
			case Format::bc7_rgba_unorm_srgb:
				return 8;
			default:
				lupanic();
				return 0;
			}
		}

		//! Specify how the resource will be used.
		enum class ResourceUsageFlag : u32
		{
			none = 0x00,
			//! Allows this resource to be bound as a shader resource by graphic pipeline.
			shader_resource = 0x01,
			//! Allows this resource to be bound as a constant buffer by graphic pipeline.
			constant_buffer = 0x02,
			//! Allows this resource to be bound for unordered access by compute pipeline.
			unordered_access = 0x04,
			//! Allows this resource to be bound as a render target by graphic pipeline.
			render_target = 0x08,
			//! Allows this resource to be bound as a depth stencil target by graphic pipeline.
			depth_stencil = 0x10,
			//! Allows this resource to be bound as a vertex buffer by graphic pipeline.
			vertex_buffer = 0x20,
			//! Allows this resource to be bound as a index buffer by graphic pipeline.
			index_buffer = 0x40,
			//! Allows this resource to be bound as a streaming output buffer by graphic pipeline.
			stream_output = 0x80,
			//! Allows this resource to be bound as a buffer providing indirect draw arguments.
			indirect_buffer = 0x100,
		};

		//! Additional flags for texture and buffer.
		enum class ResourceFlag : u32
		{
			none = 0x00,
			//! Indicates that this resource represents a cubemap texture.
			cubemap = 0x01,
			//! Allow this resource to be used simultaneously from multiple command queues, given that only 
			//! one queue is writing to the resource.
			simultaneous_access = 0x02,
		};

		//! Describe an RHI resource.
		struct ResourceDesc
		{
			//! The type of the resource.
			ResourceType type;
			//! The heap type of the resource. See `ResourceHeapType` for details.
			ResourceHeapType heap_type;
			//! If the resource is a texture, specify the pixel format of the resource.
			//! If the resource is a buffer, this is ignored when creating resources, and
			//! will be set to `unknown` in its descriptor object.
			Format pixel_format;
			//! A combination of `ResourceUsageFlag` flags to indicate all possible 
			//! usages of this resource.
			ResourceUsageFlag usages;
			//! If the resource is a texture, specify the width of the texture in pixels.
			//! If the resource is a buffer, specify the size of the buffer in bytes.
			u64 width_or_buffer_size;
			//! The height of the texture in pixels.
			//! If the resource is a buffer or 1D texture, this is ignored when creating resources, and
			//! will be set to 1 in its descriptor object.
			u32 height;
			//! If the resource is a 3D texture, specifty the depth of the resource in pixels 
			//! If the resources is a 1D and 2D texture, specify the number of array slices 
			//! If the resource is a buffer, this is ignored when creating resources, and
			//! will be set to 1 in its descriptor object.
			u32 depth_or_array_size;
			//! If the resource is a texture, specify the number of mip-map slices.
			//! If the resource is a buffer, this is ignored when creating resources, and
			//! will be set to 1 in its descriptor object.
			//! Specify 0 tells the system to create full mip-map chain for the resource.
			u32 mip_levels;
			//! The sample count per pixel for multi-sample texture resource, specify 1 if the 
			//! multi-sample is disabled for this texture.
			//! If the resource is a buffer, 1D or 3D texture, this is ignored when creating resources, and
			//! will be set to 1 in its descriptor object.
			u32 sample_count;
			//! The sample quality level for multi-sample texture resource, specify 0 if the 
			//! multi-sample is disabled for this texture. Only specify non-zero values when 
			//! `sample_count` is not 1.
			//! If the resource is a buffer, 1D or 3D texture, this is ignored when creating resources, and
			//! will be set to 0 in its descriptor object.
			u32 sample_quality;
			//! Additional flags for the resource.
			ResourceFlag flags;

			static inline ResourceDesc buffer(ResourceHeapType heap_type, ResourceUsageFlag usages, u64 size, ResourceFlag flags = ResourceFlag::none)
			{
				ResourceDesc d;
				d.type = ResourceType::buffer;
				d.heap_type = heap_type;
				d.usages = usages;
				d.width_or_buffer_size = size;
				d.flags = flags;
				return d;
			}

			static inline ResourceDesc tex1d(ResourceHeapType heap_type, Format pixel_format, ResourceUsageFlag usages, u64 width, u32 array_size = 1, u32 mip_levels = 0, ResourceFlag flags = ResourceFlag::none)
			{
				ResourceDesc d;
				d.type = ResourceType::texture_1d;
				d.heap_type = heap_type;
				d.pixel_format = pixel_format;
				d.usages = usages;
				d.width_or_buffer_size = width;
				d.sample_count = 1;
				d.sample_quality = 0;
				d.depth_or_array_size = array_size;
				d.mip_levels = mip_levels;
				d.flags = flags;
				return d;
			}

			static inline ResourceDesc tex2d(ResourceHeapType heap_type, Format pixel_format, ResourceUsageFlag usages, u64 width, u32 height, u32 array_size = 1, u32 mip_levels = 0,
				u32 sample_count = 1, u32 sample_quality = 0, ResourceFlag flags = ResourceFlag::none)
			{
				ResourceDesc d;
				d.type = ResourceType::texture_2d;
				d.heap_type = heap_type;
				d.pixel_format = pixel_format;
				d.usages = usages;
				d.width_or_buffer_size = width;
				d.height = height;
				d.sample_count = sample_count;
				d.sample_quality = sample_quality;
				d.depth_or_array_size = array_size;
				d.mip_levels = mip_levels;
				d.flags = flags;
				return d;
			}

			static inline ResourceDesc tex3d(ResourceHeapType heap_type, Format pixel_format, ResourceUsageFlag usages, u64 width, u32 height, u32 depth, u32 mip_levels = 0, ResourceFlag flags = ResourceFlag::none)
			{
				ResourceDesc d;
				d.type = ResourceType::texture_3d;
				d.heap_type = heap_type;
				d.pixel_format = pixel_format;
				d.usages = usages;
				d.width_or_buffer_size = width;
				d.height = height;
				d.sample_count = 1;
				d.sample_quality = 0;
				d.depth_or_array_size = depth;
				d.mip_levels = mip_levels;
				d.flags = flags;
				return d;
			}
		};

		struct DepthStencilValue
		{
			f32 depth;
			u8 stencil;
		};

		enum class ClearValueType : u32
		{
			color = 1,
			depth_stencil = 2
		};

		struct ClearValue
		{
			Format pixel_format;
			ClearValueType type;
			union
			{
				f32 color[4];
				DepthStencilValue depth_stencil;
			};
			static ClearValue as_color(Format pixel_format, f32 color[4])
			{
				ClearValue r;
				r.pixel_format = pixel_format;
				r.type = ClearValueType::color;
				r.color[0] = color[0];
				r.color[1] = color[1];
				r.color[2] = color[2];
				r.color[3] = color[3];
				return r;
			}
			static ClearValue as_depth_stencil(Format pixel_format, f32 depth, u8 stencil)
			{
				ClearValue r;
				r.type = ClearValueType::depth_stencil;
				r.pixel_format = pixel_format;
				r.depth_stencil.depth = depth;
				r.depth_stencil.stencil = stencil;
				return r;
			}
		};

		//! @interface IResource
		//! Represents a memory region that can be accessed by GPU.
		struct IResource : virtual IDeviceChild
		{
			luiid("{D67C47CD-1FF3-4FA4-82FE-773EC5C8AD2A}");

			//! Gets the descriptor of the resource.
			virtual ResourceDesc get_desc() = 0;

			//! Maps the resource data to system memory and enables CPU access to the resource data.
			//! Map/unmap operations are reference counted, for each `map_subresource` operation, you need to call `unmap_subresource` once to finally unmap the memory.
			//! @param[in] subresource The index of the subresource you want to map. For buffer resources, always specify `0`.
			//! @param[in] read_begin The byte offset of the beginning of the data range that will be read by CPU.
			//! @param[in] read_end The byte offset of the ending of the data range that will be read by CPU.
			//! 
			//! If `read_end <= read_begin`, no data will be read by CPU, which is required if resource heap type is not `ResourceHeapType::readback`.
			//! 
			//! If `read_end` is larger than the subresource size (like setting to `USIZE_MAX`), the read range will be clamped to [read_begin, resource_size). 
			//! @param[out] out_data If not `nullptr`, returns one pointer to the mapped resource data. 
			//! The returned pointer is not affected by `read_offset` and always points to the beginning of the subresource data.
			virtual RV map_subresource(u32 subresource, usize read_begin, usize read_end, void** out_data) = 0;

			//! Invalidates the pointer to the mapped data, and synchronizes changed data when needed.
			//! Map/unmap operations are reference counted, for each `map_subresource` operation, you need to call `unmap_subresource` once to finally unmap the memory.
			//! @param[in] subresource The index of the subresource you want to unmap. For buffer resources, always specify `0`.
			//! @param[in] write_begin The byte offset of the beginning of the data range that is changed by CPU and should be synchronized. 
			//! @param[in] write_end The byte offset of the ending of the data range that is changed by CPU and should be synchronized.
			//! 
			//! If `write_begin <= write_end`, no data will be synchronized, which is required if resource heap type is not `ResourceHeapType::upload`.
			//! 
			//! If `write_end` is larger than the subresource size (like setting to `USIZE_MAX`), the write range will be clamped to [write_begin, resource_size). 
			virtual void unmap_subresource(u32 subresource, usize write_begin, usize write_end) = 0;
		};
	}
}