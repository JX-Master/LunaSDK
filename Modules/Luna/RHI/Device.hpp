/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include "PipelineLayout.hpp"
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "Fence.hpp"
#include "QueryHeap.hpp"
#include "Adapter.hpp"

#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif

namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{
        
        //! Specifies device features that can be queried at run time.
        enum class DeviceFeature : u32
        {
            //! @ref DescriptorSetLayoutFlag::variable_descriptors is allowed when creating descriptor set layout.
            unbound_descriptor_array,
            //! Allow pixel shaders to write and perform atomic operations on buffer and texture data.
            pixel_shader_write,
            //! The alignment requiremtn for the buffer data start location and size.
            uniform_buffer_data_alignment,
        };

        //! Represents the device feature check result.
        struct DeviceFeatureData
        {
            union
            {
                //! The feature check result of @ref DeviceFeature::unbound_descriptor_array.
                bool unbound_descriptor_array;
                //! The feature check result of @ref DeviceFeature::pixel_shader_write.
                bool pixel_shader_write;
                //! The feature check result of @ref DeviceFeature::uniform_buffer_data_alignment.
                u32 uniform_buffer_data_alignment;
            };
        };

        //! Specifies command queue types.
        enum class CommandQueueType : u8
        {
            //! This command queue can be used for submitting graphics, compute and copy commands.
            graphics = 1,
            //! This command queue can be used for submitting compute and copy commands.
            compute = 2,
            //! This command queue can be used for submitting copy commands.
            copy = 3,
        };

        //! Specifies additional flags for command queues.
        enum class CommandQueueFlag : u8
        {
            none = 0,
            //! This command queue supports swap chain presenting commands.
            presenting = 0x01,
        };

        //! Describes one command queue.
        struct CommandQueueDesc
        {
            //! The command queue type.
            CommandQueueType type;
            //! The additional command queue flags.
            CommandQueueFlag flags;

            CommandQueueDesc() = default;
            CommandQueueDesc(CommandQueueType type, CommandQueueFlag flags) :
                type(type),
                flags(flags) {}
        };

        struct IAdapter;

        //! @interface IDevice
        //! Represents one logical graphic device on the platform.
        struct IDevice : virtual Interface
        {
            luiid("{099AB8FA-7239-41EE-B05C-D36B5DCE1ED7}");

            //! Checks device feature.
            //! @param[in] feature The device feature to check.
            //! @return Returns the device feature check result.
            virtual DeviceFeatureData check_feature(DeviceFeature feature) = 0;

            //! Gets the texture data placement information when storing texture data in a buffer. 
            //! The texture data is arranged in row-major order.
            //! @param[in] width The width of the texture data.
            //! @param[in] height The height of the texture data.
            //! @param[in] depth The depth of the texture data.
            //! @param[in] format The format of the texture data.
            //! @param[out] size The size of the texture data in the buffer. Specify `nullptr` if this is not needed.
            //! @param[out] alignment The alignment requirement of the texture data. Specify `nullptr` if this is not needed.
            //! @param[out] row_pitch The row pitch of the texture data. Specify `nullptr` if this is not needed.
            //! @param[out] slice_pitch The slice (row * column) pitch of the texture data. Specify `nullptr` if this is not needed.
            virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
                u64* size = nullptr, u64* alignment = nullptr, u64* row_pitch = nullptr, u64* slice_pitch = nullptr) = 0;

            //! Creates one new buffer resource and allocates device memory for the resource.
            //! @param[in] memory_type The memory type selected for allocating resource memory.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created buffer object.
            virtual R<Ref<IBuffer>> new_buffer(MemoryType memory_type, const BufferDesc& desc) = 0;

            //! Creates one new texture resource and allocates device memory for the resource.
            //! @param[in] memory_type The memory type selected for allocating resource memory.
            //! @param[in] desc The descriptor object.
            //! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if 
            //! the resource does not have a optimized clear value.
            //! @return Returns the created texture object.
            virtual R<Ref<ITexture>> new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

            //! Checks whether the given resources can share the same device memory.
            //! @details This can be used to check whether the specified resources can be allocated from the same device memory 
            //! without actually allocating such memory.
            //! @param[in] memory_type The memory type for the given buffers and textures.
            //! @param[in] buffers The buffer descriptors of resources being examined.
            //! @param[in] textures The texture descriptors of resources being examined.
            //! @return Returns `true` if such resources can share the same device memory, returns `false` otherwise.
            virtual bool is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) = 0;

            //! Allocates device memory that is capable of storing resources specified.
            //! @param[in] memory_type The memory type of the memory to allocate.
            //! @param[in] buffers One array of descriptors of buffers that once the allocation succeeds,
            //! can be created in the memory.
            //! @param[in] textures One array of descriptors of textures that once the allocation succeeds,
            //! can be created in the memory.
            //! @return Returns the allocated memory.
            virtual R<Ref<IDeviceMemory>> allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) = 0;

            //! Creates one aliasing buffer that shares the same device memory with the existing resource.
            //! @details The user may create multiple aliasing resources with the same device memory, given that only one of them is active at any given time.
            //! The user should use aliasing barrier to switch the active resource between aliasing resources sharing the same device memory.
            //! @param[in] device_memory The device memory that the new resource is created in.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created buffer object.
            virtual R<Ref<IBuffer>> new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc) = 0;

            //! Creates one aliasing texture that shares the same device memory with the existing resource.
            //! @details The user may create multiple aliasing resources with the same device memory, given that only one of them is active at any given time.
            //! The user should use aliasing barrier to switch the active resource between aliasing resources sharing the same device memory.
            //! @param[in] device_memory The device memory that the new resource is created in.
            //! @param[in] desc The descriptor object.
            //! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if 
            //! the resource does not have a optimized clear value.
            //! @return Returns the created texture object.
            virtual R<Ref<ITexture>> new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

            //! Creates one new pipeline layout.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created pipeline layout object.
            virtual R<Ref<IPipelineLayout>> new_pipeline_layout(const PipelineLayoutDesc& desc) = 0;

            //! Creates one new graphic pipeline state.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created graphic pipeline state object.
            virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) = 0;

            //! Creates one compute pipeline state.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created compute pipeline state object.
            virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) = 0;

            //! Creates one new descriptor set layout object that can be used to create descriptor sets.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created descriptor set layout object.
            virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) = 0;

            //! Creates one new descriptor set object that describes resources that are bound to the pipeline.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created descriptor set object.
            virtual R<Ref<IDescriptorSet>> new_descriptor_set(const DescriptorSetDesc& desc) = 0;

            //! Gets the number of command queues of the device.
            //! @return Returns the number of command queues of the device.
            virtual u32 get_num_command_queues() = 0;

            //! Gets the command queue descriptor of the specified command queue.
            //! @param[in] command_queue_index The index of the command queue to check.
            //! @return Returns the command queue descriptor of the specified command queue.
            //! @par Valid Usage
            //! * `command_queue_index` must be in range [`0`, `get_num_command_queues()`).
            virtual CommandQueueDesc get_command_queue_desc(u32 command_queue_index) = 0;

            //! Creates one command buffer.
            //! @param[in] command_queue_index The index of the command queue that will be attached to the 
            //! command buffer. The command buffer can only submit commands to its attached command queue.
            //! @return Returns the created command buffer object.
            //! @par Valid Usage
            //! * `command_queue_index` must be in range [`0`, `get_num_command_queues()`).
            virtual R<Ref<ICommandBuffer>> new_command_buffer(u32 command_queue_index) = 0;

            //! Gets the GPU timestamp frequency of the specified command queue. 
            //! The timestamp frequency is measured in ticks per second.
            //! @param[in] command_queue_index The index of the command queue to check.
            //! @return Returns the GPU timestamp frequency of the specified command queue. 
            //! @par Valid Usage
            //! * `command_queue_index` must be in range [`0`, `get_num_command_queues()`).
            virtual R<f64> get_command_queue_timestamp_frequency(u32 command_queue_index) = 0;

            //! Creates one new query heap that can be used to store GPU query result.
            //! @param[in] desc The descriptor object.
            //! @return Returns the created query heap object.
            virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) = 0;

            //! Creates one new fence that can be used to synchronize execution of multiple command buffers.
            //! @return Returns the created fence object.
            virtual R<Ref<IFence>> new_fence() = 0;

            //! Creates one swap chain and binds it to the specified window.
            //! @param[in] command_queue_index The command queue attached to the swap chain. Present commands will only be 
            //! submitted to the command queue attached with the swap chain.
            //! @param[in] window The window that the new swap chain should be bound to.
            //! @param[in] desc The swap chain descriptor object.
            //! @return Returns the new created swap chain object.
            //! @par Valid Usage
            //! * `command_queue_index` must be in range [`0`, `get_num_command_queues()`).
            //! * The swap chain specified by `command_queue_index` must have @ref CommandQueueFlag::presenting being set.
            virtual R<Ref<ISwapChain>> new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc) = 0;
        };

        //! Creates one device using the specified adapter.
        //! @param[in] adapter The adapter used for creating the device.
        //! @return Returns the created device object.
        LUNA_RHI_API R<Ref<IDevice>> new_device(IAdapter* adapter);

        //! Gets the main device of the platform.
        //! @return Returns the main device of the platform.
        LUNA_RHI_API IDevice* get_main_device();

        //! @}
    }
}