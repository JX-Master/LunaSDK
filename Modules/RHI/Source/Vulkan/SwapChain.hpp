// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file SwapChain.hpp
* @author JXMaster
* @date 2022/10/29
*/
#pragma once
#include "Common.hpp"
#include "Device.hpp"
namespace Luna
{
	namespace RHI
	{
		struct SwapChain
		{
			lustruct("RHI::SwapChain", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");

			SwapChainDesc m_desc;
			Ref<ICommandQueue> m_presenting_queue;
			Ref<Window::IWindow> m_window;
			Ref<Device> m_device;
			VkSurfaceKHR m_surface = VK_NULL_HANDLE;
			VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
			Vector<VkImage> m_swap_chain_images;
			Vector<VkImageView> m_swap_chain_image_views;

			RV init(ICommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc);
			~SwapChain();
		};
	}
}