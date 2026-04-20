#include "RHIUtility.h"

#include <Luna/RHI/Buffer.hpp>
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Texture.hpp>
#include <Luna/RHIUtility/BlitContext.hpp>
#include <Luna/RHIUtility/MipmapGenerationContext.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/RHIUtility/ResourceReadContext.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Runtime/MemoryUtils.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

Luna::RHI::TextureViewDesc to_texture_view_desc(const LunaRhiTextureViewDesc& desc)
{
    return Luna::RHI::TextureViewDesc(
        static_cast<Luna::RHI::ITexture*>(desc.texture),
        static_cast<Luna::RHI::TextureViewType>(desc.type),
        static_cast<Luna::RHI::Format>(desc.format),
        desc.mip_slice,
        desc.mip_size,
        desc.array_slice,
        desc.array_size);
}

Luna::RHI::SamplerDesc to_sampler_desc(const LunaRhiSamplerDesc& desc)
{
    return Luna::RHI::SamplerDesc(
        static_cast<Luna::RHI::Filter>(desc.min_filter),
        static_cast<Luna::RHI::Filter>(desc.mag_filter),
        static_cast<Luna::RHI::Filter>(desc.mip_filter),
        static_cast<Luna::RHI::TextureAddressMode>(desc.address_u),
        static_cast<Luna::RHI::TextureAddressMode>(desc.address_v),
        static_cast<Luna::RHI::TextureAddressMode>(desc.address_w),
        desc.anisotropy_enable != 0,
        desc.max_anisotropy,
        static_cast<Luna::RHI::BorderColor>(desc.border_color),
        desc.min_lod,
        desc.max_lod,
        desc.compare_enable != 0,
        static_cast<Luna::RHI::CompareFunction>(desc.compare_function));
}
}

extern "C"
{
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_init_module(void)
{
    Luna::Module* module = Luna::module_rhi_utility();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_resource_write_context(
    luna_handle_t device_object,
    LunaRhiUtilityResourceWriteContextHandle* out_context)
{
    if (!device_object || !out_context)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_context->object = nullptr;
    out_context->iresource_write_context = nullptr;

    Luna::RHI::IDevice* device = Luna::query_interface<Luna::RHI::IDevice>(device_object);
    if (!device)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::Ref<Luna::RHIUtility::IResourceWriteContext> context = Luna::RHIUtility::new_resource_write_context(device);
    Luna::object_t object = context.detach();
    void* iresource_write_context = Luna::query_interface<Luna::RHIUtility::IResourceWriteContext>(object);
    if (!iresource_write_context)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_context->object = object;
    out_context->iresource_write_context = iresource_write_context;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_resource_read_context(
    luna_handle_t device_object,
    LunaRhiUtilityResourceReadContextHandle* out_context)
{
    if (!device_object || !out_context)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_context->object = nullptr;
    out_context->iresource_read_context = nullptr;

    Luna::RHI::IDevice* device = Luna::query_interface<Luna::RHI::IDevice>(device_object);
    if (!device)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::Ref<Luna::RHIUtility::IResourceReadContext> context = Luna::RHIUtility::new_resource_read_context(device);
    Luna::object_t object = context.detach();
    void* iresource_read_context = Luna::query_interface<Luna::RHIUtility::IResourceReadContext>(object);
    if (!iresource_read_context)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_context->object = object;
    out_context->iresource_read_context = iresource_read_context;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_blit_context(
    luna_handle_t device_object,
    uint32_t destination_format,
    LunaRhiUtilityBlitContextHandle* out_context)
{
    if (!device_object || !out_context)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_context->object = nullptr;
    out_context->iblit_context = nullptr;

    Luna::RHI::IDevice* device = Luna::query_interface<Luna::RHI::IDevice>(device_object);
    if (!device)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    auto result = Luna::RHIUtility::new_blit_context(device, static_cast<Luna::RHI::Format>(destination_format));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHIUtility::IBlitContext> context = Luna::move(result.get());
    Luna::object_t object = context.detach();
    void* iblit_context = Luna::query_interface<Luna::RHIUtility::IBlitContext>(object);
    if (!iblit_context)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_context->object = object;
    out_context->iblit_context = iblit_context;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_mipmap_generation_context(
    luna_handle_t device_object,
    LunaRhiUtilityMipmapGenerationContextHandle* out_context)
{
    if (!device_object || !out_context)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_context->object = nullptr;
    out_context->imipmap_generation_context = nullptr;

    Luna::RHI::IDevice* device = Luna::query_interface<Luna::RHI::IDevice>(device_object);
    if (!device)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    auto result = Luna::RHIUtility::new_mipmap_generation_context(device);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHIUtility::IMipmapGenerationContext> context = Luna::move(result.get());
    Luna::object_t object = context.detach();
    void* imipmap_generation_context = Luna::query_interface<Luna::RHIUtility::IMipmapGenerationContext>(object);
    if (!imipmap_generation_context)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_context->object = object;
    out_context->imipmap_generation_context = imipmap_generation_context;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_reset(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHIUtility::IResourceWriteContext*>(self)->reset();
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_write_buffer(
    void* self,
    luna_handle_t buffer_object,
    uint64_t offset,
    const void* data,
    uint64_t data_size)
{
    if (!self || !buffer_object || (data_size && !data))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IBuffer* buffer = Luna::query_interface<Luna::RHI::IBuffer>(buffer_object);
    if (!buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    auto result = static_cast<Luna::RHIUtility::IResourceWriteContext*>(self)->write_buffer(
        buffer,
        offset,
        static_cast<Luna::usize>(data_size));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    if (data_size)
    {
        memcpy(result.get(), data, static_cast<Luna::usize>(data_size));
    }
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_write_texture(
    void* self,
    luna_handle_t texture_object,
    LunaRhiSubresourceIndex subresource,
    uint32_t x,
    uint32_t y,
    uint32_t z,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    const void* data,
    uint64_t data_size,
    uint32_t source_row_pitch,
    uint32_t source_slice_pitch,
    uint32_t copy_bytes_per_row,
    LunaRhiUtilityTextureWriteInfo* out_info)
{
    if (!self || !texture_object || !out_info || !width || !height || !depth || !source_row_pitch || !copy_bytes_per_row || !data)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ITexture* texture = Luna::query_interface<Luna::RHI::ITexture>(texture_object);
    if (!texture)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    uint32_t effective_source_slice_pitch = source_slice_pitch ? source_slice_pitch : source_row_pitch * height;
    uint64_t required_size =
        static_cast<uint64_t>(effective_source_slice_pitch) * (depth - 1) +
        static_cast<uint64_t>(source_row_pitch) * (height - 1) +
        copy_bytes_per_row;
    if (data_size < required_size)
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    uint32_t row_pitch = 0;
    uint32_t slice_pitch = 0;
    auto result = static_cast<Luna::RHIUtility::IResourceWriteContext*>(self)->write_texture(
        texture,
        Luna::RHI::SubresourceIndex(subresource.mip_slice, subresource.array_slice),
        x,
        y,
        z,
        width,
        height,
        depth,
        row_pitch,
        slice_pitch);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::memcpy_bitmap3d(
        result.get(),
        data,
        copy_bytes_per_row,
        height,
        depth,
        row_pitch,
        source_row_pitch,
        slice_pitch,
        effective_source_slice_pitch);
    out_info->row_pitch = row_pitch;
    out_info->slice_pitch = slice_pitch;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait)
{
    if (!self || !command_buffer_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ICommandBuffer* command_buffer = Luna::query_interface<Luna::RHI::ICommandBuffer>(command_buffer_object);
    if (!command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RV result = static_cast<Luna::RHIUtility::IResourceWriteContext*>(self)->commit(command_buffer, submit_and_wait != 0);
    return from_result(result);
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_reset(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->reset();
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_read_buffer(
    void* self,
    luna_handle_t buffer_object,
    uint64_t offset,
    uint64_t data_size,
    uint64_t* out_handle)
{
    if (!self || !buffer_object || !out_handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IBuffer* buffer = Luna::query_interface<Luna::RHI::IBuffer>(buffer_object);
    if (!buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    *out_handle = static_cast<uint64_t>(static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->read_buffer(
        buffer,
        offset,
        static_cast<Luna::usize>(data_size)));
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_read_texture(
    void* self,
    luna_handle_t texture_object,
    LunaRhiSubresourceIndex subresource,
    uint32_t x,
    uint32_t y,
    uint32_t z,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint64_t* out_handle)
{
    if (!self || !texture_object || !width || !height || !depth || !out_handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ITexture* texture = Luna::query_interface<Luna::RHI::ITexture>(texture_object);
    if (!texture)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    *out_handle = static_cast<uint64_t>(static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->read_texture(
        texture,
        Luna::RHI::SubresourceIndex(subresource.mip_slice, subresource.array_slice),
        x,
        y,
        z,
        width,
        height,
        depth));
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait)
{
    if (!self || !command_buffer_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ICommandBuffer* command_buffer = Luna::query_interface<Luna::RHI::ICommandBuffer>(command_buffer_object);
    if (!command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RV result = static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->commit(command_buffer, submit_and_wait != 0);
    return from_result(result);
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_get_buffer_data(
    void* self,
    uint64_t handle,
    void* out_data,
    uint64_t data_size)
{
    if (!self || (data_size && !out_data))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    if (!data_size)
    {
        return 0;
    }

    auto result = static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->get_buffer_data(static_cast<Luna::usize>(handle));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    memcpy(out_data, result.get(), static_cast<Luna::usize>(data_size));
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_get_texture_data(
    void* self,
    uint64_t handle,
    uint32_t copy_bytes_per_row,
    uint32_t height,
    uint32_t depth,
    void* out_data,
    uint64_t data_size,
    LunaRhiUtilityTextureReadInfo* out_info)
{
    if (!self || !copy_bytes_per_row || !height || !depth || (data_size && !out_data) || !out_info)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    uint64_t required_size = static_cast<uint64_t>(copy_bytes_per_row) * height * depth;
    if (data_size < required_size)
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    uint32_t row_pitch = 0;
    uint32_t slice_pitch = 0;
    auto result = static_cast<Luna::RHIUtility::IResourceReadContext*>(self)->get_texture_data(
        static_cast<Luna::usize>(handle),
        row_pitch,
        slice_pitch);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::memcpy_bitmap3d(
        out_data,
        result.get(),
        copy_bytes_per_row,
        height,
        depth,
        copy_bytes_per_row,
        row_pitch,
        copy_bytes_per_row * height,
        slice_pitch);
    out_info->row_pitch = row_pitch;
    out_info->slice_pitch = slice_pitch;
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_blit_context_reset(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHIUtility::IBlitContext*>(self)->reset();
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_blit_context_blit(
    void* self,
    luna_handle_t destination_texture_object,
    LunaRhiSubresourceIndex destination_subresource,
    const LunaRhiTextureViewDesc* source_view,
    const LunaRhiSamplerDesc* sampler,
    float top_left_x,
    float top_left_y,
    float top_right_x,
    float top_right_y,
    float bottom_left_x,
    float bottom_left_y,
    float bottom_right_x,
    float bottom_right_y)
{
    if (!self || !destination_texture_object || !source_view || !source_view->texture || !sampler)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ITexture* destination_texture = Luna::query_interface<Luna::RHI::ITexture>(destination_texture_object);
    if (!destination_texture)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    static_cast<Luna::RHIUtility::IBlitContext*>(self)->blit(
        destination_texture,
        Luna::RHI::SubresourceIndex(destination_subresource.mip_slice, destination_subresource.array_slice),
        to_texture_view_desc(*source_view),
        to_sampler_desc(*sampler),
        Luna::Float2U(top_left_x, top_left_y),
        Luna::Float2U(top_right_x, top_right_y),
        Luna::Float2U(bottom_left_x, bottom_left_y),
        Luna::Float2U(bottom_right_x, bottom_right_y));
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_blit_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait)
{
    if (!self || !command_buffer_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ICommandBuffer* command_buffer = Luna::query_interface<Luna::RHI::ICommandBuffer>(command_buffer_object);
    if (!command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RV result = static_cast<Luna::RHIUtility::IBlitContext*>(self)->commit(command_buffer, submit_and_wait != 0);
    return from_result(result);
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_reset(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHIUtility::IMipmapGenerationContext*>(self)->reset();
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_generate_mipmaps(
    void* self,
    luna_handle_t texture_object,
    uint32_t source_mip,
    uint32_t num_generate_mips)
{
    if (!self || !texture_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ITexture* texture = Luna::query_interface<Luna::RHI::ITexture>(texture_object);
    if (!texture)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    static_cast<Luna::RHIUtility::IMipmapGenerationContext*>(self)->generate_mipmaps(texture, source_mip, num_generate_mips);
    return 0;
}

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait)
{
    if (!self || !command_buffer_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ICommandBuffer* command_buffer = Luna::query_interface<Luna::RHI::ICommandBuffer>(command_buffer_object);
    if (!command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RV result = static_cast<Luna::RHIUtility::IMipmapGenerationContext*>(self)->commit(command_buffer, submit_and_wait != 0);
    return from_result(result);
}
}
