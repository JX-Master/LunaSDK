// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file VulkanRHI.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include "VulkanRHI.hpp"
#include <Runtime/HashSet.hpp>
#include "../RHI.hpp"
#include "Device.hpp"
#include <Window/Window.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include "Instance.hpp"
#include <GLFW/glfw3.h>

namespace Luna
{
	namespace RHI
	{
		Vector<VkPhysicalDevice> g_physical_devices;
		u32 g_main_physical_device_index;

		Vector<QueueFamily> g_device_queue_families;

		const c8* g_device_extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		usize g_num_device_extensions = 1;

		Ref<IDevice> g_main_device;

		PhysicalDeviceSurfaceInfo get_physical_device_surface_info(VkPhysicalDevice device, VkSurfaceKHR surface) 
		{
			PhysicalDeviceSurfaceInfo details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
			{
				uint32_t format_count;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
				if (format_count != 0)
				{
					details.formats.resize(format_count);
					vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
				}
			}
			{
				uint32_t present_mode_count;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
				if (present_mode_count != 0)
				{
					details.present_modes.resize(present_mode_count);
					vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
				}
			}
			return details;
		}

		static R<Vector<QueueFamily>> get_device_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
		{
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
			VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)alloca(sizeof(VkQueueFamilyProperties) * queue_family_count);
			Vector<QueueFamily> ret;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
			for (u32 i = 0; i < queue_family_count; ++i)
			{
				// Detect queue type.
				auto& src = queue_families[i];
				QueueFamily dst;
				bool valid = false;
				dst.num_queues = queue_families[i].queueCount;
				dst.index = i;
				// GRAPHICS and COMPUTE an always implicitly accept TRANSFER workload, so we don't need to explicitly check it.
				// See Vulkan Specification for VkQueueFlagBits.
				if (src.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
				{
					// For any device that supports VK_QUEUE_GRAPHICS_BIT, there must be one family that support 
					// VK_QUEUE_GRAPHICS_BIT and VK_QUEUE_COMPUTE_BIT.
					// See Vulkan Specification for VkQueueFlagBits.
					valid = true;
					dst.type = CommandQueueType::graphics;
				}
				else if (src.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					valid = true;
					dst.type = CommandQueueType::compute;
				}
				else if (src.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					valid = true;
					dst.type = CommandQueueType::copy;
				}
				VkBool32 present_support = false;
				auto r = encode_vk_result(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support));
				if (failed(r)) return r.errcode();
				dst.present_support = present_support ? true : false;
				if (valid)
				{
					ret.push_back(dst);
				}
			}
			return ret;
		}

		inline bool check_device_extension_support(VkPhysicalDevice device)
		{
			u32 extension_count;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
			VkExtensionProperties* available_extensions = (VkExtensionProperties*)alloca(sizeof(VkExtensionProperties) * extension_count);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);
			HashSet<Name> required_extensions;
			for (usize i = 0; i < g_num_device_extensions; ++i)
			{
				required_extensions.insert(g_device_extensions[i]);
			}		
			for (u32 i = 0; i < extension_count; ++i)
			{
				auto& extension = available_extensions[i];
				required_extensions.erase(extension.extensionName);
			}
			return required_extensions.empty();
		}

		inline bool is_device_suitable(VkPhysicalDevice device, const Vector<QueueFamily>& families, VkSurfaceKHR surface)
		{
			bool graphic_queue_present = false;
			for (auto& i : families)
			{
				if (i.type == CommandQueueType::graphics && i.present_support)
				{
					graphic_queue_present = true;
					break;
				}
			}
			bool extensions_supported = check_device_extension_support(device);
			auto swap_chain_support_info = get_physical_device_surface_info(device, surface);
			bool swap_chain_supported = !swap_chain_support_info.formats.empty() && !swap_chain_support_info.present_modes.empty();
			return graphic_queue_present && extensions_supported && swap_chain_supported;
		}

		static RV select_physical_device(VkSurfaceKHR dummy_surface)
		{
			lutry
			{
				u32 device_count = 0;
				vkEnumeratePhysicalDevices(g_vk_instance, &device_count, nullptr);
				if (device_count == 0)
				{
					return set_error(BasicError::not_supported(), "Failed to find GPUs with Vulkan support!");
				}
				g_physical_devices.resize(device_count);
				vkEnumeratePhysicalDevices(g_vk_instance, &device_count, g_physical_devices.data());
				bool physical_device_found = false;
				// Prepare device data.
				Vector<VkPhysicalDeviceProperties> device_properties;
				Vector<Vector<QueueFamily>> device_queue_families;
				for (u32 i = 0; i < device_count; ++i)
				{
					VkPhysicalDeviceProperties properties;
					vkGetPhysicalDeviceProperties(g_physical_devices[i], &properties);
					lulet(families, get_device_queue_families(g_physical_devices[i], dummy_surface));
					device_properties.push_back(move(properties));
					device_queue_families.push_back(move(families));
				}
				// Select dedicated device if present.
				for (u32 i = 0; i < device_count; ++i)
				{
					if (is_device_suitable(g_physical_devices[i], device_queue_families[i], dummy_surface) && device_properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						g_device_queue_families = move(device_queue_families[i]);
						g_main_physical_device_index = i;
						physical_device_found = true;
						break;
					}
				}
				// Fallback to intergrated GPU if present.
				if (!physical_device_found)
				{
					for (u32 i = 0; i < device_count; ++i)
					{
						if (is_device_suitable(g_physical_devices[i], device_queue_families[i], dummy_surface) && device_properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
						{
							g_device_queue_families = move(device_queue_families[i]);
							g_main_physical_device_index = i;
							physical_device_found = true;
							break;
						}
					}
				}
				// Fallback to Any GPU.
				if (!physical_device_found)
				{
					for (u32 i = 0; i < device_count; ++i)
					{
						if (is_device_suitable(g_physical_devices[i], device_queue_families[i], dummy_surface))
						{
							g_device_queue_families = move(device_queue_families[i]);
							g_main_physical_device_index = i;
							physical_device_found = true;
							break;
						}
					}
				}
				if (!physical_device_found)
				{
					return set_error(BasicError::not_supported(), "Failed to find a suitable GPU for Vulkan!");
				}
			}
			lucatchret;
			return ok;
		}

		RV render_api_init()
		{
			VkSurfaceKHR dummy_surface = VK_NULL_HANDLE;
			Ref<Window::IWindow> dummy_window;
			lutry
			{
				luexp(create_vk_instance());
				// Create surface for device checking.
				{
					lulet(dummy_window, Window::new_window("Dummy Window", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::hidden));
					// Fetch surface for dummy window.
					Window::IGLFWWindow* window = query_interface<Window::IGLFWWindow>(dummy_window->get_object());
					if (!window)
					{
						return BasicError::not_supported();
					}
					GLFWwindow* glfw_window = window->get_glfw_window_handle();
					luexp(encode_vk_result(glfwCreateWindowSurface(g_vk_instance, window->get_glfw_window_handle(), nullptr, &dummy_surface)));
				}
				// Select physical device.
				luexp(select_physical_device(dummy_surface));
				// Create logical device.
				Ref<Device> device = new_object<Device>();
				luexp(device->init(g_physical_devices[g_main_physical_device_index], g_device_queue_families));
				g_main_device = device;
				if (dummy_surface != VK_NULL_HANDLE)
				{
					vkDestroySurfaceKHR(g_vk_instance, dummy_surface, nullptr);
				}
			}
			lucatch
			{
				if (dummy_surface != VK_NULL_HANDLE)
				{
					vkDestroySurfaceKHR(g_vk_instance, dummy_surface, nullptr);
				}
				return lures;
			}
			return ok;
		}

		void render_api_close()
		{
			g_main_device.reset();
			g_physical_devices.clear();
			g_physical_devices.shrink_to_fit();
			g_device_queue_families.clear();
			g_device_queue_families.shrink_to_fit();

			
			destroy_vk_instance();
		}
	}
}