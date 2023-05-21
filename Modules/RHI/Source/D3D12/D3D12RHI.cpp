/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file D3D12RHI.cpp
* @author JXMaster
* @date 2019/7/10
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../../RHI.hpp"
#include <d3d12.h>
#include "Device.hpp"
#include "SwapChain.hpp"
#include <dxgi1_5.h>
#include <Runtime/Unicode.hpp>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "../RHI.hpp"

#include "ShaderInputLayout.hpp"
#include "PipelineState.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"

namespace Luna
{
	namespace RHI
	{
		ComPtr<IDXGIFactory5> g_dxgi;
		Ref<IDevice> g_device;

		Vector<ComPtr<IDXGIAdapter1>> g_adapters;

		RV render_api_init()
		{
			register_boxed_type<BufferResource>();
			impl_interface_for_type<BufferResource, IBuffer, IResource, IDeviceChild>();
			register_boxed_type<TextureResource>();
			impl_interface_for_type<TextureResource, ITexture, IResource, IDeviceChild>();
			register_boxed_type<DeviceMemory>();
			impl_interface_for_type<DeviceMemory, IDeviceMemory, IDeviceChild>();
			register_boxed_type<ShaderInputLayout>();
			impl_interface_for_type<ShaderInputLayout, IShaderInputLayout, IDeviceChild>();
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

			ComPtr<IDXGIAdapter1> ada;
			u32 index = 0;
			while (true)
			{
				hr = g_dxgi->EnumAdapters1(index, ada.ReleaseAndGetAddressOf());
				if (FAILED(hr)) break;
				g_adapters.push_back(move(ada));
				++index;
			}

#if defined(LUNA_RHI_DEBUG) && (LUNA_PLATFORM_VERSION >= LUNA_PLATFORM_VERSION_WIN10)
			ComPtr<ID3D12Debug> debug;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
			if (debug) debug->EnableDebugLayer();
#endif
			auto dev = new_device(0);
			if (failed(dev)) return dev.errcode();
			g_device = dev.get();
			return ok;
		}
		void render_api_close()
		{
			g_device = nullptr;
			g_dxgi = nullptr;
			g_adapters.clear();
			g_adapters.shrink_to_fit();
		}
		LUNA_RHI_API u32 get_num_adapters()
		{
			return (u32)g_adapters.size();
		}
		LUNA_RHI_API AdapterDesc get_adapter_desc(u32 index)
		{
			DXGI_ADAPTER_DESC1 desc;
			g_adapters[index]->GetDesc1(&desc);
			AdapterDesc dst;
			utf16_to_utf8(dst.name, 256, (char16_t*)desc.Description);
			dst.local_memory = desc.DedicatedSystemMemory + desc.DedicatedVideoMemory;
			dst.shared_memory = desc.SharedSystemMemory;
			dst.type = AdapterType::discrete_gpu;
			if (!desc.DedicatedVideoMemory)
			{
				dst.type = AdapterType::integrated_gpu;
			}
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				dst.type = AdapterType::software;
			}
			return dst;
		}
		LUNA_RHI_API R<Ref<IDevice>> new_device(u32 adapter_index)
		{
			ComPtr<ID3D12Device> dev;
			Ref<Device> device = new_object<Device>();
			auto res = device->init(g_adapters[adapter_index].Get());
			if (failed(res)) return res.errcode();
			return device;
		}
		LUNA_RHI_API IDevice* get_main_device()
		{
			return g_device;
		}
		LUNA_RHI_API APIType get_current_platform_api_type()
		{
			return APIType::d3d12;
		}
	}
}