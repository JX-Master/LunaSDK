/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Utility.hpp
* @author JXMaster
* @date 2023/6/8
* @brief The utility library of RHI contains high-level functions implemented using RHI API for common tasks.
*/
#pragma once
#include "Resource.hpp"
#include "CommandBuffer.hpp"
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif
namespace Luna
{
    namespace RHI
    {
        enum class ResourceDataCopyOp : u8
		{
			//! Copy data of one buffer resource from resource memory to host memory.
			read_buffer,
			//! Copy data of one buffer resource from host memory to resource memory.
			write_buffer,
			//! Copy data of one texture resource from resource memory to host memory.
			read_texture,
			//! Copy data of one texture resource from host memory to resource memory.
			write_texture
		};
        struct CopyResourceData
        {
            ResourceDataCopyOp op;
            struct ReadBufferDesc
            {
                void* dst;
                RHI::IBuffer* src;
                u64 src_offset;
                usize copy_size;
            };
            struct WriteBufferDesc
            {
                RHI::IBuffer* dst;
                const void* src;
                u64 dst_offset;
                usize copy_size;
            };
            struct ReadTextureDesc
            {
                void* dst;
                ITexture* src;
                u32 dst_row_pitch;
                u32 dst_slice_pitch;
                SubresourceIndex src_subresource;
                u32 src_x;
                u32 src_y;
                u32 src_z;
                u32 copy_width;
                u32 copy_height;
                u32 copy_depth;
            };
            struct WriteTextureDesc
            {
                ITexture* dst;
                const void* src;
                u32 src_row_pitch;
                u32 src_slice_pitch;
                SubresourceIndex dst_subresource;
                u32 dst_x;
                u32 dst_y;
                u32 dst_z;
                u32 copy_width;
                u32 copy_height;
                u32 copy_depth;
            };
            union
            {
                ReadBufferDesc read_buffer_desc;
                WriteBufferDesc write_buffer_desc;
                ReadTextureDesc read_texture_desc;
                WriteTextureDesc write_texture_desc;
            };
            static CopyResourceData read_buffer(void* dst, RHI::IBuffer* src, u64 src_offset, usize copy_size)
            {
                CopyResourceData r;
                r.op = ResourceDataCopyOp::read_buffer;
                r.read_buffer_desc.dst = dst;
                r.read_buffer_desc.src = src;
                r.read_buffer_desc.src_offset = src_offset;
                r.read_buffer_desc.copy_size = copy_size;
                return r;
            }
            static CopyResourceData write_buffer(RHI::IBuffer* dst, u64 dst_offset, const void* src, usize copy_size)
            {
                CopyResourceData r;
                r.op = ResourceDataCopyOp::write_buffer;
                r.write_buffer_desc.dst = dst;
                r.write_buffer_desc.dst_offset = dst_offset;
                r.write_buffer_desc.src = src;
                r.write_buffer_desc.copy_size = copy_size;
                return r;
            }
            static CopyResourceData read_texture(void* dst, u32 dst_row_pitch, u32 dst_slice_pitch, 
                ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z, 
                u32 copy_width, u32 copy_height, u32 copy_depth)
            {
                CopyResourceData r;
                r.op = ResourceDataCopyOp::read_texture;
                r.read_texture_desc.dst = dst;
                r.read_texture_desc.dst_row_pitch = dst_row_pitch;
                r.read_texture_desc.dst_slice_pitch = dst_slice_pitch;
                r.read_texture_desc.src = src;
                r.read_texture_desc.src_subresource = src_subresource;
                r.read_texture_desc.src_x = src_x;
                r.read_texture_desc.src_y = src_y;
                r.read_texture_desc.src_z = src_z;
                r.read_texture_desc.copy_width = copy_width;
                r.read_texture_desc.copy_height = copy_height;
                r.read_texture_desc.copy_depth = copy_depth;
                return r;
            }
            static CopyResourceData write_texture(ITexture* dst, SubresourceIndex dst_subresource, 
                u32 dst_x, u32 dst_y, u32 dst_z,
                const void* src, u32 src_row_pitch, u32 src_slice_pitch,
                u32 copy_width, u32 copy_height, u32 copy_depth)
            {
                CopyResourceData r;
                r.op = ResourceDataCopyOp::write_texture;
                r.write_texture_desc.dst = dst;
                r.write_texture_desc.dst_subresource = dst_subresource;
                r.write_texture_desc.dst_x = dst_x;
                r.write_texture_desc.dst_y = dst_y;
                r.write_texture_desc.dst_z = dst_z;
                r.write_texture_desc.src = src;
                r.write_texture_desc.src_row_pitch = src_row_pitch;
                r.write_texture_desc.src_slice_pitch = src_slice_pitch;
                r.write_texture_desc.copy_width = copy_width;
                r.write_texture_desc.copy_height = copy_height;
                r.write_texture_desc.copy_depth = copy_depth;
                return r;
            }
        };

        //! Copies buffer data from host memory to device local memory.
        //! The system allocates one staging buffer for the copy internally.
        LUNA_RHI_API RV copy_resource_data(ICommandBuffer* command_buffer, Span<const CopyResourceData> copies);
    }
}