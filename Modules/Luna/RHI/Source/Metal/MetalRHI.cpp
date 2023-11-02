/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MetalRHI.cpp
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../RHI.hpp"
#include "Device.hpp"
#include "Adapter.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "Resource.hpp"
#include "Fence.hpp"
#include "PipelineState.hpp"
#include "QueryHeap.hpp"
#include "PipelineLayout.hpp"
#include "SwapChain.hpp"
namespace Luna
{
    namespace RHI
    {
        RV render_api_init()
        {
            lutry
            {
                register_boxed_type<Adapter>();
                impl_interface_for_type<Adapter, IAdapter>();
                register_boxed_type<CommandBuffer>();
                impl_interface_for_type<CommandBuffer, ICommandBuffer, IDeviceChild, IWaitable>();
                register_boxed_type<DescriptorSet>();
                impl_interface_for_type<DescriptorSet, IDescriptorSet, IDeviceChild>();
                register_boxed_type<DescriptorSetLayout>();
                impl_interface_for_type<DescriptorSetLayout, IDescriptorSetLayout, IDeviceChild>();
                register_boxed_type<Device>();
                impl_interface_for_type<Device, IDevice>();
                register_boxed_type<DeviceMemory>();
                impl_interface_for_type<DeviceMemory, IDeviceMemory, IDeviceChild>();
                register_boxed_type<Fence>();
                impl_interface_for_type<Fence, IFence, IDeviceChild>();
                register_boxed_type<RenderPipelineState>();
                impl_interface_for_type<RenderPipelineState, IPipelineState, IDeviceChild>();
                register_boxed_type<ComputePipelineState>();
                impl_interface_for_type<ComputePipelineState, IPipelineState, IDeviceChild>();
                register_boxed_type<BufferQueryHeap>();
                impl_interface_for_type<BufferQueryHeap, IQueryHeap, IDeviceChild>();
                register_boxed_type<CounterSampleQueryHeap>();
                impl_interface_for_type<CounterSampleQueryHeap, IQueryHeap, IDeviceChild>();
                register_boxed_type<Buffer>();
                impl_interface_for_type<Buffer, IBuffer, IResource, IDeviceChild>();
                register_boxed_type<Texture>();
                impl_interface_for_type<Texture, ITexture, IResource, IDeviceChild>();
                register_boxed_type<PipelineLayout>();
                impl_interface_for_type<PipelineLayout, IPipelineLayout, IDeviceChild>();
                register_boxed_type<SwapChain>();
                impl_interface_for_type<SwapChain, ISwapChain, IDeviceChild>();
                register_boxed_type<TextureView>();
                init_adapters();
                luexp(init_main_device());
            }
            lucatchret;
            return ok;
        }
		void render_api_close()
        {
            g_main_device.reset();
            g_adapters.clear();
            g_adapters.shrink_to_fit();
        }
        LUNA_RHI_API BackendType get_backend_type()
		{
			return BackendType::metal;
		}
    }
}
