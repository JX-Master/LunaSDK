/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderTargetView.hpp
* @author JXMaster
* @date 2022/8/5
*/
#pragma once
#ifdef LUNA_RHI_D3D12
#include "D3D12Common.hpp"
#include "Device.hpp"
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		struct RenderTargetView : IRenderTargetView
		{
			lustruct("RHI::RenderTargetView", "{60DBE49B-B565-424A-B533-82E50FD3472C}");
			luiimpl();
			Ref<Device> m_device;
			Ref<IResource> m_resource;
			ComPtr<ID3D12DescriptorHeap> m_heap;
			RenderTargetViewDesc m_desc;

			RV init(IResource* resource, const RenderTargetViewDesc* desc);

			~RenderTargetView()
			{
				m_device->m_rtv_heap.free_view(m_heap.Get());
			}
			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			IResource* get_resource()
			{
				return m_resource;
			}
			RenderTargetViewDesc get_desc()
			{
				return m_desc;
			}
		};
	}
}

#endif