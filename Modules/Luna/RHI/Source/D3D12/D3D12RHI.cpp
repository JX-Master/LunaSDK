/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file D3D12RHI.cpp
* @author JXMaster
* @date 2019/7/10
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../../RHI.hpp"
#include <d3d12.h>
#include "Device.hpp"
#include "SwapChain.hpp"
#include <Luna/Runtime/Unicode.hpp>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "../RHI.hpp"

#include "PipelineLayout.hpp"
#include "PipelineState.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"
#include "Adapter.hpp"

namespace Luna
{
	namespace RHI
	{
		ComPtr<IDXGIFactory5> g_dxgi;
		Ref<IDevice> g_main_device;

		RV render_api_init()
		{
			register_boxed_type<Adapter>();
			impl_interface_for_type<Adapter, IAdapter>();
			register_boxed_type<BufferResource>();
			impl_interface_for_type<BufferResource, IBuffer, IResource, IDeviceChild>();
			register_boxed_type<TextureResource>();
			impl_interface_for_type<TextureResource, ITexture, IResource, IDeviceChild>();
			register_boxed_type<DeviceMemory>();
			impl_interface_for_type<DeviceMemory, IDeviceMemory, IDeviceChild>();
			register_boxed_type<PipelineLayout>();
			impl_interface_for_type<PipelineLayout, IPipelineLayout, IDeviceChild>();
			register_boxed_type<PipelineState>();
			impl_interface_for_type<PipelineState, IPipelineState, IDeviceChild>();
			register_boxed_type<CommandBuffer>();
			impl_interface_for_type<CommandBuffer, ICommandBuffer, IDeviceChild, IWaitable>();
			register_boxed_type<Device>();
			impl_interface_for_type<Device, IDevice>();
			register_boxed_type<SwapChain>();
			impl_interface_for_type<SwapChain, ISwapChain, IDeviceChild>();
			register_boxed_type<DescriptorSetLayout>();
			impl_interface_for_type<DescriptorSetLayout, IDescriptorSetLayout, IDeviceChild>();
			register_boxed_type<DescriptorSet>();
			impl_interface_for_type<DescriptorSet, IDescriptorSet, IDeviceChild>();
			register_boxed_type<QueryHeap>();
			impl_interface_for_type<QueryHeap, IQueryHeap, IDeviceChild>();
			register_boxed_type<Fence>();
			impl_interface_for_type<Fence, IFence, IDeviceChild>();

			HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgi));
			if (FAILED(hr))
			{
				return encode_hresult(hr);
			}
			lutry
			{
				luexp(init_adapters());
				auto adapters = get_adapters();
	#if (defined(LUNA_RHI_DEBUG) || defined(LUNA_DEBUG)) && (LUNA_PLATFORM_VERSION >= LUNA_PLATFORM_VERSION_WIN10)
				ComPtr<ID3D12Debug> debug;
				D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
				if (debug) debug->EnableDebugLayer();
	#endif
				luset(g_main_device, new_device(adapters[0]));
			}
			lucatchret;
			return ok;
		}
		void render_api_close()
		{
			g_main_device = nullptr;
			g_adapters.clear();
			g_adapters.shrink_to_fit();
			g_dxgi = nullptr;
		}
		LUNA_RHI_API BackendType get_backend_type()
		{
			return BackendType::d3d12;
		}
	}
}