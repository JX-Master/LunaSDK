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
#include "../../SwapChain.hpp"
#include <Runtime/TSAssert.hpp>
#include "Resource.hpp"

#pragma comment( lib, "dxguid.lib")

namespace Luna
{
	namespace RHI
	{
		class Device;
		class BackBufferResource;

		struct SwapChainResource
		{
			Ref<TextureResource> m_back_buffer;
			ComPtr<ID3D12Fence> m_fence;
			u64 m_wait_value = 0;
			HANDLE m_event = NULL;

			RV init(Device* device, ID3D12Resource* resource);
			SwapChainResource() = default;
			SwapChainResource(const SwapChainResource&) = delete;
			SwapChainResource(SwapChainResource&& rhs) :
				m_back_buffer(move(rhs.m_back_buffer)),
				m_fence(move(rhs.m_fence)),
				m_wait_value(rhs.m_wait_value),
				m_event(rhs.m_event)
			{
				rhs.m_event = NULL;
			}
			~SwapChainResource()
			{
				if (m_event != NULL)
				{
					CloseHandle(m_event);
					m_event = NULL;
				}
			}
		};

		struct SwapChain : ISwapChain
		{
			lustruct("RHI::SwapChain", "{067d14fa-59c7-4f66-8fb0-1981d90a5a45}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			u32 m_queue;
			Ref<Window::IWindow> m_window;
			ComPtr<IDXGISwapChain1> m_sc;
			SwapChainDesc m_desc;

			Vector<SwapChainResource> m_back_buffers;
			u32 m_current_back_buffer;

			SwapChain() :
				m_current_back_buffer(0) {}

			RV init(u32 queue_index, Window::IWindow* window, const SwapChainDesc& desc);

			//! Called when the back buffer is resized or when the swap chain is initialized.
			RV reset_back_buffer_resources(const SwapChainDesc& desc);

			IDevice* get_device()
			{
				return m_device;
			}
			void set_name(const Name& name) 
			{
				usize len = utf8_to_utf16_len(name.c_str(), name.size());
				wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
				utf8_to_utf16((c16*)buf, len + 1, name.c_str(), name.size());
				m_sc->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(wchar_t) * (len + 1), buf);
			}
			virtual Window::IWindow* get_window() override
			{
				return m_window;
			}
			virtual SwapChainDesc get_desc() override
			{
				return m_desc;
			}
			virtual R<Ref<ITexture>> get_current_back_buffer() override;
			virtual RV present() override;
			virtual RV reset(const SwapChainDesc& desc) override;
		};
	}
}