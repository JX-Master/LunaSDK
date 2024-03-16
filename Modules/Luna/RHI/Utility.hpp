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
        //! @addtogroup RHI
        //! @{
        
        //! Specifies the type of one resource data copy operation.
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

        //! Specifies one resource data copy operation.
        struct CopyResourceData
        {
            //! The copy operation to perform.
            ResourceDataCopyOp op;
            struct ReadBufferDesc
            {
                //! The host memory to copy data to.
                void* dst;
                //! The buffer resource to copy data from.
                RHI::IBuffer* src;
                //! The starting offset to copy, in bytes, from the start of `src` buffer.
                u64 src_offset;
                //! The number of bytes to copy.
                usize copy_size;
            };
            struct WriteBufferDesc
            {
                //! The buffer resource to copy data to.
                RHI::IBuffer* dst;
                //! The host memory to copy data from.
                const void* src;
                //! The starting offset to copy, in bytes, from the start of `dst` buffer.
                u64 dst_offset;
                //! The number of bytes to copy.
                usize copy_size;
            };
            struct ReadTextureDesc
            {
                //! The host memory to copy data to.
                void* dst;
                //! The texture resource to copy data from.
                ITexture* src;
                //! The stride, in bytes, to advance between every 2 rows of data in `dst`.
                u32 dst_row_pitch;
                //! The stride, in bytes, to advance between every 2 slices (row * column) of data in `dst`.
                u32 dst_slice_pitch;
                //! The index of the subresource in `src` to copy data from.
                SubresourceIndex src_subresource;
                //! The X position of the first pixel in `src` to copy from.
                u32 src_x;
                //! The Y position of the first pixel in `src` to copy from.
                u32 src_y;
                //! The Z position of the first pixel in `src` to copy from.
                u32 src_z;
                //! The number of pixels to copy for every row.
                u32 copy_width;
                //! The number of rows to copy.
                u32 copy_height;
                //! The number of slices to copy.
                u32 copy_depth;
            };
            struct WriteTextureDesc
            {
                //! The texture resource to copy data to.
                ITexture* dst;
                //! The host memory to copy data from.
                const void* src;
                //! The stride, in bytes, to advance between every 2 rows of data in `src`.
                u32 src_row_pitch;
                //! The stride, in bytes, to advance between every 2 slices (row * column) of data in `src`.
                u32 src_slice_pitch;
                //! The index of the subresource in `dst` to copy data to.
                SubresourceIndex dst_subresource;
                //! The X position of the first pixel in `dst` to copy to.
                u32 dst_x;
                //! The Y position of the first pixel in `dst` to copy to.
                u32 dst_y;
                //! The Z position of the first pixel in `dst` to copy to.
                u32 dst_z;
                //! The number of pixels to copy for every row.
                u32 copy_width;
                //! The number of rows to copy.
                u32 copy_height;
                //! The number of slices to copy.
                u32 copy_depth;
            };
            union
            {
                //! Describes the copy operation if `op` is @ref ResourceDataCopyOp::read_buffer.
                ReadBufferDesc read_buffer_desc;
                //! Describes the copy operation if `op` is @ref ResourceDataCopyOp::write_buffer.
                WriteBufferDesc write_buffer_desc;
                //! Describes the copy operation if `op` is @ref ResourceDataCopyOp::read_texture.
                ReadTextureDesc read_texture_desc;
                //! Describes the copy operation if `op` is @ref ResourceDataCopyOp::write_texture.
                WriteTextureDesc write_texture_desc;
            };
            //! Creates one resource data copy operation that copied data from buffer resource to host memory.
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
            //! Creates one resource data copy operation that copied data from host memory to buffer resource.
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
            //! Creates one resource data copy operation that copied data from texture resource to host memory.
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
            //! Creates one resource data copy operation that copied data from host memory to texture resource.
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

        //! Copies data between host memory and resource memory.
        //! @details The system allocates one staging buffer for the copy internally.
        //! @param[in] command_buffer The command buffer used to perform the data copy operation. The command buffer will be submitted, synchronized
        //! and reset before this function returns.
        //! @param[in] copies A number of copy operations that should be performed. 
        //! The user should batch copy operations into one `copy_resource_data` call as much as possible to reduce memory allocation 
        //! and synchronization overhead.
        LUNA_RHI_API RV copy_resource_data(ICommandBuffer* command_buffer, Span<const CopyResourceData> copies);

        //! @}
    }
}