/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DepthStencilView.hpp
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
		struct DepthStencilView : IDepthStencilView
		{
			lustruct("RHI::DepthStencilView", "{832DFAB4-A00B-446E-8863-90F68BF161C1}");
			luiimpl();
			Ref<Device> m_device;
			Ref<IResource> m_resource;
			ComPtr<ID3D12DescriptorHeap> m_heap;
			DepthStencilViewDesc m_desc;

			RV init(IResource* resource, const DepthStencilViewDesc* desc);

			~DepthStencilView()
			{
				m_device->m_dsv_heap.free_view(m_heap.Get());
			}
			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) {}
			IResource* get_resource()
			{
				return m_resource;
			}
			DepthStencilViewDesc get_desc()
			{
				return m_desc;
			}
		};
	}
}

#endif