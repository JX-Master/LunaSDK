#pragma once

#include "../RHI/RHI.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_RHI_UTILITY_C_API __declspec(dllexport)
#else
#define LUNA_RHI_UTILITY_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaRhiUtilityResourceWriteContextHandle
{
    luna_handle_t object;
    void* iresource_write_context;
} LunaRhiUtilityResourceWriteContextHandle;

typedef struct LunaRhiUtilityResourceReadContextHandle
{
    luna_handle_t object;
    void* iresource_read_context;
} LunaRhiUtilityResourceReadContextHandle;

typedef struct LunaRhiUtilityBlitContextHandle
{
    luna_handle_t object;
    void* iblit_context;
} LunaRhiUtilityBlitContextHandle;

typedef struct LunaRhiUtilityMipmapGenerationContextHandle
{
    luna_handle_t object;
    void* imipmap_generation_context;
} LunaRhiUtilityMipmapGenerationContextHandle;

typedef struct LunaRhiUtilityTextureWriteInfo
{
    uint32_t row_pitch;
    uint32_t slice_pitch;
} LunaRhiUtilityTextureWriteInfo;

typedef struct LunaRhiUtilityTextureReadInfo
{
    uint32_t row_pitch;
    uint32_t slice_pitch;
} LunaRhiUtilityTextureReadInfo;

LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_init_module(void);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_resource_write_context(
    luna_handle_t device_object,
    LunaRhiUtilityResourceWriteContextHandle* out_context);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_resource_read_context(
    luna_handle_t device_object,
    LunaRhiUtilityResourceReadContextHandle* out_context);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_blit_context(
    luna_handle_t device_object,
    uint32_t destination_format,
    LunaRhiUtilityBlitContextHandle* out_context);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_new_mipmap_generation_context(
    luna_handle_t device_object,
    LunaRhiUtilityMipmapGenerationContextHandle* out_context);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_reset(void* self);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_write_buffer(
    void* self,
    luna_handle_t buffer_object,
    uint64_t offset,
    const void* data,
    uint64_t data_size);
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
    LunaRhiUtilityTextureWriteInfo* out_info);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_write_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_reset(void* self);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_read_buffer(
    void* self,
    luna_handle_t buffer_object,
    uint64_t offset,
    uint64_t data_size,
    uint64_t* out_handle);
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
    uint64_t* out_handle);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_get_buffer_data(
    void* self,
    uint64_t handle,
    void* out_data,
    uint64_t data_size);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_resource_read_context_get_texture_data(
    void* self,
    uint64_t handle,
    uint32_t copy_bytes_per_row,
    uint32_t height,
    uint32_t depth,
    void* out_data,
    uint64_t data_size,
    LunaRhiUtilityTextureReadInfo* out_info);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_blit_context_reset(void* self);
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
    float bottom_right_y);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_blit_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_reset(void* self);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_generate_mipmaps(
    void* self,
    luna_handle_t texture_object,
    uint32_t source_mip,
    uint32_t num_generate_mips);
LUNA_RHI_UTILITY_C_API luna_errcode_t luna_rhi_utility_mipmap_generation_context_commit(
    void* self,
    luna_handle_t command_buffer_object,
    int32_t submit_and_wait);

#ifdef __cplusplus
}
#endif
