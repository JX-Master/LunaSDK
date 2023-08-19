/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SwapChain.hpp
* @author JXMaster
* @date 2022/10/29
*/
#pragma once
#include "Common.hpp"
#include "Device.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		struct SwapChain : ISwapChain
		{
			lustruct("RHI::SwapChain", "{E62614A8-3AB3-46D1-8DD8-80671C571FBC}");
			luiimpl();
			Ref<Device> m_device;
			Name m_name;

			CommandQueue m_queue;

			SwapChainDesc m_desc;
			Ref<Window::IWindow> m_window;
			VkSurfaceKHR m_surface = VK_NULL_HANDLE;

			VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
			Vector<Ref<ImageResource>> m_swap_chain_images;

			VkFence m_acqure_fence;

			u32 m_current_back_buffer;
			bool m_back_buffer_fetched = false;

			RV init(const CommandQueue& queue, Window::IWindow* window, const SwapChainDesc& desc);
			
			void clean_up_swap_chain();
			RV create_swap_chain(const SwapChainDesc& desc);
			
			~SwapChain();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
			virtual Window::IWindow* get_window() override { return m_window; }
			virtual SwapChainDesc get_desc() override { return m_desc; }
			virtual R<ITexture*> get_current_back_buffer() override;
			virtual RV present() override;
			virtual RV reset(const SwapChainDesc& desc) override;
		};
	}
}