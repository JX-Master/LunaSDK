/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.cpp
* @author JXMaster
* @date 2019/9/20
*/
#include "SwapChain.hpp"
#include "Resource.hpp"
#include "../../RHI.hpp"
#include <Luna/Window/Windows/Win32Window.hpp>
#include <dxgi1_5.h>

namespace Luna
{
	namespace RHI
	{ 
		extern ComPtr<IDXGIFactory5> g_dxgi;
		RV SwapChainResource::init(Device* device, ID3D12Resource* resource)
		{
			lutry
			{
				luexp(encode_hresult(device->m_device->CreateFence(m_wait_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))));
				m_event = CreateEventA(NULL, TRUE, TRUE, NULL);
				if (m_event == NULL)
				{
					return BasicError::bad_platform_call();
				}
				m_back_buffer = new_object<TextureResource>();
				m_back_buffer->m_device = device;
				m_back_buffer->m_res = resource;
				D3D12_RESOURCE_DESC desc = resource->GetDesc();
				TextureUsageFlag usages = TextureUsageFlag::none;
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				{
					usages |= TextureUsageFlag::color_attachment;
				}
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				{
					usages |= TextureUsageFlag::depth_stencil_attachment;
				}
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
				{
					usages |= TextureUsageFlag::read_write_texture;
				}
				m_back_buffer->m_desc = TextureDesc::tex2d(
					decode_format(desc.Format),
					usages,
					desc.Width, desc.Height,
					desc.DepthOrArraySize, desc.MipLevels, desc.SampleDesc.Count
				);
				m_back_buffer->post_init();
			}
			lucatchret;
			return ok;
		}
		RV SwapChain::init(u32 queue_index, Window::IWindow* window, const SwapChainDesc& desc)
		{
			m_window = window;
			m_queue = queue_index;
			ComPtr<IDXGIFactory2> dxgifac;
			g_dxgi.As(&dxgifac);
			m_desc = desc;
			if (!m_desc.width || !m_desc.height)
			{
				UInt2U sz = window->get_size();
				if (!m_desc.width)
				{
					m_desc.width = sz.x;
				}
				if (!m_desc.height)
				{
					m_desc.height = sz.y;
				}
			}
			DXGI_SWAP_CHAIN_DESC1 d;
			d.Width = m_desc.width;
			d.Height = m_desc.height;
			d.Format = encode_format(m_desc.format);
			d.Stereo = FALSE;
			d.SampleDesc.Count = 1;
			d.SampleDesc.Quality = 0;
			d.BufferCount = m_desc.buffer_count;
			d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
			d.Scaling = DXGI_SCALING_NONE;
			d.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			d.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			d.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			HWND hwnd = query_interface<Window::IWin32Window>(window->get_object())->get_hwnd();
			lutry
			{
				luexp(encode_hresult(g_dxgi->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_allow_tearing, sizeof(m_allow_tearing))));
				if (m_allow_tearing)
				{
					d.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				}
				luexp(encode_hresult(dxgifac->CreateSwapChainForHwnd(m_device->m_command_queues[queue_index]->m_command_queue.Get(), hwnd, &d, NULL, NULL, &m_sc)));
				luexp(reset_back_buffer_resources());
			}
			lucatchret;
			return ok;
		}
		SwapChain::~SwapChain()
		{
			for (auto& back_buffer : m_back_buffers)
			{
				WaitForSingleObject(back_buffer.m_event, INFINITE);
				back_buffer.m_back_buffer->m_res.Reset();
			}
		}
		RV SwapChain::reset_back_buffer_resources()
		{
			// Fetch resources.
			lutry
			{
				m_current_back_buffer = 0;
				m_present_flags = 0;
				if (!m_desc.vertical_synchronized && m_allow_tearing)
				{
					BOOL state;
					m_sc->GetFullscreenState(&state, NULL);
					if (!state)
					{
						m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
					}
				}
				for (u32 i = 0; i < m_desc.buffer_count; ++i)
				{
					SwapChainResource res;
					ComPtr<ID3D12Resource> resource;
					luexp(encode_hresult(m_sc->GetBuffer(i, IID_PPV_ARGS(&resource))));
					luexp(res.init(m_device, resource.Get()));
					m_back_buffers.push_back(move(res));
				}
			}
			lucatchret;
			return ok;
		}
		R<ITexture*> SwapChain::get_current_back_buffer()
		{
			lutsassert();
			lutry
			{
				WaitForSingleObject(m_back_buffers[m_current_back_buffer].m_event, INFINITE);
				return (ITexture*)m_back_buffers[m_current_back_buffer].m_back_buffer.get();
			}
		}
		RV SwapChain::present()
		{
			lutsassert();
			lutry
			{
				auto queue = m_device->m_command_queues[m_queue]->m_command_queue.Get();
				luexp(encode_hresult(m_sc->Present(m_desc.vertical_synchronized ? 1 : 0, m_present_flags)));
				auto& back_buffer = m_back_buffers[m_current_back_buffer];
				++back_buffer.m_wait_value;
				::ResetEvent(back_buffer.m_event);
				luexp(encode_hresult(back_buffer.m_fence->SetEventOnCompletion(back_buffer.m_wait_value, back_buffer.m_event)));
				luexp(encode_hresult(queue->Signal(back_buffer.m_fence.Get(), back_buffer.m_wait_value)));
				m_current_back_buffer = (m_current_back_buffer + 1) % (m_desc.buffer_count);
			}
			lucatchret;
			return ok;
		}
		RV SwapChain::reset(const SwapChainDesc& desc)
		{
			lutsassert();
			SwapChainDesc modified_desc = desc;
			if (!modified_desc.buffer_count)
			{
				modified_desc.buffer_count = m_desc.buffer_count;
			}
			if (modified_desc.format == Format::unknown)
			{
				modified_desc.format = m_desc.format;
			}
			for (auto& back_buffer : m_back_buffers)
			{
				WaitForSingleObject(back_buffer.m_event, INFINITE);
				back_buffer.m_back_buffer->m_res.Reset();
			}
			m_back_buffers.clear();
			if (!modified_desc.width || !modified_desc.height)
			{
				auto sz = m_window->get_size();
				if (!modified_desc.width)
				{
					modified_desc.width = sz.x;
				}
				if (!modified_desc.height)
				{
					modified_desc.height = sz.y;
				}
			}
			if (modified_desc.format == Format::unknown)
			{
				modified_desc.format = m_desc.format;
			}
			lutry
			{
				DXGI_SWAP_CHAIN_FLAG flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
				if (m_allow_tearing)
				{
					flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				}
				luexp(encode_hresult(m_sc->ResizeBuffers(modified_desc.buffer_count, modified_desc.width, modified_desc.height, encode_format(modified_desc.format), flags)));
				m_desc = modified_desc;
				luexp(reset_back_buffer_resources());
			}
			lucatchret;
			return ok;
		}
	}
}