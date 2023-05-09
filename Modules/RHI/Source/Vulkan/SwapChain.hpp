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
#include "CommandQueue.hpp"
namespace Luna
{
	namespace RHI
	{
		struct SwapChain : ISwapChain
		{
			lustruct("RHI::SwapChain", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");
			luiimpl();
			Ref<Device> m_device;

			SwapChainDesc m_desc;
			Ref<CommandQueue> m_presenting_queue;
			Ref<Window::IWindow> m_window;
			VkSurfaceKHR m_surface = VK_NULL_HANDLE;

			VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
			Vector<Ref<ImageResource>> m_swap_chain_images;

			VkFence m_acqure_fence;

			u32 m_current_back_buffer;
			bool m_back_buffer_fetched = false;

			RV init(CommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc);
			
			void clean_up_swap_chain();
			RV create_swap_chain(const SwapChainDesc& desc);
			
			~SwapChain();

			virtual Window::IWindow* get_window() override { return m_window; }
			virtual SwapChainDesc get_desc() override { return m_desc; }
			virtual R<Ref<ITexture>> get_current_back_buffer() override;
			virtual RV present() override;
			virtual RV reset(const SwapChainDesc& desc) override;
		};
	}
}