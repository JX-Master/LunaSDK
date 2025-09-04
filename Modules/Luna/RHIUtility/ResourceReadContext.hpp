/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceReadContext.hpp
* @author JXMaster
* @date 2025/9/4
*/
#pragma once
#include <Luna/RHI/Buffer.hpp>
#include <Luna/RHI/Texture.hpp>
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_RHI_UTILITY_API
#define LUNA_RHI_UTILITY_API
#endif

namespace Luna
{
    namespace RHIUtility
    {
        //! @addtogroup RHIUtility
        //! @{

        //! The utility object to copy data from resource memory to host memory using a temporary readback buffer.
        struct IResourceReadContext : virtual RHI::IDeviceChild
        {
            luiid("31ed53a5-f64c-4d5d-963f-f0cf4566e266");
            
            //! Resets this context to empty state.
            //! @remarks This clears all recorded copy operations and data, but retains resources allocated so that
            //! they can be reused for a new transmission batch.
            //! @par Valid Usage
            //! 1. This function should only be called when no command buffer is using data from this copy context 
            //! (before @ref commit is called or after the command buffer is finished executing).
            virtual void reset() = 0;

            //! Copy data of one buffer resource from resource memory to host memory.
            //! @param[in] buffer The buffer resource to copy data from.
            //! @param[in] offset The starting offset to copy, in bytes, from the start of `buffer`.
            //! @param[in] size The number of bytes to copy.
            //! @return Returns one handle that can be passed into @ref get_buffer_data to get the real buffer data pointer after
            //! copy is finished.
            virtual usize read_buffer(RHI::IBuffer* buffer, u64 offset, usize size) = 0;

            //! Copy data of one texture resource from resource memory to host memory.
            //! @param[in] texture The texture resource to copy data from.
            //! @param[in] subresource The index of the subresource in `texture` to copy data from.
            //! @param[in] x The X position of the first pixel in `texture` to copy from.
            //! @param[in] y The Y position of the first pixel in `texture` to copy from.
            //! @param[in] z The Z position of the first pixel in `texture` to copy from.
            //! @param[in] width The number of pixels to copy for every row.
            //! @param[in] height The number of rows to copy.
            //! @param[in] depth The number of slices to copy.
            //! @return Returns one handle that can be passed into @ref get_texture_data to get the real texture data pointer after
            //! copy is finished.
            virtual usize read_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
                u32 x, u32 y, u32 z, 
                u32 width, u32 height, u32 depth) = 0;

            //! Commits data copy calls to the target command buffer.
            //! @param[in] command_buffer The command buffer used to perform the data copy operation. 
            //! @param[in] submit_and_wait If this is `true`, the command buffer will be submitted, synchronized
            //! and reset before this function returns. Otherwise, this function only commits data copy calls to the 
            //! command buffer, and the user should submit and wait for the command buffer manually. The later case is 
            //! useful if the user wants to commit multiple data copy contexts and submit them all together.
            virtual RV commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait) = 0;

            //! Gets the copied buffer data.
            //! @param[in] handle The handle returned by the @ref read_buffer for the operation to query.
            //! @return Returns one pointer to the read buffer data. The pointer is valid until @ref reset is called.
            //! @par Valid Usage
            //! 1. This function should only be called after @ref commit and before @ref reset. If `submit_and_wait` is `false`,
            //! this function should only be called after all copy commands are finished.
            virtual const void* get_buffer_data(usize handle) = 0;
            
            //! Gets the copied texture data.
            //! @param[in] handle The handle returned by the @ref read_texture for the operation to query.
            //! @param[out] out_row_pitch Returns the stride, in bytes, to advance between every 2 rows of data in the returned pointer.
            //! @param[out] out_slide_pitch Returns the stride, in bytes, to advance between every 2 slices (row * column) of data in the returned pointer.
            //! @return Returns one pointer to the read texture data. Texture data is arranged in row-major order. The pointer is valid until @ref reset is called.
            //! @par Valid Usage
            //! 1. This function should only be called after @ref commit and before @ref reset. If `submit_and_wait` is `false`,
            //! this function should only be called after all copy commands are finished.
            virtual const void* get_texture_data(usize handle, u32& out_row_pitch, u32& out_slide_pitch) = 0;
        };

        //! Creates a new resource read context.
        //! @param[in] device The device this read context is created for.
        //! @return Returns the created context.
        LUNA_RHI_UTILITY_API Ref<IResourceReadContext> new_resource_read_context(RHI::IDevice* device);

        //! @}
    }
}