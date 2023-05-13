/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.cpp
* @author JXMaster
* @date 2023/4/16
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Adapter.hpp"
#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Runtime/HashSet.hpp>

namespace Luna
{
	namespace RHI
	{
		Vector<VkPhysicalDevice> g_physical_devices;
		Vector<Vector<QueueFamily>> g_physical_device_queue_families;
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

		inline bool check_device_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR check_surface)
		{
			auto swap_chain_support_info = get_physical_device_surface_info(device, check_surface);
			return !swap_chain_support_info.formats.empty() && !swap_chain_support_info.present_modes.empty();
		}

		static R<Vector<QueueFamily>> get_device_queue_families(VkPhysicalDevice device, VkSurfaceKHR check_surface)
		{
			// Check device swap chain support.
			bool device_swap_chain_supported = check_device_swap_chain_support(device, check_surface);
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
			VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)alloca(sizeof(VkQueueFamilyProperties) * queue_family_count);
			Vector<QueueFamily> ret;
			bool graphics_queue_family_present = false;
			bool compute_queue_family_present = false;
			bool copy_queue_family_present = false;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
			for (u32 i = 0; i < queue_family_count; ++i)
			{
				// Detect queue type.
				auto& src = queue_families[i];
				QueueFamily dst;
				bool valid = false;
				dst.num_queues = queue_families[i].queueCount;
				dst.index = i;
				dst.desc.flags = CommandQueueFlags::none;
				// GRAPHICS and COMPUTE an always implicitly accept TRANSFER workload, so we don't need to explicitly check it.
				// See Vulkan Specification for VkQueueFlagBits.
				if (src.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
				{
					// For any device that supports VK_QUEUE_GRAPHICS_BIT, there must be one family that support 
					// VK_QUEUE_GRAPHICS_BIT and VK_QUEUE_COMPUTE_BIT.
					// See Vulkan Specification for VkQueueFlagBits.
					if (!graphics_queue_family_present)
					{
						valid = true;
						dst.desc.type = CommandQueueType::graphics;
						graphics_queue_family_present = true;
					}
				}
				else if (src.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					if (!compute_queue_family_present)
					{
						valid = true;
						dst.desc.type = CommandQueueType::compute;
						compute_queue_family_present = true;
					}
				}
				else if (src.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					if (!copy_queue_family_present)
					{
						valid = true;
						dst.desc.type = CommandQueueType::copy;
						copy_queue_family_present = true;
					}
				}
				if (valid)
				{
					VkBool32 present_support = false;
					auto r = encode_vk_result(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, check_surface, &present_support));
					if (failed(r)) return r.errcode();
					if (present_support && device_swap_chain_supported)
					{
						set_flags(dst.desc.flags, CommandQueueFlags::presenting);
					}
					ret.push_back(dst);
				}
			}
			return ret;
		}

		static RV init_physical_device_queue_families()
		{
			VkSurfaceKHR dummy_surface = VK_NULL_HANDLE;
			Ref<Window::IWindow> dummy_window;
			lutry
			{
				luset(dummy_window, Window::new_window("Dummy Window", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::hidden));
				// Fetch surface for dummy window.
				Window::IGLFWWindow* window = query_interface<Window::IGLFWWindow>(dummy_window->get_object());
				if (!window)
				{
					return BasicError::not_supported();
				}
				GLFWwindow* glfw_window = window->get_glfw_window_handle();
				luexp(encode_vk_result(glfwCreateWindowSurface(g_vk_instance, window->get_glfw_window_handle(), nullptr, &dummy_surface)));
				// Select physical device.
				for (usize i = 0; i < g_physical_devices.size(); ++i)
				{
					lulet(queue_families, get_device_queue_families(g_physical_devices[i], dummy_surface));
					g_physical_device_queue_families.push_back(move(queue_families));
				}
				if (dummy_surface != VK_NULL_HANDLE)
				{
					vkDestroySurfaceKHR(g_vk_instance, dummy_surface, nullptr);
					dummy_surface = VK_NULL_HANDLE;
				}
			}
			lucatch
			{
				if (dummy_surface != VK_NULL_HANDLE)
				{
					vkDestroySurfaceKHR(g_vk_instance, dummy_surface, nullptr);
					dummy_surface = VK_NULL_HANDLE;
				}
				return lures;
			}
			return ok;
		}

		RV init_physical_devices()
		{
			lutry
			{
				{
					u32 device_count = 0;
					vkEnumeratePhysicalDevices(g_vk_instance, &device_count, nullptr);
					if (device_count == 0)
					{
						return set_error(BasicError::not_supported(), "Failed to find GPUs with Vulkan support!");
					}
					g_physical_devices.resize(device_count);
					vkEnumeratePhysicalDevices(g_vk_instance, &device_count, g_physical_devices.data());
				}
				luexp(init_physical_device_queue_families());
			}
			lucatchret;
			return ok;
		}
		void clear_physical_devices()
		{
			g_physical_device_queue_families.clear();
			g_physical_device_queue_families.shrink_to_fit();
			g_physical_devices.clear();
			g_physical_devices.shrink_to_fit();
		}
		inline bool check_device_extension_support(VkPhysicalDevice device)
		{
			u32 extension_count;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
			VkExtensionProperties* available_extensions = (VkExtensionProperties*)alloca(sizeof(VkExtensionProperties) * extension_count);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);
			HashSet<Name> required_extensions;

			for (usize i = 0; i < NUM_VK_DEVICE_ENTENSIONS; ++i)
			{
				required_extensions.insert(VK_DEVICE_ENTENSIONS[i]);
			}
			for (u32 i = 0; i < extension_count; ++i)
			{
				auto& extension = available_extensions[i];
				required_extensions.erase(extension.extensionName);
			}
			return required_extensions.empty();
		}
		inline R<bool> is_device_suitable(VkPhysicalDevice device, const Vector<QueueFamily>& families)
		{
			bool graphic_queue_present = false;
			bool present_queue_present = false;
			for (auto& i : families)
			{
				if (i.desc.type == CommandQueueType::graphics)
				{
					graphic_queue_present = true;
				}
				if (test_flags(i.desc.flags, CommandQueueFlags::presenting))
				{
					present_queue_present = true;
				}
			}
			bool extensions_supported = check_device_extension_support(device);
			return graphic_queue_present && present_queue_present && extensions_supported;
		}
		R<usize> select_main_physical_device()
		{
			lutry
			{
				// Prepare device data.
				Vector<VkPhysicalDeviceProperties> device_properties;
				for (usize i = 0; i < g_physical_devices.size(); ++i)
				{
					VkPhysicalDeviceProperties properties;
					vkGetPhysicalDeviceProperties(g_physical_devices[i], &properties);
					device_properties.push_back(move(properties));
				}
				// Select dedicated device if present.
				for (usize i = 0; i < g_physical_devices.size(); ++i)
				{
					lulet(suitable, is_device_suitable(g_physical_devices[i], g_physical_device_queue_families[i]));
					if (suitable && device_properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						return i;
					}
				}
				// Fallback to intergrated GPU if present.
				for (usize i = 0; i < g_physical_devices.size(); ++i)
				{
					lulet(suitable, is_device_suitable(g_physical_devices[i], g_physical_device_queue_families[i]));
					if (suitable && device_properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
					{
						return i;
					}
				}
				// Fallback to Any GPU.
				for (usize i = 0; i < g_physical_devices.size(); ++i)
				{
					lulet(suitable, is_device_suitable(g_physical_devices[i], g_physical_device_queue_families[i]));
					if (suitable)
					{
						return i;
					}
				}
			}
			lucatchret;
			return set_error(BasicError::not_supported(), "Failed to find a suitable GPU for Vulkan!");
		}
		LUNA_RHI_API u32 get_num_adapters()
		{
			return (u32)g_physical_devices.size();
		}
		LUNA_RHI_API AdapterDesc get_adapter_desc(u32 adapter_index)
		{
			lucheck(adapter_index < g_physical_devices.size());
			AdapterDesc ret;
			VkPhysicalDeviceProperties device_properties;
			VkPhysicalDeviceMemoryProperties memory_properties;
			vkGetPhysicalDeviceProperties(g_physical_devices[adapter_index], &device_properties);
			vkGetPhysicalDeviceMemoryProperties(g_physical_devices[adapter_index], &memory_properties);
			strncpy(ret.name, device_properties.deviceName, 256);
			u64 local_memory = 0;
			u64 shared_memory = 0;
			for (u32 i = 0; i < memory_properties.memoryHeapCount; ++i)
			{
				if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				{
					local_memory += memory_properties.memoryHeaps[i].size;
				}
				else
				{
					shared_memory += memory_properties.memoryHeaps[i].size;
				}
			}
			ret.local_memory = local_memory;
			ret.shared_memory = shared_memory;
			switch (device_properties.deviceType)
			{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				ret.type = AdapterType::integrated_gpu; break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				ret.type = AdapterType::discrete_gpu; break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				ret.type = AdapterType::virtual_gpu; break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				ret.type = AdapterType::software; break;
			default:
				ret.type = AdapterType::unknwon;
			}
			return ret;
		}
	}
}