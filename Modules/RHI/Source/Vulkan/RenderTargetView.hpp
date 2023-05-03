/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RenderTargetView.hpp
* @author JXMaster
* @date 2023/4/23
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct RenderTargetView : IRenderTargetView
		{
			lustruct("RHI::RenderTargetView", "{F3ABC6B3-D8D5-4636-A14A-166EE7406687}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			Ref<ITexture> m_resource;
			RenderTargetViewDesc m_desc;

			VkImageView m_view = VK_NULL_HANDLE;
			RV init(ITexture* resource, const RenderTargetViewDesc* desc);
			~RenderTargetView();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual ITexture* get_resource() override { return m_resource; }
			virtual RenderTargetViewDesc get_desc() override { return m_desc; }
		};
	}
}