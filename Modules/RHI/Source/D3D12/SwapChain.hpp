/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.hpp
* @author JXMaster
* @date 2019/9/20
*/
#pragma once
#include "D3D12Common.hpp"
#include <dxgi1_2.h>
#include "Device.hpp"
#include "CommandQueue.hpp"
#include "../../SwapChain.hpp"
#include <Runtime/TSAssert.hpp>

#pragma comment( lib, "dxguid.lib")

namespace Luna
{
	namespace RHI
	{
		class Device;
		class BackBufferResource;
		struct SwapChain : ISwapChain
		{
			lustruct("RHI::SwapChain", "{067d14fa-59c7-4f66-8fb0-1981d90a5a45}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			Ref<CommandQueue> m_queue;
			Ref<Window::IWindow> m_window;
			ComPtr<IDXGISwapChain1> m_sc;
			SwapChainDesc m_desc;

			ComPtr<ID3D12Fence> m_fence;
			HANDLE m_event;
			ComPtr<ID3D12CommandAllocator> m_ca;
			ComPtr<ID3D12GraphicsCommandList> m_li;
			ComPtr<ID3D12RootSignature> m_root_signature;
			ComPtr<ID3D12DescriptorHeap> m_rtvs;
			ComPtr<ID3D12DescriptorHeap> m_srv;
			usize m_rtv_size;

			// Present resources, need to be reset when the swap chain is resized.
			ComPtr<ID3D12PipelineState> m_pso;
			Vector<ComPtr<ID3D12Resource>> m_back_buffers;
			u32 m_current_back_buffer;

			SwapChain() :
				m_event(NULL),
				m_current_back_buffer(0) {}

			~SwapChain()
			{
				if (m_event)
				{
					::CloseHandle(m_event);
					m_event = NULL;
				}
			}

			//! Initializes all resources stored in device, which is shared between all swap chains for the same device.
			RV init_shared_res();

			RV init(Window::IWindow* window, CommandQueue* queue, const SwapChainDesc& desc);

			//! Called when the back buffer is resized or when the swap chain is initialized.
			RV reset_back_buffer_resources(const SwapChainDesc& desc);

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) 
			{
				usize len = utf8_to_utf16_len(name.c_str(), name.size());
				wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
				utf8_to_utf16((c16*)buf, len + 1, name.c_str(), name.size());
				m_sc->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(wchar_t) * (len + 1), buf);
			}

			void wait()
			{
				DWORD res = ::WaitForSingleObject(m_event, INFINITE);
				if (res != WAIT_OBJECT_0)
				{
					lupanic_msg_always("WaitForSingleObject failed.");
				}
			}
			bool try_wait()
			{
				DWORD res = ::WaitForSingleObject(m_event, 0);
				if (res == WAIT_OBJECT_0)
				{
					return true;
				}
				return false;
			}

			Window::IWindow* get_bounding_window()
			{
				return m_window;
			}

			SwapChainDesc get_desc()
			{
				return m_desc;
			}
			RV present(IResource* resource, u32 subresource);
			RV reset(const SwapChainDesc& desc);
		};
	}
}