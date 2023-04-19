// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file SwapChain.cpp
* @author JXMaster
* @date 2022/10/29
*/
#include "SwapChain.hpp"
#include <GLFW/glfw3.h>
#include "VulkanRHI.hpp"
#include <Window/GLFW/GLFWWindow.hpp>
#include "Instance.hpp"
namespace Luna
{
	namespace RHI
	{
		R<VkSurfaceFormatKHR> choose_swap_surface_format(const Vector<VkSurfaceFormatKHR>& available_formats, Format desired_format)
		{
			VkFormat desired_vk_format = encode_pixel_format(desired_format);
			for (const auto& format : available_formats)
			{
				if (format.format == desired_vk_format)
				{
					return format;
				}
			}
			return set_error(BasicError::not_supported(), "The specified pixel format for swap chain is not supported.");
		}

		VkPresentModeKHR choose_present_mode(const Vector<VkPresentModeKHR>& available_presnet_modes)
		{
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		R<VkExtent2D> choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const SwapChainDesc& desc)
		{
			if (desc.width < capabilities.minImageExtent.width || desc.height < capabilities.minImageExtent.height ||
				desc.width > capabilities.maxImageExtent.width || desc.height > capabilities.maxImageExtent.height)
			{
				return set_error(BasicError::not_supported(), 
					"The swap chain size specified is not supported by the current window. Speciifed size is: (%u, %u), supportted range is: (%u-%u, %u-%u)", 
					desc.width, desc.height, capabilities.minImageExtent.width, capabilities.maxImageExtent.width, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			}
			VkExtent2D ret;
			ret.width = desc.width;
			ret.height = desc.height;
			return ret;
		}

		RV SwapChain::init(ICommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				auto framebuffer_size = window->get_framebuffer_size();
				m_desc.width = desc.width == 0 ? framebuffer_size.x : desc.width;
				m_desc.height = desc.height == 0 ? framebuffer_size.y : desc.height;
				if (!test_flags(queue->get_desc().flags, CommandQueueFlags::presenting))
				{
					return set_error(BasicError::not_supported(), "The specified command queue for creating swap chain does not have presenting support");
				}
				m_presenting_queue = queue;
				m_window = window;
				Window::IGLFWWindow* w = query_interface<Window::IGLFWWindow>(window->get_object());
				if (!w) return BasicError::not_supported();
				luexp(encode_vk_result(glfwCreateWindowSurface(g_vk_instance, w->get_glfw_window_handle(), nullptr, &m_surface)));
				auto& surface_info = get_physical_device_surface_info(m_device->m_physical_device, m_surface);
				lulet(surface_format, choose_swap_surface_format(surface_info.formats, desc.pixel_format));
				auto present_mode = choose_present_mode(surface_info.present_modes);
				lulet(extent, choose_swap_extent(surface_info.capabilities, m_desc));
				if (desc.buffer_count < surface_info.capabilities.minImageCount || desc.buffer_count > surface_info.capabilities.maxImageCount)
				{
					return set_error(BasicError::not_supported(),
						"The specified buffer count is not supported by the current window. Specified buffer count is %u, supported range is %u-%u",
						desc.buffer_count, surface_info.capabilities.minImageCount, surface_info.capabilities.maxImageCount);
				}
				VkSwapchainCreateInfoKHR create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				create_info.surface = m_surface;
				create_info.minImageCount = desc.buffer_count;
				create_info.imageFormat = surface_format.format;
				create_info.imageColorSpace = surface_format.colorSpace;
				create_info.imageExtent = extent;
				create_info.imageArrayLayers = 1;
				create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0;
				create_info.pQueueFamilyIndices = nullptr;
				create_info.preTransform = surface_info.capabilities.currentTransform;
				create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				create_info.presentMode = present_mode;
				create_info.clipped = VK_TRUE;
				create_info.oldSwapchain = VK_NULL_HANDLE;
				luexp(encode_vk_result(vkCreateSwapchainKHR(m_device->m_device, &create_info, nullptr, &m_swap_chain)));
				u32 image_count;
				luexp(encode_vk_result(vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, nullptr)));
				m_swap_chain_images.resize(image_count);
				luexp(encode_vk_result(vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, m_swap_chain_images.data())));
				m_swap_chain_image_views.resize(image_count);
				for (usize i = 0; i < m_swap_chain_image_views.size(); ++i)
				{
					VkImageViewCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					createInfo.image = m_swap_chain_images[i];
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = surface_format.format;
					createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.levelCount = 1;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.layerCount = 1;
					luexp(encode_vk_result(vkCreateImageView(m_device->m_device, &createInfo, nullptr, &m_swap_chain_image_views[i])));
				}
			}
			lucatchret;
			return ok;
		}
		SwapChain::~SwapChain()
		{
			for (VkImageView i : m_swap_chain_image_views)
			{
				vkDestroyImageView(m_device->m_device, i, nullptr);
			}
			if (m_swap_chain != VK_NULL_HANDLE)
			{
				vkDestroySwapchainKHR(m_device->m_device, m_swap_chain, nullptr);
				m_swap_chain = nullptr;
			}
			if (m_surface != VK_NULL_HANDLE)
			{
				vkDestroySurfaceKHR(g_vk_instance, m_surface, nullptr);
				m_surface = VK_NULL_HANDLE;
			}
		}
	}
}