/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SwapChain.cpp
* @author JXMaster
* @date 2022/10/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#include "SwapChain.hpp"
#include <GLFW/glfw3.h>
#include <Luna/Window/GLFW/GLFWWindow.hpp>
#include "Instance.hpp"
namespace Luna
{
	namespace RHI
	{
		R<VkSurfaceFormatKHR> choose_swap_surface_format(const Vector<VkSurfaceFormatKHR>& available_formats, Format desired_format)
		{
			if (desired_format == Format::unknown)
			{
				return available_formats.front();
			}
			VkFormat desired_vk_format = encode_format(desired_format);
			for (const auto& format : available_formats)
			{
				if (format.format == desired_vk_format)
				{
					return format;
				}
			}
			return set_error(BasicError::not_supported(), "The specified pixel format for swap chain is not supported.");
		}

		VkPresentModeKHR choose_present_mode(const Vector<VkPresentModeKHR>& available_presnet_modes, bool vertical_synchronized)
		{
			if (!vertical_synchronized)
			{
				for (auto i : available_presnet_modes)
				{
					if (i == VK_PRESENT_MODE_IMMEDIATE_KHR) return VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
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

		RV SwapChain::init(const CommandQueue& queue, Window::IWindow* window, const SwapChainDesc& desc)
		{
			lutry
			{
				m_queue = queue;
				m_window = window;
				Window::IGLFWWindow* w = query_interface<Window::IGLFWWindow>(m_window->get_object());
				if (!w) return BasicError::not_supported();
				luexp(encode_vk_result(glfwCreateWindowSurface(g_vk_instance, w->get_glfw_window_handle(), nullptr, &m_surface)));
				luexp(create_swap_chain(desc));
				VkFenceCreateInfo fence_create_info{};
				fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fence_create_info.flags = 0;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateFence(m_device->m_device, &fence_create_info, nullptr, &m_acqure_fence)));
			}
			lucatchret;
			return ok;
		}
		void SwapChain::clean_up_swap_chain()
		{
			MutexGuard guard(m_queue.queue_mtx);
			m_device->m_funcs.vkQueueWaitIdle(m_queue.queue);
			m_swap_chain_images.clear();
			if (m_swap_chain != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroySwapchainKHR(m_device->m_device, m_swap_chain, nullptr);
				m_swap_chain = VK_NULL_HANDLE;
			}
		}
		RV SwapChain::create_swap_chain(const SwapChainDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				auto framebuffer_size = m_window->get_framebuffer_size();
				m_desc.width = desc.width == 0 ? framebuffer_size.x : desc.width;
				m_desc.height = desc.height == 0 ? framebuffer_size.y : desc.height;
				if (!test_flags(m_queue.desc.flags, CommandQueueFlag::presenting))
				{
					return set_error(BasicError::not_supported(), "The specified command queue for creating swap chain does not have presenting support");
				}
				auto surface_info = get_physical_device_surface_info(m_device->m_physical_device, m_surface);
				lulet(surface_format, choose_swap_surface_format(surface_info.formats, desc.format));
				auto present_mode = choose_present_mode(surface_info.present_modes, desc.vertical_synchronized);
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
				create_info.imageUsage = surface_info.capabilities.supportedUsageFlags;
				create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0;
				create_info.pQueueFamilyIndices = nullptr;
				create_info.preTransform = surface_info.capabilities.currentTransform;
				create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				create_info.presentMode = present_mode;
				create_info.clipped = VK_TRUE;
				create_info.oldSwapchain = VK_NULL_HANDLE;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateSwapchainKHR(m_device->m_device, &create_info, nullptr, &m_swap_chain)));
				u32 image_count;
				luexp(encode_vk_result(m_device->m_funcs.vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, nullptr)));
				VkImage* images = (VkImage*)alloca(sizeof(VkImage) * image_count);
				luexp(encode_vk_result(m_device->m_funcs.vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, images)));
				TextureDesc desc;
				desc.type = TextureType::tex2d;
				desc.format = m_desc.format;
				desc.width = m_desc.width;
				desc.height = m_desc.height;
				desc.depth = 1;
				desc.array_size = 1;
				desc.mip_levels = 1;
				desc.sample_count = 1;
				for (u32 i = 0; i < image_count; ++i)
				{
					auto res = new_object<ImageResource>();
					res->m_device = m_device;
					res->m_desc = desc;
					res->m_image = images[i];
					res->m_global_states.emplace_back();
					res->m_is_image_externally_managed = true;
					m_swap_chain_images.push_back(res);
				}
				m_back_buffer_fetched = false;
			}
			lucatchret;
			return ok;
		}
		SwapChain::~SwapChain()
		{
			clean_up_swap_chain();
			if (m_acqure_fence != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyFence(m_device->m_device, m_acqure_fence, nullptr);
				m_acqure_fence = VK_NULL_HANDLE;
			}
			if (m_surface != VK_NULL_HANDLE)
			{
				vkDestroySurfaceKHR(g_vk_instance, m_surface, nullptr);
				m_surface = VK_NULL_HANDLE;
			}
		}
		R<ITexture*> SwapChain::get_current_back_buffer()
		{
			lutry
			{
				if (!m_back_buffer_fetched)
				{
					luexp(encode_vk_result(m_device->m_funcs.vkAcquireNextImageKHR(
						m_device->m_device,
						m_swap_chain,
						UINT64_MAX,
						VK_NULL_HANDLE,
						m_acqure_fence,
						&m_current_back_buffer
					)));
					luexp(encode_vk_result(m_device->m_funcs.vkWaitForFences(m_device->m_device, 1,
						&m_acqure_fence, VK_TRUE, UINT64_MAX)));
					luexp(encode_vk_result(m_device->m_funcs.vkResetFences(m_device->m_device, 1, &m_acqure_fence)));
					m_back_buffer_fetched = true;
				}
			}
			lucatchret;
			return (ITexture*)(m_swap_chain_images[m_current_back_buffer].get());
		}
		RV SwapChain::present()
		{
			lutry
			{
				if (!m_back_buffer_fetched)
				{
					// To fetch m_current_back_buffer
					lulet(back_buffer, get_current_back_buffer());
				}
				VkPresentInfoKHR present_info{};
				present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				present_info.waitSemaphoreCount = 0;
				present_info.swapchainCount = 1;
				present_info.pSwapchains = &m_swap_chain;
				present_info.pImageIndices = &m_current_back_buffer;
				MutexGuard guard(m_queue.queue_mtx);
				luexp(encode_vk_result(m_device->m_funcs.vkQueuePresentKHR(m_queue.queue, &present_info)));
				m_back_buffer_fetched = false;
			}
			lucatchret;
			return ok;
		}
		RV SwapChain::reset(const SwapChainDesc& desc)
		{
			// Wait for all presenting calls to finish.
			lutry
			{
				clean_up_swap_chain();
				auto new_desc = desc;
				if (new_desc.format == Format::unknown)
				{
					new_desc.format = m_desc.format;
				}
				luexp(create_swap_chain(new_desc));
			}
			lucatchret;
			return ok;
		}
	}
}