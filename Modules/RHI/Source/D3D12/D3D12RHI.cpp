/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file D3D12RHI.cpp
* @author JXMaster
* @date 2019/7/10
*/
#include <Runtime/PlatformDefines.hpp>
#ifdef LUNA_RHI_D3D12
#define LUNA_RHI_API LUNA_EXPORT
#include "../../RHI.hpp"
#include <d3d12.h>
#include "Device.hpp"
#include "SwapChain.hpp"
#include <dxgi1_4.h>
#include <Runtime/Unicode.hpp>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "../RHI.hpp"

#include "ResourceHeap.hpp"
#include "ShaderInputLayout.hpp"
#include "PipelineState.hpp"
#include "CommandBuffer.hpp"
#include "CommandQueue.hpp"
#include "SwapChain.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		ComPtr<IDXGIFactory1> g_dxgi;
		Ref<IDevice> g_device;

		RV render_api_init()
		{
			register_boxed_type<Resource>();
			impl_interface_for_type<Resource, IDeviceChild, IResource>();
			register_boxed_type<ResourceHeap>();
			impl_interface_for_type<ResourceHeap, IDeviceChild, IResourceHeap>();
			register_boxed_type<ShaderInputLayout>();
			impl_interface_for_type<ShaderInputLayout, IShaderInputLayout, IDeviceChild>();
			register_boxed_type<RenderTargetView>();
			impl_interface_for_type<RenderTargetView, IRenderTargetView, IDeviceChild>();
			register_boxed_type<DepthStencilView>();
			impl_interface_for_type<DepthStencilView, IDepthStencilView, IDeviceChild>();
			register_boxed_type<PipelineState>();
			impl_interface_for_type<PipelineState, IPipelineState, IDeviceChild>();
			register_boxed_type<CommandBuffer>();
			impl_interface_for_type<CommandBuffer, ICommandBuffer, IDeviceChild, IWaitable>();
			register_boxed_type<CommandQueue>();
			impl_interface_for_type<CommandQueue, ICommandQueue, IDeviceChild>();
			register_boxed_type<Device>();
			impl_interface_for_type<Device, IDevice>();
			register_boxed_type<SwapChain>();
			impl_interface_for_type<SwapChain, ISwapChain, IDeviceChild, IWaitable>();
			register_boxed_type<DescriptorSetLayout>();
			impl_interface_for_type<DescriptorSetLayout, IDescriptorSetLayout, IDeviceChild>();
			register_boxed_type<DescriptorSet>();
			impl_interface_for_type<DescriptorSet, IDescriptorSet, IDeviceChild>();

			if (FAILED(::CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgi))))
			{
				return BasicError::bad_platform_call();
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
		}

		LUNA_RHI_API R<GraphicAdapterDesc> get_adapter_desc(u32 index)
		{
			ComPtr<IDXGIAdapter1> ada;
			if (FAILED(g_dxgi->EnumAdapters1(index, ada.GetAddressOf())))
			{
				return BasicError::not_found();
			}
			DXGI_ADAPTER_DESC1 desc;
			ada->GetDesc1(&desc);
			GraphicAdapterDesc dst;
			utf16_to_utf8(dst.name, 256, (char16_t*)desc.Description);
			dst.local_memory = desc.DedicatedSystemMemory + desc.DedicatedVideoMemory;
			dst.shared_memory = desc.SharedSystemMemory;
			dst.flags = GraphicAdapterFlag::none;
			if (!desc.DedicatedVideoMemory)
			{
				dst.flags |= GraphicAdapterFlag::uma;
			}
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				dst.flags |= GraphicAdapterFlag::software;
			}
			return dst;
		}

		LUNA_RHI_API R<Ref<IDevice>> new_device(u32 adapter_index)
		{
			ComPtr<ID3D12Device> dev;
			ComPtr<IDXGIAdapter> ada;
			if (FAILED(g_dxgi->EnumAdapters(adapter_index, ada.GetAddressOf())))
			{
				return BasicError::not_found();
			}
			if (FAILED(::D3D12CreateDevice(ada.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))))
			{
				// try warp device.
				ComPtr<IDXGIAdapter> warp;
				ComPtr<IDXGIFactory4> fac;
				g_dxgi.As(&fac);
				fac->EnumWarpAdapter(IID_PPV_ARGS(&warp));

				if (FAILED(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))))
				{
					return set_error(BasicError::bad_platform_call(), "IGraphicSystem::new_device - Failed to create D3D12 Device.");
				}
			}
			Ref<Device> device = new_object<Device>();
			auto res = device->init(dev.Get());
			if (failed(res)) return res.errcode();
			return device;
		}

		LUNA_RHI_API IDevice* get_main_device()
		{
			return g_device;
		}

		LUNA_RHI_API R<Ref<ISwapChain>> new_swap_chain(ICommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc)
		{
			Ref<SwapChain> r = new_object<SwapChain>();
			lutry
			{
				luexp(r->init(window, static_cast<CommandQueue*>(queue->get_object()), desc));
			}
			lucatchret;
			return r;
		}

		LUNA_RHI_API APIType get_current_platform_api_type()
		{
			return APIType::d3d12;
		}
	}
}

#endif