/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceWriteContext.hpp
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

        //! The utility object to copy data from host memory to resource memory using a temporary upload buffer.
        struct IResourceWriteContext : virtual RHI::IDeviceChild
        {
            luiid("12af1af4-a369-425e-a088-ec2ca8b66535");

            //! Resets this context to empty state.
            //! @remarks This clears all recorded copy operations and data, but retains resources allocated so that
            //! they can be reused for a new transmission batch.
            //! @par Valid Usage
            //! 1. This function should only be called when no command buffer is using data from this copy context 
            //! (before @ref commit is called or after the command buffer is finished executing).
            virtual void reset() = 0;

            //! Copy data of one buffer resource from host memory to resource memory.
            //! @param[in] buffer The buffer resource to copy data to.
            //! @param[in] offset The starting offset to copy, in bytes, from the start of `buffer`.
            //! @param[in] size The number of bytes to copy.
            //! @return Returns the pointer to the internal buffer that the user can write data to.
            //! The user should not read data from this buffer.
            virtual R<void*> write_buffer(RHI::IBuffer* buffer, u64 offset, usize size) = 0;

            //! Copy data of one texture resource from host memory to resource memory.
            //! @param[in] texture The texture resource to copy data to.
            //! @param[in] subresource The index of the subresource in `texture` to copy data to.
            //! @param[in] x The X position of the first pixel in `texture` to copy to.
            //! @param[in] y The Y position of the first pixel in `texture` to copy to.
            //! @param[in] z The Z position of the first pixel in `texture` to copy to.
            //! @param[in] width The number of pixels to copy for every row.
            //! @param[in] height The number of rows to copy.
            //! @param[in] depth The number of slices to copy.
            //! @param[in] out_row_pitch Returns the stride, in bytes, to advance between every 2 rows of data in `out_memory`.
            //! @param[in] out_slice_pitch Returns the stride, in bytes, to advance between every 2 slices (row * column) of data in `out_memory`.
            //! @return Returns the pointer to the internal buffer that the user can write data to. The texture data should be arranged in row-major order. 
            //! @par Valid Usage
            //! 1. `src` must points to a valid memory until the copy operation is finished.
            virtual R<void*> write_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
                u32 x, u32 y, u32 z,
                u32 width, u32 height, u32 depth,
                u32& out_row_pitch, u32& out_slice_pitch) = 0;

            //! Commits data copy calls to the target command buffer.
            //! @param[in] command_buffer The command buffer used to perform the data copy operation. 
            //! @param[in] submit_and_wait If this is `true`, the command buffer will be submitted, synchronized
            //! and reset before this function returns. Otherwise, this function only commits data copy calls to the 
            //! command buffer, and the user should submit and wait for the command buffer manually. The later case is 
            //! useful if the user wants to commit multiple data copy contexts and submit them all together.
            virtual RV commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait) = 0;
        };

        //! Creates a new resource write context.
        //! @param[in] device The device this write context is created for.
        //! @return Returns the created context.
        LUNA_RHI_UTILITY_API Ref<IResourceWriteContext> new_resource_write_context(RHI::IDevice* device);

        //! @}
    }
}