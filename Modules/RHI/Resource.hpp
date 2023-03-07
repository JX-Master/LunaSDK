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

		//! Specify which heap the resource should be created in.
		enum class ResourceHeapType : u8
		{
			//! The resource is optimized for maximum GPU bandwidth. CPU access is disabled.
			//! For non-UMA devices, specifying this causes the resource to be allocated on physical video memory, such 
			//! memory cannot be accessed by CPU.
			//! For UMA devices, the resource is allocated on physical system memory, but CPU access is disabled.
			local = 0,

			//! The resource is optimized for maximum GPU bandwidth, but can be accessed by both CPU and GPU.
			//! For non-UMA devices, specifying this causes the resource to be allocated on physical video memory, just as `local` does.
			//! When CPU maps the resource data, GPU copies data from video memory to system memory and exposes them to CPU; when CPU unmaps the resource data, 
			//! GPU copies the data on system memory back to video memory. The copy operation is costly and has high latency, so they should not be done very often.
			//! For UMA devices, the resource is allocated on physical system memory, CPU can access the it just like normal memory. No data copy
			//! is occured in such case.
			shared = 1,

			//! Same as `shared`, but CPU can only writes to the resource, never reads from it. Specify this instead of `shared`
			//! gives the drive more room to optimize CPU bandwidth, such as using write-combined CPU cache mode.
			shared_upload = 2,

			//! The resource is created in a heap optimized for CPU writing. GPU bandwidth is limited.
			//! The resource will be created in physical system memory, which does not require any data copy on mapping and unmapping resources.
			//! On non-UMA devices, GPU will read the system memory through PCI-E bus, which limits the bandwidth for GPU.
			//! Only buffer resources can be created on upload heap, CPU cannot read data from such resources, and GPU cannot write to such resources.
			upload = 3,

			//! The resource is created in a heap optimized for CPU reading. GPU bandwidth is limited.
			//! The resource will be created in physical system memory, which does not require any data copy on mapping and unmapping resources.
			//! On non-UMA devices, GPU will write to the system memory through PCI-E bus, which limits the bandwidth for GPU.
			//! Only buffer resources can be created on readback heap, such resource can only be used as the copy destination for GPU copy commands.
			readback = 4,
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
			//! Specify 0 tells the system to create full mip-map chain for the resource.
			u32 depth_or_array_size;
			//! If the resource is a texture, specify the number of mip-map slices.
			//! If the resource is a buffer, this is ignored when creating resources, and
			//! will be set to 1 in its descriptor object.
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

			//! Maps the resource data to system memory and gets a pointer to the buffer in order to enable CPU read/write.
			//! Map/unmap operations are reference counted, for each `map_subresource` operation, you need to call `unmap_subresource` once to finally unmap the memory.
			//! @param[in] subresource The index of the subresource you want to map. For buffer resources, always specify `0`.
			//! @param[in] load_data Whether the mapped memory should be initialized by the subresource data.
			//! This can only be `true` for resources created on `shared`, `shared_update` and `readback` heaps.
			//! 
			//! On non-UMA devices, for resources created on `shared` and `shared_update` heaps, specifying `true` causes the system to 
			//! copy data from video memory to system memory in order to be read by CPU.
			//! 
			//! `load_data` is only used for the first mapping call, subsequent mapping calls only increases the reference count, thus ignores this argument.
			//! 
			//! For best performance, specify `true` only when you do need to read data from video memory.
			//! @param[out] out_data If not `nullptr`, returns one pointer to the mapped resource data.
			//! The data pointer can only be fetched if the mapped resource is a buffer. For textures, the user must set `out_data` to `nullptr`, then
			//! use `read_subresource` and `write_subresource` to read and write texture data.
			virtual RV map_subresource(u32 subresource, bool load_data, void** out_data = nullptr) = 0;

			//! Invalidates the pointer to the mapped data, and synchronizes change data with GPU when needed.
			//! Map/unmap operations are reference counted, for each `map_subresource` operation, you need to call `unmap_subresource` once to finally unmap the memory.
			//! @param[in] subresource The index of the subresource you want to unmap. For buffer resources, always specify `0`.
			//! @param[in] store_data Whether the data in the mapped memory should be synchronized with (copied to) GPU.
			//! This can only be `true` for resources created on `shared`, `shared_update` and `upload` heaps.
			//! 
			//! On non-UMA devices, for resources created on `shared` and `shared_update` heaps, specifying `true` causes the system to 
			//! copy data from system memory back to video memory in order to be accessed by GPU.
			//! 
			//! `store_data` is only used for the last unmapping call, former unmapping calls only decreases the reference count, thus ignores this argument.
			//!
			//! For best performance, specify `true` only when you do need to write data to video memory.
			virtual void unmap_subresource(u32 subresource, bool store_data) = 0;
		
			//! Read data from subresource to the address specified.
			//! The resource must be mapped first by calling `map_subresource`. No GPU queue should use the resource when the resource is mapped to the 
			//! CPU, and the resource should be transferred to `EResourceState::common` state exlicitly or imexplicitly from the last command list.
			//! @param[in] dest A pointer to the memory that the data will be written to.
			//! @param[in] dest_row_pitch The offset between every row of destination buffer.
			//! @param[in] dest_depth_pitch The offset between every layer of destination buffer.
			//! @param[in] subresource The index of the subresource to operate.
			//! @param[in] read_box Specify the read offset in width, height and depth, and the size to read in width, height and depth.
			virtual RV read_subresource(void* dest, u32 dest_row_pitch, u32 dest_depth_pitch, u32 subresource, const BoxU& read_box) = 0;

			//! Write data from the address specified to the resource.
			//! The resource must be mapped first by calling `map_subresource`. No GPU queue should use the resource when the resource is mapped to the 
			//! CPU, and the resource should be transferred to `EResourceState::common` state exlicitly or imexplicitly from the last command list.
			//! @param[in] subresource The index of the subresource to operate.
			//! @param[in] src A pointer to the source data.
			//! @param[in] src_row_pitch The offset between every row of source data.
			//! @param[in] src_depth_pitch The offset between every layer of source data.
			//! @param[in] write_box Specify the write offset in width, height and depth, and the size to write in width, height and depth.
			virtual RV write_subresource(u32 subresource, const void* src, u32 src_row_pitch, u32 src_depth_pitch, const BoxU& write_box) = 0;
		};
	}
}