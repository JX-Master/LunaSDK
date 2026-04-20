#pragma once

#include "../Runtime/Runtime.h"
#include "../Window/Window.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_RHI_C_API __declspec(dllexport)
#else
#define LUNA_RHI_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaRhiDeviceHandle
{
    luna_handle_t object;
    void* idevice;
} LunaRhiDeviceHandle;

typedef struct LunaRhiAdapterHandle
{
    luna_handle_t object;
    void* iadapter;
} LunaRhiAdapterHandle;

typedef struct LunaRhiDeviceMemoryHandle
{
    luna_handle_t object;
    void* idevice_memory;
} LunaRhiDeviceMemoryHandle;

typedef struct LunaRhiFenceHandle
{
    luna_handle_t object;
    void* ifence;
} LunaRhiFenceHandle;

typedef struct LunaRhiQueryHeapHandle
{
    luna_handle_t object;
    void* iquery_heap;
} LunaRhiQueryHeapHandle;

typedef struct LunaRhiSwapChainHandle
{
    luna_handle_t object;
    void* iswap_chain;
} LunaRhiSwapChainHandle;

typedef struct LunaRhiCommandBufferHandle
{
    luna_handle_t object;
    void* icommand_buffer;
} LunaRhiCommandBufferHandle;

typedef struct LunaRhiBufferHandle
{
    luna_handle_t object;
    void* ibuffer;
} LunaRhiBufferHandle;

typedef struct LunaRhiPipelineLayoutHandle
{
    luna_handle_t object;
    void* ipipeline_layout;
} LunaRhiPipelineLayoutHandle;

typedef struct LunaRhiPipelineStateHandle
{
    luna_handle_t object;
    void* ipipeline_state;
} LunaRhiPipelineStateHandle;

typedef struct LunaRhiDescriptorSetLayoutHandle
{
    luna_handle_t object;
    void* idescriptor_set_layout;
} LunaRhiDescriptorSetLayoutHandle;

typedef struct LunaRhiDescriptorSetHandle
{
    luna_handle_t object;
    void* idescriptor_set;
} LunaRhiDescriptorSetHandle;

typedef struct LunaRhiTextureHandle
{
    luna_handle_t object;
    void* itexture;
} LunaRhiTextureHandle;

typedef struct LunaRhiCommandQueueDesc
{
    uint32_t type;
    uint32_t flags;
} LunaRhiCommandQueueDesc;

typedef struct LunaRhiQueryHeapDesc
{
    uint32_t type;
    uint32_t count;
} LunaRhiQueryHeapDesc;

typedef struct LunaRhiPipelineStatistics
{
    uint64_t vs_invocations;
    uint64_t rasterizer_input_primitives;
    uint64_t rendered_primitives;
    uint64_t ps_invocations;
    uint64_t cs_invocations;
} LunaRhiPipelineStatistics;

typedef struct LunaRhiViewport
{
    float top_left_x;
    float top_left_y;
    float width;
    float height;
    float min_depth;
    float max_depth;
} LunaRhiViewport;

typedef struct LunaRhiRectI
{
    int32_t offset_x;
    int32_t offset_y;
    int32_t width;
    int32_t height;
} LunaRhiRectI;

typedef struct LunaRhiVertexBufferView
{
    void* buffer;
    uint64_t offset;
    uint32_t size;
    uint32_t element_size;
} LunaRhiVertexBufferView;

typedef struct LunaRhiSwapChainDesc
{
    uint32_t width;
    uint32_t height;
    uint32_t buffer_count;
    uint32_t format;
    uint32_t color_space;
    int32_t vertical_synchronized;
} LunaRhiSwapChainDesc;

typedef struct LunaRhiTextureDesc
{
    uint32_t type;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t array_size;
    uint32_t mip_levels;
    uint32_t sample_count;
    uint32_t usages;
    uint32_t flags;
} LunaRhiTextureDesc;

typedef struct LunaRhiClearValue
{
    uint32_t format;
    uint32_t type;
    float color_red;
    float color_green;
    float color_blue;
    float color_alpha;
    float depth;
    uint8_t stencil;
} LunaRhiClearValue;

typedef struct LunaRhiBufferDesc
{
    uint64_t size;
    uint32_t usages;
    uint32_t flags;
} LunaRhiBufferDesc;

typedef struct LunaRhiInputBindingDesc
{
    uint32_t binding_slot;
    uint32_t element_size;
    uint32_t input_rate;
} LunaRhiInputBindingDesc;

typedef struct LunaRhiInputAttributeDesc
{
    const char* semantic_name;
    uint32_t semantic_index;
    uint32_t location;
    uint32_t binding_slot;
    uint32_t offset;
    uint32_t format;
} LunaRhiInputAttributeDesc;

typedef struct LunaRhiShaderData
{
    const uint8_t* data;
    uint64_t size;
    const char* entry_point;
    uint32_t format;
} LunaRhiShaderData;

typedef struct LunaRhiRasterizerDesc
{
    int32_t depth_bias;
    float slope_scaled_depth_bias;
    float depth_bias_clamp;
    uint32_t fill_mode;
    uint32_t cull_mode;
    int32_t front_counter_clockwise;
    int32_t depth_clamp_enable;
} LunaRhiRasterizerDesc;

typedef struct LunaRhiDepthStencilDesc
{
    int32_t depth_test_enable;
    int32_t depth_write_enable;
    uint32_t depth_func;
    int32_t stencil_enable;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    uint32_t front_stencil_fail_op;
    uint32_t front_stencil_depth_fail_op;
    uint32_t front_stencil_pass_op;
    uint32_t front_stencil_func;
    uint32_t back_stencil_fail_op;
    uint32_t back_stencil_depth_fail_op;
    uint32_t back_stencil_pass_op;
    uint32_t back_stencil_func;
} LunaRhiDepthStencilDesc;

typedef struct LunaRhiAttachmentBlendDesc
{
    int32_t blend_enable;
    uint32_t src_blend_color;
    uint32_t dst_blend_color;
    uint32_t blend_op_color;
    uint32_t src_blend_alpha;
    uint32_t dst_blend_alpha;
    uint32_t blend_op_alpha;
    uint32_t color_write_mask;
} LunaRhiAttachmentBlendDesc;

typedef struct LunaRhiBlendDesc
{
    int32_t alpha_to_coverage_enable;
    int32_t independent_blend_enable;
    LunaRhiAttachmentBlendDesc attachments[8];
} LunaRhiBlendDesc;

typedef struct LunaRhiGraphicsPipelineStateDesc
{
    const LunaRhiInputBindingDesc* input_bindings;
    uint64_t input_binding_count;
    const LunaRhiInputAttributeDesc* input_attributes;
    uint64_t input_attribute_count;
    void* pipeline_layout;
    LunaRhiShaderData vertex_shader;
    LunaRhiShaderData pixel_shader;
    LunaRhiRasterizerDesc rasterizer_state;
    LunaRhiDepthStencilDesc depth_stencil_state;
    LunaRhiBlendDesc blend_state;
    uint32_t index_buffer_strip_cut_value;
    uint32_t primitive_topology;
    uint8_t num_color_attachments;
    uint32_t color_formats[8];
    uint32_t depth_stencil_format;
    uint32_t sample_count;
} LunaRhiGraphicsPipelineStateDesc;

typedef struct LunaRhiComputePipelineStateDesc
{
    void* pipeline_layout;
    LunaRhiShaderData compute_shader;
    uint32_t metal_numthreads_x;
    uint32_t metal_numthreads_y;
    uint32_t metal_numthreads_z;
} LunaRhiComputePipelineStateDesc;

typedef struct LunaRhiTextureDataPlacementInfo
{
    uint64_t size;
    uint64_t alignment;
    uint64_t row_pitch;
    uint64_t slice_pitch;
} LunaRhiTextureDataPlacementInfo;

typedef struct LunaRhiDescriptorSetLayoutBinding
{
    uint32_t binding_slot;
    uint32_t num_descs;
    uint32_t type;
    uint32_t texture_view_type;
    uint32_t shader_visibility_flags;
} LunaRhiDescriptorSetLayoutBinding;

typedef struct LunaRhiBufferViewDesc
{
    uint64_t first_element;
    void* buffer;
    uint32_t element_count;
    uint32_t element_size;
} LunaRhiBufferViewDesc;

typedef struct LunaRhiTextureViewDesc
{
    void* texture;
    uint32_t type;
    uint32_t format;
    uint32_t mip_slice;
    uint32_t mip_size;
    uint32_t array_slice;
    uint32_t array_size;
} LunaRhiTextureViewDesc;

typedef struct LunaRhiSamplerDesc
{
    uint32_t min_filter;
    uint32_t mag_filter;
    uint32_t mip_filter;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    int32_t anisotropy_enable;
    int32_t compare_enable;
    uint32_t compare_function;
    uint32_t border_color;
    uint32_t max_anisotropy;
    float min_lod;
    float max_lod;
} LunaRhiSamplerDesc;

typedef struct LunaRhiSubresourceIndex
{
    uint32_t mip_slice;
    uint32_t array_slice;
} LunaRhiSubresourceIndex;

typedef struct LunaRhiBufferBarrier
{
    void* buffer;
    uint32_t before;
    uint32_t after;
    uint32_t flags;
} LunaRhiBufferBarrier;

typedef struct LunaRhiTextureBarrier
{
    void* texture;
    LunaRhiSubresourceIndex subresource;
    uint32_t before;
    uint32_t after;
    uint32_t flags;
} LunaRhiTextureBarrier;

typedef struct LunaRhiColorAttachment
{
    void* texture;
    uint32_t load_op;
    uint32_t store_op;
    float clear_red;
    float clear_green;
    float clear_blue;
    float clear_alpha;
    uint32_t view_type;
    uint32_t format;
    uint32_t mip_slice;
    uint32_t array_slice;
} LunaRhiColorAttachment;

typedef struct LunaRhiResolveAttachment
{
    void* texture;
    uint32_t mip_slice;
    uint32_t array_slice;
    uint32_t array_size;
} LunaRhiResolveAttachment;

typedef struct LunaRhiDepthStencilAttachment
{
    void* texture;
    int32_t read_only;
    uint32_t depth_load_op;
    uint32_t depth_store_op;
    float depth_clear_value;
    uint32_t stencil_load_op;
    uint32_t stencil_store_op;
    uint8_t stencil_clear_value;
    uint32_t view_type;
    uint32_t format;
    uint32_t mip_slice;
    uint32_t array_slice;
} LunaRhiDepthStencilAttachment;

typedef struct LunaRhiComputePassDesc
{
    void* timestamp_query_heap;
    void* pipeline_statistics_query_heap;
    uint32_t timestamp_query_begin_pass_write_index;
    uint32_t timestamp_query_end_pass_write_index;
    uint32_t pipeline_statistics_query_write_index;
} LunaRhiComputePassDesc;

typedef struct LunaRhiCopyPassDesc
{
    void* timestamp_query_heap;
    uint32_t timestamp_query_begin_pass_write_index;
    uint32_t timestamp_query_end_pass_write_index;
} LunaRhiCopyPassDesc;

LUNA_RHI_C_API luna_errcode_t luna_rhi_init_module(void);
LUNA_RHI_C_API uint32_t luna_rhi_get_backend_type(void);
LUNA_RHI_C_API luna_errcode_t luna_rhi_get_main_device(LunaRhiDeviceHandle* out_device);
LUNA_RHI_C_API luna_errcode_t luna_rhi_new_device(void* adapter, LunaRhiDeviceHandle* out_device);
LUNA_RHI_C_API luna_errcode_t luna_rhi_get_num_adapters(uint32_t* out_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_get_adapter(uint32_t index, LunaRhiAdapterHandle* out_adapter);
LUNA_RHI_C_API luna_errcode_t luna_rhi_adapter_get_name(void* self, const char** out_name);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_child_get_device(luna_handle_t object, LunaRhiDeviceHandle* out_device);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_child_set_name(luna_handle_t object, const char* name);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_num_command_queues(void* self, uint32_t* out_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_command_queue_desc(void* self, uint32_t index, LunaRhiCommandQueueDesc* out_desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_command_queue_timestamp_frequency(void* self, uint32_t index, double* out_frequency);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_check_feature(void* self, uint32_t feature, uint64_t* out_value);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_texture_data_placement_info(void* self, uint32_t width, uint32_t height, uint32_t depth, uint32_t format, LunaRhiTextureDataPlacementInfo* out_info);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_is_resources_aliasing_compatible(void* self, uint32_t memory_type, const LunaRhiBufferDesc* buffers, uint64_t buffer_count, const LunaRhiTextureDesc* textures, uint64_t texture_count, int32_t* out_compatible);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_allocate_memory(void* self, uint32_t memory_type, const LunaRhiBufferDesc* buffers, uint64_t buffer_count, const LunaRhiTextureDesc* textures, uint64_t texture_count, LunaRhiDeviceMemoryHandle* out_memory);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_buffer(void* self, uint32_t memory_type, const LunaRhiBufferDesc* desc, LunaRhiBufferHandle* out_buffer);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_texture(void* self, uint32_t memory_type, const LunaRhiTextureDesc* desc, LunaRhiTextureHandle* out_texture);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_texture_with_clear_value(void* self, uint32_t memory_type, const LunaRhiTextureDesc* desc, const LunaRhiClearValue* optimized_clear_value, LunaRhiTextureHandle* out_texture);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_buffer(void* self, void* device_memory, const LunaRhiBufferDesc* desc, LunaRhiBufferHandle* out_buffer);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_texture(void* self, void* device_memory, const LunaRhiTextureDesc* desc, LunaRhiTextureHandle* out_texture);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_texture_with_clear_value(void* self, void* device_memory, const LunaRhiTextureDesc* desc, const LunaRhiClearValue* optimized_clear_value, LunaRhiTextureHandle* out_texture);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_fence(void* self, LunaRhiFenceHandle* out_fence);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_query_heap(void* self, const LunaRhiQueryHeapDesc* desc, LunaRhiQueryHeapHandle* out_query_heap);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_descriptor_set_layout(void* self, const LunaRhiDescriptorSetLayoutBinding* bindings, uint64_t binding_count, uint32_t flags, LunaRhiDescriptorSetLayoutHandle* out_descriptor_set_layout);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_descriptor_set(void* self, void* descriptor_set_layout, uint32_t num_variable_descriptors, LunaRhiDescriptorSetHandle* out_descriptor_set);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_pipeline_layout(void* self, const void* const* descriptor_set_layouts, uint64_t descriptor_set_layout_count, uint32_t flags, LunaRhiPipelineLayoutHandle* out_pipeline_layout);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_graphics_pipeline_state(void* self, const LunaRhiGraphicsPipelineStateDesc* desc, LunaRhiPipelineStateHandle* out_pipeline_state);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_compute_pipeline_state(void* self, const LunaRhiComputePipelineStateDesc* desc, LunaRhiPipelineStateHandle* out_pipeline_state);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_swap_chain(void* self, uint32_t command_queue_index, luna_handle_t window_object, const LunaRhiSwapChainDesc* desc, LunaRhiSwapChainHandle* out_swap_chain);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_command_buffer(void* self, uint32_t command_queue_index, LunaRhiCommandBufferHandle* out_command_buffer);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_desc(void* self, LunaRhiSwapChainDesc* out_desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_window(void* self, LunaWindowHandle* out_window);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_surface_transform(void* self, uint32_t* out_transform);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_current_back_buffer(void* self, LunaRhiTextureHandle* out_texture);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_present(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_reset_suggested(void* self, int32_t* out_suggested);
LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_reset(void* self, const LunaRhiSwapChainDesc* desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_get_command_queue_index(void* self, uint32_t* out_index);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_reset(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_attach_device_object(void* self, luna_handle_t object);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_event(void* self, const char* event_name);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_event(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_resource_barrier(void* self, const LunaRhiBufferBarrier* buffer_barriers, uint64_t buffer_barrier_count, const LunaRhiTextureBarrier* texture_barriers, uint64_t texture_barrier_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_render_pass(void* self, const LunaRhiColorAttachment* color_attachments, uint32_t color_attachment_count, const LunaRhiResolveAttachment* resolve_attachments, uint32_t resolve_attachment_count, const LunaRhiDepthStencilAttachment* depth_stencil_attachment, void* occlusion_query_heap, void* timestamp_query_heap, void* pipeline_statistics_query_heap, uint32_t timestamp_query_begin_pass_write_index, uint32_t timestamp_query_end_pass_write_index, uint32_t pipeline_statistics_query_write_index, uint32_t array_size, uint8_t sample_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_pipeline_layout(void* self, void* pipeline_layout);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_pipeline_state(void* self, void* pipeline_state);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_descriptor_set(void* self, uint32_t index, void* descriptor_set);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_descriptor_sets(void* self, uint32_t start_index, const void* const* descriptor_sets, uint64_t descriptor_set_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_vertex_buffer(void* self, uint32_t slot, void* buffer, uint64_t offset, uint32_t size, uint32_t element_size);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_vertex_buffers(void* self, uint32_t start_slot, const LunaRhiVertexBufferView* views, uint64_t view_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_index_buffer(void* self, void* buffer, uint64_t offset, uint32_t size, uint32_t format);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_viewport(void* self, float top_left_x, float top_left_y, float width, float height, float min_depth, float max_depth);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_viewports(void* self, const LunaRhiViewport* viewports, uint64_t viewport_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_scissor_rect(void* self, int32_t offset_x, int32_t offset_y, int32_t width, int32_t height);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_scissor_rects(void* self, const LunaRhiRectI* rects, uint64_t rect_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_blend_factor(void* self, float red, float green, float blue, float alpha);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_stencil_ref(void* self, uint32_t stencil_ref);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw(void* self, uint32_t vertex_count, uint32_t start_vertex_location);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_indexed(void* self, uint32_t index_count, uint32_t start_index_location, int32_t base_vertex_location);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_instanced(void* self, uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_indexed_instanced(void* self, uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_occlusion_query(void* self, uint32_t mode, uint32_t index);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_occlusion_query(void* self, uint32_t index);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_render_pass(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_compute_pass(void* self, const LunaRhiComputePassDesc* desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_pipeline_layout(void* self, void* pipeline_layout);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_pipeline_state(void* self, void* pipeline_state);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_descriptor_set(void* self, uint32_t index, void* descriptor_set);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_descriptor_sets(void* self, uint32_t start_index, const void* const* descriptor_sets, uint64_t descriptor_set_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_dispatch(void* self, uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_compute_pass(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_copy_pass(void* self, const LunaRhiCopyPassDesc* desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_resource(void* self, luna_handle_t dst_object, luna_handle_t src_object);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_buffer(void* self, void* dst, uint64_t dst_offset, void* src, uint64_t src_offset, uint64_t copy_bytes);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_texture(void* self, void* dst, LunaRhiSubresourceIndex dst_subresource, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z, void* src, LunaRhiSubresourceIndex src_subresource, uint32_t src_x, uint32_t src_y, uint32_t src_z, uint32_t copy_width, uint32_t copy_height, uint32_t copy_depth);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_buffer_to_texture(void* self, void* dst, LunaRhiSubresourceIndex dst_subresource, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z, void* src, uint64_t src_offset, uint32_t src_row_pitch, uint32_t src_slice_pitch, uint32_t copy_width, uint32_t copy_height, uint32_t copy_depth);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_texture_to_buffer(void* self, void* dst, uint64_t dst_offset, uint32_t dst_row_pitch, uint32_t dst_slice_pitch, void* src, LunaRhiSubresourceIndex src_subresource, uint32_t src_x, uint32_t src_y, uint32_t src_z, uint32_t copy_width, uint32_t copy_height, uint32_t copy_depth);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_copy_pass(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_submit(void* self, int32_t allow_host_waiting);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_submit_with_fences(void* self, const void* const* wait_fences, uint64_t wait_fence_count, const void* const* signal_fences, uint64_t signal_fence_count, int32_t allow_host_waiting);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_wait(void* self);
LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_try_wait(void* self, int32_t* out_waited);
LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_get_desc(void* self, LunaRhiBufferDesc* out_desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_map(void* self, uint64_t read_begin, uint64_t read_end, void** out_data);
LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_unmap(void* self, uint64_t write_begin, uint64_t write_end);
LUNA_RHI_C_API luna_errcode_t luna_rhi_resource_get_memory(luna_handle_t object, LunaRhiDeviceMemoryHandle* out_memory);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_memory_get_memory_type(void* self, uint32_t* out_memory_type);
LUNA_RHI_C_API luna_errcode_t luna_rhi_device_memory_get_size(void* self, uint64_t* out_size);
LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_desc(void* self, LunaRhiQueryHeapDesc* out_desc);
LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_timestamp_values(void* self, uint32_t index, uint32_t count, uint64_t* out_values);
LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_occlusion_values(void* self, uint32_t index, uint32_t count, uint64_t* out_values);
LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_pipeline_statistics_values(void* self, uint32_t index, uint32_t count, LunaRhiPipelineStatistics* out_values);
LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_buffer_view(void* self, uint32_t binding_slot, uint32_t first_array_index, uint32_t descriptor_type, const LunaRhiBufferViewDesc* views, uint32_t view_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_texture_view(void* self, uint32_t binding_slot, uint32_t first_array_index, uint32_t descriptor_type, const LunaRhiTextureViewDesc* views, uint32_t view_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_sampler(void* self, uint32_t binding_slot, uint32_t first_array_index, const LunaRhiSamplerDesc* samplers, uint32_t sampler_count);
LUNA_RHI_C_API luna_errcode_t luna_rhi_texture_get_desc(void* self, LunaRhiTextureDesc* out_desc);
#ifdef __cplusplus
}
#endif
