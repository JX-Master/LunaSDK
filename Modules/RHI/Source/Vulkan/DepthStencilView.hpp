/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DepthStencilView.hpp
* @author JXMaster
* @date 2023/4/23
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DepthStencilView : IDepthStencilView
		{
			lustruct("RHI::DepthStencilView", "{7ABC70D6-199E-44BE-9D59-197EF94BDCFD}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			Ref<IResource> m_resource;
			DepthStencilViewDesc m_desc;

			VkImageView m_view = VK_NULL_HANDLE;
			RV init(IResource* resource, const DepthStencilViewDesc* desc);
			~DepthStencilView();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual IResource* get_resource() override { return m_resource; }
			virtual DepthStencilViewDesc get_desc() override { return m_desc; }
		};
	}
}