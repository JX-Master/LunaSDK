#include "RHI.h"

#include <Luna/RHI/Adapter.hpp>
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/DeviceChild.hpp>
#include <Luna/RHI/DeviceMemory.hpp>
#include <Luna/RHI/Fence.hpp>
#include <Luna/RHI/QueryHeap.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/Resource.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Window/Window.hpp>

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

Luna::RHI::SwapChainDesc to_swap_chain_desc(const LunaRhiSwapChainDesc& desc)
{
    return Luna::RHI::SwapChainDesc(
        desc.width,
        desc.height,
        desc.buffer_count,
        static_cast<Luna::RHI::Format>(desc.format),
        desc.vertical_synchronized != 0,
        static_cast<Luna::RHI::ColorSpace>(desc.color_space));
}

LunaRhiSwapChainDesc from_swap_chain_desc(const Luna::RHI::SwapChainDesc& desc)
{
    return LunaRhiSwapChainDesc{
        desc.width,
        desc.height,
        desc.buffer_count,
        static_cast<uint32_t>(desc.format),
        static_cast<uint32_t>(desc.color_space),
        desc.vertical_synchronized ? 1 : 0
    };
}

Luna::RHI::QueryHeapDesc to_query_heap_desc(const LunaRhiQueryHeapDesc& desc)
{
    return Luna::RHI::QueryHeapDesc(
        static_cast<Luna::RHI::QueryType>(desc.type),
        desc.count);
}

Luna::RHI::DepthStencilOpDesc to_depth_stencil_op_desc(
    uint32_t stencil_fail_op,
    uint32_t stencil_depth_fail_op,
    uint32_t stencil_pass_op,
    uint32_t stencil_func)
{
    return Luna::RHI::DepthStencilOpDesc(
        static_cast<Luna::RHI::StencilOp>(stencil_fail_op),
        static_cast<Luna::RHI::StencilOp>(stencil_depth_fail_op),
        static_cast<Luna::RHI::StencilOp>(stencil_pass_op),
        static_cast<Luna::RHI::CompareFunction>(stencil_func));
}

Luna::RHI::AttachmentBlendDesc to_attachment_blend_desc(const LunaRhiAttachmentBlendDesc& desc)
{
    return Luna::RHI::AttachmentBlendDesc(
        desc.blend_enable != 0,
        static_cast<Luna::RHI::BlendFactor>(desc.src_blend_color),
        static_cast<Luna::RHI::BlendFactor>(desc.dst_blend_color),
        static_cast<Luna::RHI::BlendOp>(desc.blend_op_color),
        static_cast<Luna::RHI::BlendFactor>(desc.src_blend_alpha),
        static_cast<Luna::RHI::BlendFactor>(desc.dst_blend_alpha),
        static_cast<Luna::RHI::BlendOp>(desc.blend_op_alpha),
        static_cast<Luna::RHI::ColorWriteMask>(desc.color_write_mask));
}

Luna::RHI::BlendDesc to_blend_desc(const LunaRhiBlendDesc& desc)
{
    Luna::RHI::BlendDesc native_desc;
    native_desc.alpha_to_coverage_enable = desc.alpha_to_coverage_enable != 0;
    native_desc.independent_blend_enable = desc.independent_blend_enable != 0;
    for (uint32_t i = 0; i < 8; ++i)
    {
        native_desc.attachments[i] = to_attachment_blend_desc(desc.attachments[i]);
    }
    return native_desc;
}

LunaRhiQueryHeapDesc from_query_heap_desc(const Luna::RHI::QueryHeapDesc& desc)
{
    return LunaRhiQueryHeapDesc{
        static_cast<uint32_t>(desc.type),
        desc.count
    };
}

LunaRhiPipelineStatistics from_pipeline_statistics(const Luna::RHI::PipelineStatistics& statistics)
{
    return LunaRhiPipelineStatistics{
        statistics.vs_invocations,
        statistics.rasterizer_input_primitives,
        statistics.rendered_primitives,
        statistics.ps_invocations,
        statistics.cs_invocations
    };
}

Luna::RHI::BufferDesc to_buffer_desc(const LunaRhiBufferDesc& desc)
{
    Luna::RHI::BufferDesc buffer_desc;
    buffer_desc.size = desc.size;
    buffer_desc.usages = static_cast<Luna::RHI::BufferUsageFlag>(desc.usages);
    buffer_desc.flags = static_cast<Luna::RHI::ResourceFlag>(desc.flags);
    return buffer_desc;
}

LunaRhiBufferDesc from_buffer_desc(const Luna::RHI::BufferDesc& desc)
{
    return LunaRhiBufferDesc{
        desc.size,
        static_cast<uint32_t>(desc.usages),
        static_cast<uint32_t>(desc.flags)
    };
}

Luna::Vector<Luna::RHI::BufferDesc> to_buffer_descs(const LunaRhiBufferDesc* descs, uint64_t count)
{
    Luna::Vector<Luna::RHI::BufferDesc> ret;
    ret.reserve(static_cast<Luna::usize>(count));
    for (uint64_t i = 0; i < count; ++i)
    {
        ret.push_back(to_buffer_desc(descs[i]));
    }
    return ret;
}

Luna::RHI::TextureDesc to_texture_desc(const LunaRhiTextureDesc& desc)
{
    Luna::RHI::TextureDesc texture_desc;
    texture_desc.type = static_cast<Luna::RHI::TextureType>(desc.type);
    texture_desc.format = static_cast<Luna::RHI::Format>(desc.format);
    texture_desc.width = desc.width;
    texture_desc.height = desc.height;
    texture_desc.depth = desc.depth;
    texture_desc.array_size = desc.array_size;
    texture_desc.mip_levels = desc.mip_levels;
    texture_desc.sample_count = desc.sample_count;
    texture_desc.usages = static_cast<Luna::RHI::TextureUsageFlag>(desc.usages);
    texture_desc.flags = static_cast<Luna::RHI::ResourceFlag>(desc.flags);
    return texture_desc;
}

Luna::RHI::ClearValue to_clear_value(const LunaRhiClearValue& value)
{
    Luna::RHI::ClearValue ret;
    ret.format = static_cast<Luna::RHI::Format>(value.format);
    ret.type = static_cast<Luna::RHI::ClearValueType>(value.type);
    switch (ret.type)
    {
    case Luna::RHI::ClearValueType::color:
        ret.color[0] = value.color_red;
        ret.color[1] = value.color_green;
        ret.color[2] = value.color_blue;
        ret.color[3] = value.color_alpha;
        break;
    case Luna::RHI::ClearValueType::depth_stencil:
        ret.depth_stencil.depth = value.depth;
        ret.depth_stencil.stencil = value.stencil;
        break;
    default:
        break;
    }
    return ret;
}

Luna::Vector<Luna::RHI::TextureDesc> to_texture_descs(const LunaRhiTextureDesc* descs, uint64_t count)
{
    Luna::Vector<Luna::RHI::TextureDesc> ret;
    ret.reserve(static_cast<Luna::usize>(count));
    for (uint64_t i = 0; i < count; ++i)
    {
        ret.push_back(to_texture_desc(descs[i]));
    }
    return ret;
}

Luna::RHI::ShaderData to_shader_data(const LunaRhiShaderData& shader)
{
    if (!shader.data || !shader.size)
    {
        return Luna::RHI::ShaderData();
    }
    return Luna::RHI::ShaderData(
        Luna::Span<const Luna::byte_t>(reinterpret_cast<const Luna::byte_t*>(shader.data), static_cast<Luna::usize>(shader.size)),
        Luna::Name(shader.entry_point ? shader.entry_point : ""),
        static_cast<Luna::RHI::ShaderDataFormat>(shader.format));
}

Luna::RHI::DescriptorSetLayoutBinding to_descriptor_set_layout_binding(const LunaRhiDescriptorSetLayoutBinding& binding)
{
    return Luna::RHI::DescriptorSetLayoutBinding(
        static_cast<Luna::RHI::DescriptorType>(binding.type),
        static_cast<Luna::RHI::TextureViewType>(binding.texture_view_type),
        binding.binding_slot,
        binding.num_descs,
        static_cast<Luna::RHI::ShaderVisibilityFlag>(binding.shader_visibility_flags));
}

Luna::RHI::BufferViewDesc to_buffer_view_desc(const LunaRhiBufferViewDesc& desc)
{
    Luna::RHI::BufferViewDesc ret;
    ret.first_element = desc.first_element;
    ret.buffer = static_cast<Luna::RHI::IBuffer*>(desc.buffer);
    ret.element_count = desc.element_count;
    ret.element_size = desc.element_size;
    return ret;
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

Luna::RHI::Viewport to_viewport(const LunaRhiViewport& viewport)
{
    return Luna::RHI::Viewport(
        viewport.top_left_x,
        viewport.top_left_y,
        viewport.width,
        viewport.height,
        viewport.min_depth,
        viewport.max_depth);
}

Luna::RectI to_rect_i(const LunaRhiRectI& rect)
{
    return Luna::RectI(rect.offset_x, rect.offset_y, rect.width, rect.height);
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

LunaRhiTextureDesc from_texture_desc(const Luna::RHI::TextureDesc& desc)
{
    return LunaRhiTextureDesc{
        static_cast<uint32_t>(desc.type),
        static_cast<uint32_t>(desc.format),
        desc.width,
        desc.height,
        desc.depth,
        desc.array_size,
        desc.mip_levels,
        desc.sample_count,
        static_cast<uint32_t>(desc.usages),
        static_cast<uint32_t>(desc.flags)
    };
}

Luna::RHI::BufferBarrier to_buffer_barrier(const LunaRhiBufferBarrier& barrier)
{
    return Luna::RHI::BufferBarrier(
        static_cast<Luna::RHI::IBuffer*>(barrier.buffer),
        static_cast<Luna::RHI::BufferStateFlag>(barrier.before),
        static_cast<Luna::RHI::BufferStateFlag>(barrier.after),
        static_cast<Luna::RHI::ResourceBarrierFlag>(barrier.flags));
}

Luna::RHI::TextureBarrier to_texture_barrier(const LunaRhiTextureBarrier& barrier)
{
    return Luna::RHI::TextureBarrier(
        static_cast<Luna::RHI::ITexture*>(barrier.texture),
        Luna::RHI::SubresourceIndex(barrier.subresource.mip_slice, barrier.subresource.array_slice),
        static_cast<Luna::RHI::TextureStateFlag>(barrier.before),
        static_cast<Luna::RHI::TextureStateFlag>(barrier.after),
        static_cast<Luna::RHI::ResourceBarrierFlag>(barrier.flags));
}

Luna::RHI::SubresourceIndex to_subresource_index(const LunaRhiSubresourceIndex& index)
{
    return Luna::RHI::SubresourceIndex(index.mip_slice, index.array_slice);
}

Luna::RHI::ColorAttachment to_color_attachment(const LunaRhiColorAttachment& attachment)
{
    return Luna::RHI::ColorAttachment(
        static_cast<Luna::RHI::ITexture*>(attachment.texture),
        static_cast<Luna::RHI::LoadOp>(attachment.load_op),
        static_cast<Luna::RHI::StoreOp>(attachment.store_op),
        Luna::Float4U(attachment.clear_red, attachment.clear_green, attachment.clear_blue, attachment.clear_alpha),
        static_cast<Luna::RHI::TextureViewType>(attachment.view_type),
        static_cast<Luna::RHI::Format>(attachment.format),
        attachment.mip_slice,
        attachment.array_slice);
}

Luna::RHI::ResolveAttachment to_resolve_attachment(const LunaRhiResolveAttachment& attachment)
{
    Luna::RHI::ResolveAttachment ret;
    ret.texture = static_cast<Luna::RHI::ITexture*>(attachment.texture);
    ret.mip_slice = attachment.mip_slice;
    ret.array_slice = attachment.array_slice;
    ret.array_size = attachment.array_size ? attachment.array_size : 1;
    return ret;
}

Luna::RHI::VertexBufferView to_vertex_buffer_view(const LunaRhiVertexBufferView& view)
{
    return Luna::RHI::VertexBufferView(
        static_cast<Luna::RHI::IResource*>(view.buffer),
        view.offset,
        view.size,
        view.element_size);
}

Luna::RHI::DepthStencilAttachment to_depth_stencil_attachment(const LunaRhiDepthStencilAttachment& attachment)
{
    return Luna::RHI::DepthStencilAttachment(
        static_cast<Luna::RHI::ITexture*>(attachment.texture),
        attachment.read_only != 0,
        static_cast<Luna::RHI::LoadOp>(attachment.depth_load_op),
        static_cast<Luna::RHI::StoreOp>(attachment.depth_store_op),
        attachment.depth_clear_value,
        static_cast<Luna::RHI::LoadOp>(attachment.stencil_load_op),
        static_cast<Luna::RHI::StoreOp>(attachment.stencil_store_op),
        attachment.stencil_clear_value,
        static_cast<Luna::RHI::TextureViewType>(attachment.view_type),
        static_cast<Luna::RHI::Format>(attachment.format),
        attachment.mip_slice,
        attachment.array_slice);
}

}

extern "C"
{
LUNA_RHI_C_API luna_errcode_t luna_rhi_init_module(void)
{
    Luna::Module* rhi_module = Luna::module_rhi();
    Luna::RV result = Luna::add_module(rhi_module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(rhi_module);
    return from_result(result);
}

LUNA_RHI_C_API uint32_t luna_rhi_get_backend_type(void)
{
    return static_cast<uint32_t>(Luna::RHI::get_backend_type());
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_get_main_device(LunaRhiDeviceHandle* out_device)
{
    if (!out_device)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_device->object = nullptr;
    out_device->idevice = nullptr;

    Luna::RHI::IDevice* device = Luna::RHI::get_main_device();
    if (!device)
    {
        return from_errcode(Luna::BasicError::bad_calling_time());
    }

    Luna::object_t object = device->get_object();
    if (!object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_device->object = object;
    out_device->idevice = device;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_new_device(void* adapter, LunaRhiDeviceHandle* out_device)
{
    if (!adapter || !out_device)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_device->object = nullptr;
    out_device->idevice = nullptr;

    auto result = Luna::RHI::new_device(static_cast<Luna::RHI::IAdapter*>(adapter));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IDevice> device = Luna::move(result.get());
    Luna::object_t object = device.detach();
    void* idevice = Luna::query_interface<Luna::RHI::IDevice>(object);
    if (!idevice)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_device->object = object;
    out_device->idevice = idevice;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_get_num_adapters(uint32_t* out_count)
{
    if (!out_count)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_count = static_cast<uint32_t>(Luna::RHI::get_adapters().size());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_get_adapter(uint32_t index, LunaRhiAdapterHandle* out_adapter)
{
    if (!out_adapter)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_adapter->object = nullptr;
    out_adapter->iadapter = nullptr;

    Luna::Vector<Luna::Ref<Luna::RHI::IAdapter>> adapters = Luna::RHI::get_adapters();
    if (index >= adapters.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    Luna::RHI::IAdapter* adapter = adapters[index].get();
    Luna::object_t object = adapter ? adapter->get_object() : nullptr;
    if (!adapter || !object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::object_retain(object);
    out_adapter->object = object;
    out_adapter->iadapter = adapter;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_adapter_get_name(void* self, const char** out_name)
{
    if (!self || !out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = static_cast<Luna::RHI::IAdapter*>(self)->get_name();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_child_get_device(luna_handle_t object, LunaRhiDeviceHandle* out_device)
{
    if (!object || !out_device)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_device->object = nullptr;
    out_device->idevice = nullptr;

    Luna::RHI::IDeviceChild* child = Luna::query_interface<Luna::RHI::IDeviceChild>(object);
    if (!child)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RHI::IDevice* device = child->get_device();
    Luna::object_t device_object = device ? device->get_object() : nullptr;
    if (!device || !device_object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_device->object = device_object;
    out_device->idevice = device;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_child_set_name(luna_handle_t object, const char* name)
{
    if (!object || !name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IDeviceChild* child = Luna::query_interface<Luna::RHI::IDeviceChild>(object);
    if (!child)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    child->set_name(name);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_num_command_queues(void* self, uint32_t* out_count)
{
    if (!self || !out_count)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_count = static_cast<Luna::RHI::IDevice*>(self)->get_num_command_queues();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_command_queue_desc(void* self, uint32_t index, LunaRhiCommandQueueDesc* out_desc)
{
    if (!self || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IDevice* device = static_cast<Luna::RHI::IDevice*>(self);
    if (index >= device->get_num_command_queues())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    Luna::RHI::CommandQueueDesc desc = device->get_command_queue_desc(index);
    out_desc->type = static_cast<uint32_t>(desc.type);
    out_desc->flags = static_cast<uint32_t>(desc.flags);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_command_queue_timestamp_frequency(void* self, uint32_t index, double* out_frequency)
{
    if (!self || !out_frequency)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IDevice* device = static_cast<Luna::RHI::IDevice*>(self);
    if (index >= device->get_num_command_queues())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    Luna::R<Luna::f64> result = device->get_command_queue_timestamp_frequency(index);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_frequency = result.get();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_check_feature(void* self, uint32_t feature, uint64_t* out_value)
{
    if (!self || !out_value)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::DeviceFeature device_feature = static_cast<Luna::RHI::DeviceFeature>(feature);
    switch (device_feature)
    {
    case Luna::RHI::DeviceFeature::unbound_descriptor_array:
    case Luna::RHI::DeviceFeature::pixel_shader_write:
    case Luna::RHI::DeviceFeature::uniform_buffer_data_alignment:
    case Luna::RHI::DeviceFeature::structured_buffer_offset_alignment:
    case Luna::RHI::DeviceFeature::rasterizer_depth_clamp:
        break;
    default:
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IDevice* device = static_cast<Luna::RHI::IDevice*>(self);
    Luna::RHI::DeviceFeatureData data = device->check_feature(device_feature);
    switch (device_feature)
    {
    case Luna::RHI::DeviceFeature::unbound_descriptor_array:
        *out_value = data.unbound_descriptor_array ? 1 : 0;
        return 0;
    case Luna::RHI::DeviceFeature::pixel_shader_write:
        *out_value = data.pixel_shader_write ? 1 : 0;
        return 0;
    case Luna::RHI::DeviceFeature::uniform_buffer_data_alignment:
        *out_value = data.uniform_buffer_data_alignment;
        return 0;
    case Luna::RHI::DeviceFeature::structured_buffer_offset_alignment:
        *out_value = data.structured_buffer_offset_alignment;
        return 0;
    case Luna::RHI::DeviceFeature::rasterizer_depth_clamp:
        *out_value = data.rasterizer_depth_clamp ? 1 : 0;
        return 0;
    default:
        return 0;
    }
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_get_texture_data_placement_info(
    void* self,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t format,
    LunaRhiTextureDataPlacementInfo* out_info)
{
    if (!self || !out_info)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_info->size = 0;
    out_info->alignment = 0;
    out_info->row_pitch = 0;
    out_info->slice_pitch = 0;
    static_cast<Luna::RHI::IDevice*>(self)->get_texture_data_placement_info(
        width,
        height,
        depth,
        static_cast<Luna::RHI::Format>(format),
        &out_info->size,
        &out_info->alignment,
        &out_info->row_pitch,
        &out_info->slice_pitch);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_is_resources_aliasing_compatible(
    void* self,
    uint32_t memory_type,
    const LunaRhiBufferDesc* buffers,
    uint64_t buffer_count,
    const LunaRhiTextureDesc* textures,
    uint64_t texture_count,
    int32_t* out_compatible)
{
    if (!self || (buffer_count && !buffers) || (texture_count && !textures) || !out_compatible)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::BufferDesc> native_buffers = to_buffer_descs(buffers, buffer_count);
    Luna::Vector<Luna::RHI::TextureDesc> native_textures = to_texture_descs(textures, texture_count);
    *out_compatible = static_cast<Luna::RHI::IDevice*>(self)->is_resources_aliasing_compatible(
        static_cast<Luna::RHI::MemoryType>(memory_type),
        Luna::Span<const Luna::RHI::BufferDesc>(native_buffers.data(), native_buffers.size()),
        Luna::Span<const Luna::RHI::TextureDesc>(native_textures.data(), native_textures.size())) ? 1 : 0;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_allocate_memory(
    void* self,
    uint32_t memory_type,
    const LunaRhiBufferDesc* buffers,
    uint64_t buffer_count,
    const LunaRhiTextureDesc* textures,
    uint64_t texture_count,
    LunaRhiDeviceMemoryHandle* out_memory)
{
    if (!self || (buffer_count && !buffers) || (texture_count && !textures) || !out_memory)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_memory->object = nullptr;
    out_memory->idevice_memory = nullptr;

    Luna::Vector<Luna::RHI::BufferDesc> native_buffers = to_buffer_descs(buffers, buffer_count);
    Luna::Vector<Luna::RHI::TextureDesc> native_textures = to_texture_descs(textures, texture_count);
    auto result = static_cast<Luna::RHI::IDevice*>(self)->allocate_memory(
        static_cast<Luna::RHI::MemoryType>(memory_type),
        Luna::Span<const Luna::RHI::BufferDesc>(native_buffers.data(), native_buffers.size()),
        Luna::Span<const Luna::RHI::TextureDesc>(native_textures.data(), native_textures.size()));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IDeviceMemory> memory = Luna::move(result.get());
    Luna::object_t object = memory.detach();
    void* idevice_memory = Luna::query_interface<Luna::RHI::IDeviceMemory>(object);
    if (!idevice_memory)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_memory->object = object;
    out_memory->idevice_memory = idevice_memory;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_buffer(void* self, uint32_t memory_type, const LunaRhiBufferDesc* desc, LunaRhiBufferHandle* out_buffer)
{
    if (!self || !desc || !out_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_buffer->object = nullptr;
    out_buffer->ibuffer = nullptr;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_buffer(
        static_cast<Luna::RHI::MemoryType>(memory_type),
        to_buffer_desc(*desc));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IBuffer> buffer = Luna::move(result.get());
    Luna::object_t object = buffer.detach();
    void* ibuffer = Luna::query_interface<Luna::RHI::IBuffer>(object);
    if (!ibuffer)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_buffer->object = object;
    out_buffer->ibuffer = ibuffer;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_buffer(void* self, void* device_memory, const LunaRhiBufferDesc* desc, LunaRhiBufferHandle* out_buffer)
{
    if (!self || !device_memory || !desc || !out_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_buffer->object = nullptr;
    out_buffer->ibuffer = nullptr;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_aliasing_buffer(
        static_cast<Luna::RHI::IDeviceMemory*>(device_memory),
        to_buffer_desc(*desc));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IBuffer> buffer = Luna::move(result.get());
    Luna::object_t object = buffer.detach();
    void* ibuffer = Luna::query_interface<Luna::RHI::IBuffer>(object);
    if (!ibuffer)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_buffer->object = object;
    out_buffer->ibuffer = ibuffer;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_texture(void* self, uint32_t memory_type, const LunaRhiTextureDesc* desc, LunaRhiTextureHandle* out_texture)
{
    return luna_rhi_device_new_texture_with_clear_value(self, memory_type, desc, nullptr, out_texture);
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_texture_with_clear_value(void* self, uint32_t memory_type, const LunaRhiTextureDesc* desc, const LunaRhiClearValue* optimized_clear_value, LunaRhiTextureHandle* out_texture)
{
    if (!self || !desc || !out_texture)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_texture->object = nullptr;
    out_texture->itexture = nullptr;
    Luna::RHI::ClearValue clear_value;
    Luna::RHI::ClearValue* clear_value_ptr = nullptr;
    if (optimized_clear_value)
    {
        clear_value = to_clear_value(*optimized_clear_value);
        clear_value_ptr = &clear_value;
    }

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_texture(
        static_cast<Luna::RHI::MemoryType>(memory_type),
        to_texture_desc(*desc),
        clear_value_ptr);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::ITexture> texture = Luna::move(result.get());
    Luna::object_t object = texture.detach();
    void* itexture = Luna::query_interface<Luna::RHI::ITexture>(object);
    if (!itexture)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_texture->object = object;
    out_texture->itexture = itexture;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_texture(void* self, void* device_memory, const LunaRhiTextureDesc* desc, LunaRhiTextureHandle* out_texture)
{
    return luna_rhi_device_new_aliasing_texture_with_clear_value(self, device_memory, desc, nullptr, out_texture);
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_aliasing_texture_with_clear_value(void* self, void* device_memory, const LunaRhiTextureDesc* desc, const LunaRhiClearValue* optimized_clear_value, LunaRhiTextureHandle* out_texture)
{
    if (!self || !device_memory || !desc || !out_texture)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_texture->object = nullptr;
    out_texture->itexture = nullptr;
    Luna::RHI::ClearValue clear_value;
    Luna::RHI::ClearValue* clear_value_ptr = nullptr;
    if (optimized_clear_value)
    {
        clear_value = to_clear_value(*optimized_clear_value);
        clear_value_ptr = &clear_value;
    }

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_aliasing_texture(
        static_cast<Luna::RHI::IDeviceMemory*>(device_memory),
        to_texture_desc(*desc),
        clear_value_ptr);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::ITexture> texture = Luna::move(result.get());
    Luna::object_t object = texture.detach();
    void* itexture = Luna::query_interface<Luna::RHI::ITexture>(object);
    if (!itexture)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_texture->object = object;
    out_texture->itexture = itexture;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_fence(void* self, LunaRhiFenceHandle* out_fence)
{
    if (!self || !out_fence)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_fence->object = nullptr;
    out_fence->ifence = nullptr;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_fence();
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IFence> fence = Luna::move(result.get());
    Luna::object_t object = fence.detach();
    void* ifence = Luna::query_interface<Luna::RHI::IFence>(object);
    if (!ifence)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_fence->object = object;
    out_fence->ifence = ifence;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_query_heap(void* self, const LunaRhiQueryHeapDesc* desc, LunaRhiQueryHeapHandle* out_query_heap)
{
    if (!self || !desc || !out_query_heap)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_query_heap->object = nullptr;
    out_query_heap->iquery_heap = nullptr;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_query_heap(to_query_heap_desc(*desc));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IQueryHeap> query_heap = Luna::move(result.get());
    Luna::object_t object = query_heap.detach();
    void* iquery_heap = Luna::query_interface<Luna::RHI::IQueryHeap>(object);
    if (!iquery_heap)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_query_heap->object = object;
    out_query_heap->iquery_heap = iquery_heap;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_descriptor_set_layout(
    void* self,
    const LunaRhiDescriptorSetLayoutBinding* bindings,
    uint64_t binding_count,
    uint32_t flags,
    LunaRhiDescriptorSetLayoutHandle* out_descriptor_set_layout)
{
    if (!self || !out_descriptor_set_layout || (binding_count && !bindings))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_descriptor_set_layout->object = nullptr;
    out_descriptor_set_layout->idescriptor_set_layout = nullptr;

    Luna::Vector<Luna::RHI::DescriptorSetLayoutBinding> native_bindings;
    native_bindings.reserve(static_cast<Luna::usize>(binding_count));
    for (uint64_t i = 0; i < binding_count; ++i)
    {
        native_bindings.push_back(to_descriptor_set_layout_binding(bindings[i]));
    }

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_descriptor_set_layout(
        Luna::RHI::DescriptorSetLayoutDesc(
            Luna::Span<const Luna::RHI::DescriptorSetLayoutBinding>(native_bindings.data(), native_bindings.size()),
            static_cast<Luna::RHI::DescriptorSetLayoutFlag>(flags)));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IDescriptorSetLayout> layout = Luna::move(result.get());
    Luna::object_t object = layout.detach();
    void* ilayout = Luna::query_interface<Luna::RHI::IDescriptorSetLayout>(object);
    if (!ilayout)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_descriptor_set_layout->object = object;
    out_descriptor_set_layout->idescriptor_set_layout = ilayout;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_descriptor_set(void* self, void* descriptor_set_layout, uint32_t num_variable_descriptors, LunaRhiDescriptorSetHandle* out_descriptor_set)
{
    if (!self || !descriptor_set_layout || !out_descriptor_set)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_descriptor_set->object = nullptr;
    out_descriptor_set->idescriptor_set = nullptr;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_descriptor_set(
        Luna::RHI::DescriptorSetDesc(static_cast<Luna::RHI::IDescriptorSetLayout*>(descriptor_set_layout), num_variable_descriptors));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IDescriptorSet> descriptor_set = Luna::move(result.get());
    Luna::object_t object = descriptor_set.detach();
    void* idescriptor_set = Luna::query_interface<Luna::RHI::IDescriptorSet>(object);
    if (!idescriptor_set)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_descriptor_set->object = object;
    out_descriptor_set->idescriptor_set = idescriptor_set;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_pipeline_layout(
    void* self,
    const void* const* descriptor_set_layouts,
    uint64_t descriptor_set_layout_count,
    uint32_t flags,
    LunaRhiPipelineLayoutHandle* out_pipeline_layout)
{
    if (!self || !out_pipeline_layout || (descriptor_set_layout_count && !descriptor_set_layouts))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_pipeline_layout->object = nullptr;
    out_pipeline_layout->ipipeline_layout = nullptr;

    Luna::Vector<Luna::RHI::IDescriptorSetLayout*> native_layouts;
    native_layouts.reserve(static_cast<Luna::usize>(descriptor_set_layout_count));
    for (uint64_t i = 0; i < descriptor_set_layout_count; ++i)
    {
        native_layouts.push_back(static_cast<Luna::RHI::IDescriptorSetLayout*>(const_cast<void*>(descriptor_set_layouts[i])));
    }

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_pipeline_layout(
        Luna::RHI::PipelineLayoutDesc(
            Luna::Span<Luna::RHI::IDescriptorSetLayout*>(native_layouts.data(), native_layouts.size()),
            static_cast<Luna::RHI::PipelineLayoutFlag>(flags)));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IPipelineLayout> pipeline_layout = Luna::move(result.get());
    Luna::object_t object = pipeline_layout.detach();
    void* ipipeline_layout = Luna::query_interface<Luna::RHI::IPipelineLayout>(object);
    if (!ipipeline_layout)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_pipeline_layout->object = object;
    out_pipeline_layout->ipipeline_layout = ipipeline_layout;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_graphics_pipeline_state(void* self, const LunaRhiGraphicsPipelineStateDesc* desc, LunaRhiPipelineStateHandle* out_pipeline_state)
{
    if (!self || !desc || !out_pipeline_state ||
        (desc->input_binding_count && !desc->input_bindings) ||
        (desc->input_attribute_count && !desc->input_attributes) ||
        !desc->pipeline_layout)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_pipeline_state->object = nullptr;
    out_pipeline_state->ipipeline_state = nullptr;

    Luna::Vector<Luna::RHI::InputBindingDesc> input_bindings;
    Luna::Vector<Luna::RHI::InputAttributeDesc> input_attributes;
    input_bindings.reserve(static_cast<Luna::usize>(desc->input_binding_count));
    input_attributes.reserve(static_cast<Luna::usize>(desc->input_attribute_count));
    for (uint64_t i = 0; i < desc->input_binding_count; ++i)
    {
        const LunaRhiInputBindingDesc& binding = desc->input_bindings[i];
        input_bindings.push_back(Luna::RHI::InputBindingDesc(
            binding.binding_slot,
            binding.element_size,
            static_cast<Luna::RHI::InputRate>(binding.input_rate)));
    }
    for (uint64_t i = 0; i < desc->input_attribute_count; ++i)
    {
        const LunaRhiInputAttributeDesc& attribute = desc->input_attributes[i];
        input_attributes.push_back(Luna::RHI::InputAttributeDesc(
            attribute.location,
            attribute.binding_slot,
            attribute.offset,
            static_cast<Luna::RHI::Format>(attribute.format)));
    }

    Luna::RHI::GraphicsPipelineStateDesc native_desc;
    native_desc.input_layout = Luna::RHI::InputLayoutDesc(
        Luna::Span<const Luna::RHI::InputBindingDesc>(input_bindings.data(), input_bindings.size()),
        Luna::Span<const Luna::RHI::InputAttributeDesc>(input_attributes.data(), input_attributes.size()));
    native_desc.pipeline_layout = static_cast<Luna::RHI::IPipelineLayout*>(desc->pipeline_layout);
    native_desc.vs = to_shader_data(desc->vertex_shader);
    native_desc.ps = to_shader_data(desc->pixel_shader);
    native_desc.rasterizer_state = Luna::RHI::RasterizerDesc(
        static_cast<Luna::RHI::FillMode>(desc->rasterizer_state.fill_mode),
        static_cast<Luna::RHI::CullMode>(desc->rasterizer_state.cull_mode),
        desc->rasterizer_state.depth_bias,
        desc->rasterizer_state.slope_scaled_depth_bias,
        desc->rasterizer_state.depth_bias_clamp,
        desc->rasterizer_state.front_counter_clockwise != 0,
        desc->rasterizer_state.depth_clamp_enable != 0);
    native_desc.depth_stencil_state = Luna::RHI::DepthStencilDesc(
        desc->depth_stencil_state.depth_test_enable != 0,
        desc->depth_stencil_state.depth_write_enable != 0,
        static_cast<Luna::RHI::CompareFunction>(desc->depth_stencil_state.depth_func),
        desc->depth_stencil_state.stencil_enable != 0,
        desc->depth_stencil_state.stencil_read_mask,
        desc->depth_stencil_state.stencil_write_mask,
        to_depth_stencil_op_desc(
            desc->depth_stencil_state.front_stencil_fail_op,
            desc->depth_stencil_state.front_stencil_depth_fail_op,
            desc->depth_stencil_state.front_stencil_pass_op,
            desc->depth_stencil_state.front_stencil_func),
        to_depth_stencil_op_desc(
            desc->depth_stencil_state.back_stencil_fail_op,
            desc->depth_stencil_state.back_stencil_depth_fail_op,
            desc->depth_stencil_state.back_stencil_pass_op,
            desc->depth_stencil_state.back_stencil_func));
    native_desc.blend_state = to_blend_desc(desc->blend_state);
    native_desc.ib_strip_cut_value = static_cast<Luna::RHI::IndexBufferStripCutValue>(desc->index_buffer_strip_cut_value);
    native_desc.primitive_topology = static_cast<Luna::RHI::PrimitiveTopology>(desc->primitive_topology);
    native_desc.num_color_attachments = desc->num_color_attachments;
    for (uint32_t i = 0; i < 8; ++i)
    {
        native_desc.color_formats[i] = static_cast<Luna::RHI::Format>(desc->color_formats[i]);
    }
    native_desc.depth_stencil_format = static_cast<Luna::RHI::Format>(desc->depth_stencil_format);
    native_desc.sample_count = desc->sample_count;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_graphics_pipeline_state(native_desc);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IPipelineState> pipeline_state = Luna::move(result.get());
    Luna::object_t object = pipeline_state.detach();
    void* ipipeline_state = Luna::query_interface<Luna::RHI::IPipelineState>(object);
    if (!ipipeline_state)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_pipeline_state->object = object;
    out_pipeline_state->ipipeline_state = ipipeline_state;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_compute_pipeline_state(void* self, const LunaRhiComputePipelineStateDesc* desc, LunaRhiPipelineStateHandle* out_pipeline_state)
{
    if (!self || !desc || !out_pipeline_state || !desc->pipeline_layout)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_pipeline_state->object = nullptr;
    out_pipeline_state->ipipeline_state = nullptr;

    Luna::RHI::ComputePipelineStateDesc native_desc;
    native_desc.pipeline_layout = static_cast<Luna::RHI::IPipelineLayout*>(desc->pipeline_layout);
    native_desc.cs = to_shader_data(desc->compute_shader);
    native_desc.metal_numthreads_x = desc->metal_numthreads_x;
    native_desc.metal_numthreads_y = desc->metal_numthreads_y;
    native_desc.metal_numthreads_z = desc->metal_numthreads_z;

    auto result = static_cast<Luna::RHI::IDevice*>(self)->new_compute_pipeline_state(native_desc);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::IPipelineState> pipeline_state = Luna::move(result.get());
    Luna::object_t object = pipeline_state.detach();
    void* ipipeline_state = Luna::query_interface<Luna::RHI::IPipelineState>(object);
    if (!ipipeline_state)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_pipeline_state->object = object;
    out_pipeline_state->ipipeline_state = ipipeline_state;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_swap_chain(
    void* self,
    uint32_t command_queue_index,
    luna_handle_t window_object,
    const LunaRhiSwapChainDesc* desc,
    LunaRhiSwapChainHandle* out_swap_chain)
{
    if (!self || !window_object || !desc || !out_swap_chain)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_swap_chain->object = nullptr;
    out_swap_chain->iswap_chain = nullptr;

    Luna::RHI::IDevice* device = static_cast<Luna::RHI::IDevice*>(self);
    if (command_queue_index >= device->get_num_command_queues())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    Luna::Window::IWindow* window = Luna::query_interface<Luna::Window::IWindow>(window_object);
    if (!window)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    auto result = device->new_swap_chain(command_queue_index, window, to_swap_chain_desc(*desc));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::ISwapChain> swap_chain = Luna::move(result.get());
    Luna::object_t object = swap_chain.detach();
    void* iswap_chain = Luna::query_interface<Luna::RHI::ISwapChain>(object);
    if (!iswap_chain)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_swap_chain->object = object;
    out_swap_chain->iswap_chain = iswap_chain;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_new_command_buffer(void* self, uint32_t command_queue_index, LunaRhiCommandBufferHandle* out_command_buffer)
{
    if (!self || !out_command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_command_buffer->object = nullptr;
    out_command_buffer->icommand_buffer = nullptr;

    Luna::RHI::IDevice* device = static_cast<Luna::RHI::IDevice*>(self);
    if (command_queue_index >= device->get_num_command_queues())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }

    auto result = device->new_command_buffer(command_queue_index);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::RHI::ICommandBuffer> command_buffer = Luna::move(result.get());
    Luna::object_t object = command_buffer.detach();
    void* icommand_buffer = Luna::query_interface<Luna::RHI::ICommandBuffer>(object);
    if (!icommand_buffer)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_command_buffer->object = object;
    out_command_buffer->icommand_buffer = icommand_buffer;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_desc(void* self, LunaRhiSwapChainDesc* out_desc)
{
    if (!self || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_desc = from_swap_chain_desc(static_cast<Luna::RHI::ISwapChain*>(self)->get_desc());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_window(void* self, LunaWindowHandle* out_window)
{
    if (!self || !out_window)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_window->object = nullptr;
    out_window->iwindow = nullptr;

    Luna::Window::IWindow* window = static_cast<Luna::RHI::ISwapChain*>(self)->get_window();
    Luna::object_t object = window ? window->get_object() : nullptr;
    if (!window || !object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::object_retain(object);
    out_window->object = object;
    out_window->iwindow = window;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_surface_transform(void* self, uint32_t* out_transform)
{
    if (!self || !out_transform)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_transform = static_cast<uint32_t>(static_cast<Luna::RHI::ISwapChain*>(self)->get_surface_transform());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_get_current_back_buffer(void* self, LunaRhiTextureHandle* out_texture)
{
    if (!self || !out_texture)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_texture->object = nullptr;
    out_texture->itexture = nullptr;

    auto result = static_cast<Luna::RHI::ISwapChain*>(self)->get_current_back_buffer();
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::RHI::ITexture* texture = result.get();
    Luna::object_t object = texture ? texture->get_object() : nullptr;
    if (!object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_texture->object = object;
    out_texture->itexture = texture;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_present(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::ISwapChain*>(self)->present());
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_reset_suggested(void* self, int32_t* out_suggested)
{
    if (!self || !out_suggested)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_suggested = static_cast<Luna::RHI::ISwapChain*>(self)->reset_suggested() ? 1 : 0;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_swap_chain_reset(void* self, const LunaRhiSwapChainDesc* desc)
{
    if (!self || !desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::ISwapChain*>(self)->reset(to_swap_chain_desc(*desc)));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_get_command_queue_index(void* self, uint32_t* out_index)
{
    if (!self || !out_index)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_index = static_cast<Luna::RHI::ICommandBuffer*>(self)->get_command_queue_index();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_reset(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::ICommandBuffer*>(self)->reset());
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_attach_device_object(void* self, luna_handle_t object)
{
    if (!self || !object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IDeviceChild* device_child = Luna::query_interface<Luna::RHI::IDeviceChild>(object);
    if (!device_child)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->attach_device_object(device_child);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_event(void* self, const char* event_name)
{
    if (!self || !event_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->begin_event(event_name);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_event(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->end_event();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_resource_barrier(
    void* self,
    const LunaRhiBufferBarrier* buffer_barriers,
    uint64_t buffer_barrier_count,
    const LunaRhiTextureBarrier* texture_barriers,
    uint64_t texture_barrier_count)
{
    if (!self || (buffer_barrier_count && !buffer_barriers) || (texture_barrier_count && !texture_barriers))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::BufferBarrier> native_buffer_barriers;
    Luna::Vector<Luna::RHI::TextureBarrier> native_texture_barriers;
    native_buffer_barriers.reserve(static_cast<Luna::usize>(buffer_barrier_count));
    native_texture_barriers.reserve(static_cast<Luna::usize>(texture_barrier_count));
    for (uint64_t i = 0; i < buffer_barrier_count; ++i)
    {
        native_buffer_barriers.push_back(to_buffer_barrier(buffer_barriers[i]));
    }
    for (uint64_t i = 0; i < texture_barrier_count; ++i)
    {
        native_texture_barriers.push_back(to_texture_barrier(texture_barriers[i]));
    }

    static_cast<Luna::RHI::ICommandBuffer*>(self)->resource_barrier(
        Luna::Span<const Luna::RHI::BufferBarrier>(native_buffer_barriers.data(), native_buffer_barriers.size()),
        Luna::Span<const Luna::RHI::TextureBarrier>(native_texture_barriers.data(), native_texture_barriers.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_render_pass(
    void* self,
    const LunaRhiColorAttachment* color_attachments,
    uint32_t color_attachment_count,
    const LunaRhiResolveAttachment* resolve_attachments,
    uint32_t resolve_attachment_count,
    const LunaRhiDepthStencilAttachment* depth_stencil_attachment,
    void* occlusion_query_heap,
    void* timestamp_query_heap,
    void* pipeline_statistics_query_heap,
    uint32_t timestamp_query_begin_pass_write_index,
    uint32_t timestamp_query_end_pass_write_index,
    uint32_t pipeline_statistics_query_write_index,
    uint32_t array_size,
    uint8_t sample_count)
{
    if (!self ||
        (color_attachment_count && !color_attachments) ||
        (resolve_attachment_count && !resolve_attachments) ||
        color_attachment_count > 8 ||
        resolve_attachment_count > 8)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::RenderPassDesc desc;
    for (uint32_t i = 0; i < color_attachment_count; ++i)
    {
        desc.color_attachments[i] = to_color_attachment(color_attachments[i]);
    }
    for (uint32_t i = 0; i < resolve_attachment_count; ++i)
    {
        desc.resolve_attachments[i] = to_resolve_attachment(resolve_attachments[i]);
    }
    if (depth_stencil_attachment)
    {
        desc.depth_stencil_attachment = to_depth_stencil_attachment(*depth_stencil_attachment);
    }
    desc.occlusion_query_heap = static_cast<Luna::RHI::IQueryHeap*>(occlusion_query_heap);
    desc.timestamp_query_heap = static_cast<Luna::RHI::IQueryHeap*>(timestamp_query_heap);
    desc.pipeline_statistics_query_heap = static_cast<Luna::RHI::IQueryHeap*>(pipeline_statistics_query_heap);
    desc.timestamp_query_begin_pass_write_index = timestamp_query_begin_pass_write_index;
    desc.timestamp_query_end_pass_write_index = timestamp_query_end_pass_write_index;
    desc.pipeline_statistics_query_write_index = pipeline_statistics_query_write_index;
    desc.array_size = array_size ? array_size : 1;
    desc.sample_count = sample_count ? sample_count : 1;
    static_cast<Luna::RHI::ICommandBuffer*>(self)->begin_render_pass(desc);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_pipeline_layout(void* self, void* pipeline_layout)
{
    if (!self || !pipeline_layout)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_graphics_pipeline_layout(static_cast<Luna::RHI::IPipelineLayout*>(pipeline_layout));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_pipeline_state(void* self, void* pipeline_state)
{
    if (!self || !pipeline_state)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_graphics_pipeline_state(static_cast<Luna::RHI::IPipelineState*>(pipeline_state));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_descriptor_set(void* self, uint32_t index, void* descriptor_set)
{
    if (!self || !descriptor_set)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_graphics_descriptor_set(index, static_cast<Luna::RHI::IDescriptorSet*>(descriptor_set));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_graphics_descriptor_sets(void* self, uint32_t start_index, const void* const* descriptor_sets, uint64_t descriptor_set_count)
{
    if (!self || (descriptor_set_count && !descriptor_sets))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::IDescriptorSet*> native_descriptor_sets;
    native_descriptor_sets.reserve(static_cast<Luna::usize>(descriptor_set_count));
    for (uint64_t i = 0; i < descriptor_set_count; ++i)
    {
        native_descriptor_sets.push_back(static_cast<Luna::RHI::IDescriptorSet*>(const_cast<void*>(descriptor_sets[i])));
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_graphics_descriptor_sets(
        start_index,
        Luna::Span<Luna::RHI::IDescriptorSet*>(native_descriptor_sets.data(), native_descriptor_sets.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_vertex_buffer(void* self, uint32_t slot, void* buffer, uint64_t offset, uint32_t size, uint32_t element_size)
{
    if (!self || !buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::RHI::VertexBufferView view(static_cast<Luna::RHI::IResource*>(static_cast<Luna::RHI::IBuffer*>(buffer)), offset, size, element_size);
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_vertex_buffers(slot, {view});
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_vertex_buffers(void* self, uint32_t start_slot, const LunaRhiVertexBufferView* views, uint64_t view_count)
{
    if (!self || (view_count && !views))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::VertexBufferView> native_views;
    native_views.reserve(static_cast<Luna::usize>(view_count));
    for (uint64_t i = 0; i < view_count; ++i)
    {
        native_views.push_back(to_vertex_buffer_view(views[i]));
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_vertex_buffers(
        start_slot,
        Luna::Span<const Luna::RHI::VertexBufferView>(native_views.data(), native_views.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_index_buffer(void* self, void* buffer, uint64_t offset, uint32_t size, uint32_t format)
{
    if (!self || !buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::RHI::IndexBufferView view(static_cast<Luna::RHI::IResource*>(static_cast<Luna::RHI::IBuffer*>(buffer)), offset, size, static_cast<Luna::RHI::Format>(format));
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_index_buffer(view);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_viewport(void* self, float top_left_x, float top_left_y, float width, float height, float min_depth, float max_depth)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_viewport(Luna::RHI::Viewport(top_left_x, top_left_y, width, height, min_depth, max_depth));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_viewports(void* self, const LunaRhiViewport* viewports, uint64_t viewport_count)
{
    if (!self || (viewport_count && !viewports))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::Viewport> native_viewports;
    native_viewports.reserve(static_cast<Luna::usize>(viewport_count));
    for (uint64_t i = 0; i < viewport_count; ++i)
    {
        native_viewports.push_back(to_viewport(viewports[i]));
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_viewports(
        Luna::Span<const Luna::RHI::Viewport>(native_viewports.data(), native_viewports.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_scissor_rect(void* self, int32_t offset_x, int32_t offset_y, int32_t width, int32_t height)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_scissor_rect(Luna::RectI(offset_x, offset_y, width, height));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_scissor_rects(void* self, const LunaRhiRectI* rects, uint64_t rect_count)
{
    if (!self || (rect_count && !rects))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RectI> native_rects;
    native_rects.reserve(static_cast<Luna::usize>(rect_count));
    for (uint64_t i = 0; i < rect_count; ++i)
    {
        native_rects.push_back(to_rect_i(rects[i]));
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_scissor_rects(
        Luna::Span<const Luna::RectI>(native_rects.data(), native_rects.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_blend_factor(void* self, float red, float green, float blue, float alpha)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_blend_factor(Luna::Float4U(red, green, blue, alpha));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_stencil_ref(void* self, uint32_t stencil_ref)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_stencil_ref(stencil_ref);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw(void* self, uint32_t vertex_count, uint32_t start_vertex_location)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->draw(vertex_count, start_vertex_location);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_indexed(void* self, uint32_t index_count, uint32_t start_index_location, int32_t base_vertex_location)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->draw_indexed(index_count, start_index_location, base_vertex_location);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_instanced(void* self, uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->draw_instanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_draw_indexed_instanced(void* self, uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->draw_indexed_instanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_occlusion_query(void* self, uint32_t mode, uint32_t index)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->begin_occlusion_query(static_cast<Luna::RHI::OcclusionQueryMode>(mode), index);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_occlusion_query(void* self, uint32_t index)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->end_occlusion_query(index);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_render_pass(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->end_render_pass();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_compute_pass(void* self, const LunaRhiComputePassDesc* desc)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::ComputePassDesc native_desc;
    if (desc)
    {
        native_desc.timestamp_query_heap = static_cast<Luna::RHI::IQueryHeap*>(desc->timestamp_query_heap);
        native_desc.pipeline_statistics_query_heap = static_cast<Luna::RHI::IQueryHeap*>(desc->pipeline_statistics_query_heap);
        native_desc.timestamp_query_begin_pass_write_index = desc->timestamp_query_begin_pass_write_index;
        native_desc.timestamp_query_end_pass_write_index = desc->timestamp_query_end_pass_write_index;
        native_desc.pipeline_statistics_query_write_index = desc->pipeline_statistics_query_write_index;
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->begin_compute_pass(native_desc);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_pipeline_layout(void* self, void* pipeline_layout)
{
    if (!self || !pipeline_layout)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_compute_pipeline_layout(static_cast<Luna::RHI::IPipelineLayout*>(pipeline_layout));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_pipeline_state(void* self, void* pipeline_state)
{
    if (!self || !pipeline_state)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_compute_pipeline_state(static_cast<Luna::RHI::IPipelineState*>(pipeline_state));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_descriptor_set(void* self, uint32_t index, void* descriptor_set)
{
    if (!self || !descriptor_set)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_compute_descriptor_set(index, static_cast<Luna::RHI::IDescriptorSet*>(descriptor_set));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_set_compute_descriptor_sets(void* self, uint32_t start_index, const void* const* descriptor_sets, uint64_t descriptor_set_count)
{
    if (!self || (descriptor_set_count && !descriptor_sets))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::IDescriptorSet*> native_descriptor_sets;
    native_descriptor_sets.reserve(static_cast<Luna::usize>(descriptor_set_count));
    for (uint64_t i = 0; i < descriptor_set_count; ++i)
    {
        native_descriptor_sets.push_back(static_cast<Luna::RHI::IDescriptorSet*>(const_cast<void*>(descriptor_sets[i])));
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->set_compute_descriptor_sets(
        start_index,
        Luna::Span<Luna::RHI::IDescriptorSet*>(native_descriptor_sets.data(), native_descriptor_sets.size()));
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_dispatch(void* self, uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_compute_pass(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->end_compute_pass();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_begin_copy_pass(void* self, const LunaRhiCopyPassDesc* desc)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::CopyPassDesc native_desc;
    if (desc)
    {
        native_desc.timestamp_query_heap = static_cast<Luna::RHI::IQueryHeap*>(desc->timestamp_query_heap);
        native_desc.timestamp_query_begin_pass_write_index = desc->timestamp_query_begin_pass_write_index;
        native_desc.timestamp_query_end_pass_write_index = desc->timestamp_query_end_pass_write_index;
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->begin_copy_pass(native_desc);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_resource(void* self, luna_handle_t dst_object, luna_handle_t src_object)
{
    if (!self || !dst_object || !src_object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::RHI::IResource* dst = Luna::query_interface<Luna::RHI::IResource>(dst_object);
    Luna::RHI::IResource* src = Luna::query_interface<Luna::RHI::IResource>(src_object);
    if (!dst || !src)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->copy_resource(dst, src);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_buffer(void* self, void* dst, uint64_t dst_offset, void* src, uint64_t src_offset, uint64_t copy_bytes)
{
    if (!self || !dst || !src)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->copy_buffer(
        static_cast<Luna::RHI::IBuffer*>(dst),
        dst_offset,
        static_cast<Luna::RHI::IBuffer*>(src),
        src_offset,
        copy_bytes);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_texture(
    void* self,
    void* dst,
    LunaRhiSubresourceIndex dst_subresource,
    uint32_t dst_x,
    uint32_t dst_y,
    uint32_t dst_z,
    void* src,
    LunaRhiSubresourceIndex src_subresource,
    uint32_t src_x,
    uint32_t src_y,
    uint32_t src_z,
    uint32_t copy_width,
    uint32_t copy_height,
    uint32_t copy_depth)
{
    if (!self || !dst || !src)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->copy_texture(
        static_cast<Luna::RHI::ITexture*>(dst),
        to_subresource_index(dst_subresource),
        dst_x,
        dst_y,
        dst_z,
        static_cast<Luna::RHI::ITexture*>(src),
        to_subresource_index(src_subresource),
        src_x,
        src_y,
        src_z,
        copy_width,
        copy_height,
        copy_depth);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_buffer_to_texture(
    void* self,
    void* dst,
    LunaRhiSubresourceIndex dst_subresource,
    uint32_t dst_x,
    uint32_t dst_y,
    uint32_t dst_z,
    void* src,
    uint64_t src_offset,
    uint32_t src_row_pitch,
    uint32_t src_slice_pitch,
    uint32_t copy_width,
    uint32_t copy_height,
    uint32_t copy_depth)
{
    if (!self || !dst || !src)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->copy_buffer_to_texture(
        static_cast<Luna::RHI::ITexture*>(dst),
        to_subresource_index(dst_subresource),
        dst_x,
        dst_y,
        dst_z,
        static_cast<Luna::RHI::IBuffer*>(src),
        src_offset,
        src_row_pitch,
        src_slice_pitch,
        copy_width,
        copy_height,
        copy_depth);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_copy_texture_to_buffer(
    void* self,
    void* dst,
    uint64_t dst_offset,
    uint32_t dst_row_pitch,
    uint32_t dst_slice_pitch,
    void* src,
    LunaRhiSubresourceIndex src_subresource,
    uint32_t src_x,
    uint32_t src_y,
    uint32_t src_z,
    uint32_t copy_width,
    uint32_t copy_height,
    uint32_t copy_depth)
{
    if (!self || !dst || !src)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->copy_texture_to_buffer(
        static_cast<Luna::RHI::IBuffer*>(dst),
        dst_offset,
        dst_row_pitch,
        dst_slice_pitch,
        static_cast<Luna::RHI::ITexture*>(src),
        to_subresource_index(src_subresource),
        src_x,
        src_y,
        src_z,
        copy_width,
        copy_height,
        copy_depth);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_end_copy_pass(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->end_copy_pass();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_submit(void* self, int32_t allow_host_waiting)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::ICommandBuffer*>(self)->submit({}, {}, allow_host_waiting != 0));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_submit_with_fences(
    void* self,
    const void* const* wait_fences,
    uint64_t wait_fence_count,
    const void* const* signal_fences,
    uint64_t signal_fence_count,
    int32_t allow_host_waiting)
{
    if (!self || (wait_fence_count && !wait_fences) || (signal_fence_count && !signal_fences))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::IFence*> native_wait_fences;
    native_wait_fences.reserve(static_cast<Luna::usize>(wait_fence_count));
    for (uint64_t i = 0; i < wait_fence_count; ++i)
    {
        native_wait_fences.push_back(static_cast<Luna::RHI::IFence*>(const_cast<void*>(wait_fences[i])));
    }

    Luna::Vector<Luna::RHI::IFence*> native_signal_fences;
    native_signal_fences.reserve(static_cast<Luna::usize>(signal_fence_count));
    for (uint64_t i = 0; i < signal_fence_count; ++i)
    {
        native_signal_fences.push_back(static_cast<Luna::RHI::IFence*>(const_cast<void*>(signal_fences[i])));
    }

    return from_result(static_cast<Luna::RHI::ICommandBuffer*>(self)->submit(
        Luna::Span<Luna::RHI::IFence*>(native_wait_fences.data(), native_wait_fences.size()),
        Luna::Span<Luna::RHI::IFence*>(native_signal_fences.data(), native_signal_fences.size()),
        allow_host_waiting != 0));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_wait(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::ICommandBuffer*>(self)->wait();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_command_buffer_try_wait(void* self, int32_t* out_waited)
{
    if (!self || !out_waited)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_waited = static_cast<Luna::RHI::ICommandBuffer*>(self)->try_wait() ? 1 : 0;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_get_desc(void* self, LunaRhiBufferDesc* out_desc)
{
    if (!self || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_desc = from_buffer_desc(static_cast<Luna::RHI::IBuffer*>(self)->get_desc());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_map(void* self, uint64_t read_begin, uint64_t read_end, void** out_data)
{
    if (!self || !out_data)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::IBuffer*>(self)->map(read_begin, read_end, out_data));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_buffer_unmap(void* self, uint64_t write_begin, uint64_t write_end)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    static_cast<Luna::RHI::IBuffer*>(self)->unmap(write_begin, write_end);
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_resource_get_memory(luna_handle_t object, LunaRhiDeviceMemoryHandle* out_memory)
{
    if (!object || !out_memory)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_memory->object = nullptr;
    out_memory->idevice_memory = nullptr;

    Luna::RHI::IResource* resource = Luna::query_interface<Luna::RHI::IResource>(object);
    if (!resource)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::RHI::IDeviceMemory* memory = resource->get_memory();
    Luna::object_t memory_object = memory ? memory->get_object() : nullptr;
    if (!memory || !memory_object)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }

    Luna::object_retain(memory_object);
    out_memory->object = memory_object;
    out_memory->idevice_memory = memory;
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_memory_get_memory_type(void* self, uint32_t* out_memory_type)
{
    if (!self || !out_memory_type)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_memory_type = static_cast<uint32_t>(static_cast<Luna::RHI::IDeviceMemory*>(self)->get_memory_type());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_device_memory_get_size(void* self, uint64_t* out_size)
{
    if (!self || !out_size)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_size = static_cast<Luna::RHI::IDeviceMemory*>(self)->get_size();
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_desc(void* self, LunaRhiQueryHeapDesc* out_desc)
{
    if (!self || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_desc = from_query_heap_desc(static_cast<Luna::RHI::IQueryHeap*>(self)->get_desc());
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_timestamp_values(void* self, uint32_t index, uint32_t count, uint64_t* out_values)
{
    if (!self || (count && !out_values))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::IQueryHeap*>(self)->get_timestamp_values(index, count, out_values));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_occlusion_values(void* self, uint32_t index, uint32_t count, uint64_t* out_values)
{
    if (!self || (count && !out_values))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(static_cast<Luna::RHI::IQueryHeap*>(self)->get_occlusion_values(index, count, out_values));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_query_heap_get_pipeline_statistics_values(void* self, uint32_t index, uint32_t count, LunaRhiPipelineStatistics* out_values)
{
    if (!self || (count && !out_values))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::PipelineStatistics> native_values;
    native_values.resize(count);
    Luna::RV result = static_cast<Luna::RHI::IQueryHeap*>(self)->get_pipeline_statistics_values(index, count, native_values.data());
    if (!result.valid())
    {
        return from_result(result);
    }
    for (uint32_t i = 0; i < count; ++i)
    {
        out_values[i] = from_pipeline_statistics(native_values[i]);
    }
    return 0;
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_buffer_view(void* self, uint32_t binding_slot, uint32_t first_array_index, uint32_t descriptor_type, const LunaRhiBufferViewDesc* views, uint32_t view_count)
{
    if (!self || (view_count && !views))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::BufferViewDesc> native_views;
    native_views.reserve(view_count);
    for (uint32_t i = 0; i < view_count; ++i)
    {
        native_views.push_back(to_buffer_view_desc(views[i]));
    }

    Luna::RHI::WriteDescriptorSet write;
    write.binding_slot = binding_slot;
    write.first_array_index = first_array_index;
    write.num_descs = view_count;
    write.type = static_cast<Luna::RHI::DescriptorType>(descriptor_type);
    write.buffer_views = native_views.data();
    return from_result(static_cast<Luna::RHI::IDescriptorSet*>(self)->update_descriptors({write}));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_texture_view(void* self, uint32_t binding_slot, uint32_t first_array_index, uint32_t descriptor_type, const LunaRhiTextureViewDesc* views, uint32_t view_count)
{
    if (!self || (view_count && !views))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::TextureViewDesc> native_views;
    native_views.reserve(view_count);
    for (uint32_t i = 0; i < view_count; ++i)
    {
        native_views.push_back(to_texture_view_desc(views[i]));
    }

    Luna::RHI::WriteDescriptorSet write;
    write.binding_slot = binding_slot;
    write.first_array_index = first_array_index;
    write.num_descs = view_count;
    write.type = static_cast<Luna::RHI::DescriptorType>(descriptor_type);
    write.texture_views = native_views.data();
    return from_result(static_cast<Luna::RHI::IDescriptorSet*>(self)->update_descriptors({write}));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_descriptor_set_update_sampler(void* self, uint32_t binding_slot, uint32_t first_array_index, const LunaRhiSamplerDesc* samplers, uint32_t sampler_count)
{
    if (!self || (sampler_count && !samplers))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::RHI::SamplerDesc> native_samplers;
    native_samplers.reserve(sampler_count);
    for (uint32_t i = 0; i < sampler_count; ++i)
    {
        native_samplers.push_back(to_sampler_desc(samplers[i]));
    }

    Luna::RHI::WriteDescriptorSet write;
    write.binding_slot = binding_slot;
    write.first_array_index = first_array_index;
    write.num_descs = sampler_count;
    write.type = Luna::RHI::DescriptorType::sampler;
    write.samplers = native_samplers.data();
    return from_result(static_cast<Luna::RHI::IDescriptorSet*>(self)->update_descriptors({write}));
}

LUNA_RHI_C_API luna_errcode_t luna_rhi_texture_get_desc(void* self, LunaRhiTextureDesc* out_desc)
{
    if (!self || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_desc = from_texture_desc(static_cast<Luna::RHI::ITexture*>(self)->get_desc());
    return 0;
}
}
