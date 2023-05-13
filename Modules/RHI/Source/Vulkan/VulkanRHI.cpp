/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file VulkanRHI.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include "VulkanRHI.hpp"
#include <Runtime/HashSet.hpp>
#include "../RHI.hpp"
#include "Device.hpp"
#include <Window/Window.hpp>
#include "Instance.hpp"
#include "Adapter.hpp"
#include "CommandBuffer.hpp"
#include "DepthStencilView.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Fence.hpp"
#include "PipelineState.hpp"
#include "QueryHeap.hpp"
#include "RenderTargetView.hpp"
#include "ResolveTargetView.hpp"
#include "Resource.hpp"
#include "Sampler.hpp"
#include "ShaderInputLayout.hpp"
#include "SwapChain.hpp"
namespace Luna
{
	namespace RHI
	{
		RV render_api_init()
		{
			VkSurfaceKHR dummy_surface = VK_NULL_HANDLE;
			Ref<Window::IWindow> dummy_window;
			lutry
			{
				register_boxed_type<CommandBuffer>();
				impl_interface_for_type<CommandBuffer, ICommandBuffer, IDeviceChild, IWaitable>();
				register_boxed_type<DepthStencilView>();
				impl_interface_for_type<DepthStencilView, IDepthStencilView, IDeviceChild>();
				register_boxed_type<DescriptorSet>();
				impl_interface_for_type<DescriptorSet, IDescriptorSet, IDeviceChild>();
				register_boxed_type<DescriptorSetLayout>();
				impl_interface_for_type<DescriptorSetLayout, IDescriptorSetLayout, IDeviceChild>();
				register_boxed_type<Device>();
				impl_interface_for_type<Device, IDevice>();
				register_boxed_type<DeviceMemory>();
				register_boxed_type<Fence>();
				impl_interface_for_type<Fence, IFence, IDeviceChild>();
				register_boxed_type<ImageView>();
				register_boxed_type<PipelineState>();
				impl_interface_for_type<PipelineState, IPipelineState, IDeviceChild>();
				register_boxed_type<QueryHeap>();
				impl_interface_for_type<QueryHeap, IQueryHeap, IDeviceChild>();
				register_boxed_type<RenderTargetView>();
				impl_interface_for_type<RenderTargetView, IRenderTargetView, IDeviceChild>();
				register_boxed_type<ResolveTargetView>();
				impl_interface_for_type<ResolveTargetView, IResolveTargetView, IDeviceChild>();
				register_boxed_type<BufferResource>();
				impl_interface_for_type<BufferResource, IBuffer, IResource, IDeviceChild>();
				register_boxed_type<ImageResource>();
				impl_interface_for_type<ImageResource, ITexture, IResource, IDeviceChild>();
				register_boxed_type<Sampler>();
				register_boxed_type<ShaderInputLayout>();
				impl_interface_for_type<ShaderInputLayout, IShaderInputLayout, IDeviceChild>();
				register_boxed_type<SwapChain>();
				impl_interface_for_type<SwapChain, ISwapChain, IDeviceChild>();

				luexp(create_vk_instance());
				luexp(init_physical_devices());
				lulet(main_physical_device, select_main_physical_device());
				Ref<Device> dev = new_object<Device>();
				luexp(dev->init(g_physical_devices[main_physical_device], g_physical_device_queue_families[main_physical_device]));
				g_main_device = dev;
			}
			lucatchret;
			return ok;
		}
		void render_api_close()
		{
			g_main_device.reset();
			destroy_vk_instance();
		}
	}
}