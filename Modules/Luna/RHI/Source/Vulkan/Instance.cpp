/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Instance.cpp
* @author JXMaster
* @date 2023/4/16
*/
#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
	namespace RHI
	{
		//! The global Vulkan instance.
		u32 g_vk_version;
		VkInstance g_vk_instance;
		VkDebugUtilsMessengerEXT g_vk_debug_messenger = VK_NULL_HANDLE;

		bool g_enable_validation_layer = false;
		Vector<const c8*> g_enabled_layers;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		)
		{
			const c8* tag = "";
			switch (messageType)
			{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				tag = "Vulkan::General"; break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				tag = "Vulkan::Validation"; break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				tag = "Vulkan::Performance"; break;
			}
			LogVerbosity verbosity;
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				verbosity = LogVerbosity::verbose; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				verbosity = LogVerbosity::info; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				verbosity = LogVerbosity::warning; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				verbosity = LogVerbosity::error; break;
			default:
				lupanic(); break;
			}
			log(verbosity, tag, "%s", pCallbackData->pMessage);
			return VK_FALSE;
		}

		inline void init_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debug_callback;
			createInfo.pUserData = nullptr;
		}

		static bool check_validation_layer_support()
		{
			u32 layer_count;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			VkLayerProperties* available_layers = (VkLayerProperties*)alloca(layer_count * sizeof(VkLayerProperties));
			vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

			bool layer_found = false;
			for (u32 i = 0; i < layer_count; ++i)
			{
				if (strcmp("VK_LAYER_KHRONOS_validation", available_layers[i].layerName) == 0)
				{
					layer_found = true;
					break;
				}
			}
			return layer_found;
		}

		static RV setup_debug_messanger()
		{
			lutry
			{
				if (g_enable_validation_layer)
				{
					VkDebugUtilsMessengerCreateInfoEXT create_info{};
					init_debug_messenger_create_info(create_info);
					auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vk_instance, "vkCreateDebugUtilsMessengerEXT");
					if (func != nullptr)
					{
						luexp(encode_vk_result(func(g_vk_instance, &create_info, nullptr, &g_vk_debug_messenger)));
					}
					else
					{
						luexp(encode_vk_result(VK_ERROR_EXTENSION_NOT_PRESENT));
					}
				}
			}
			lucatchret;
			return ok;
		}

		RV create_vk_instance()
		{
			g_vk_version = VK_API_VERSION_1_0;
			g_vk_instance = VK_NULL_HANDLE;
			lutry
			{
				luexp(encode_vk_result(volkInitialize()));
				//luexp(encode_vk_result(vkEnumerateInstanceVersion(&g_vk_version)));
				VkApplicationInfo app_info{};
				app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				app_info.pApplicationName = "Luna SDK";
				app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				app_info.pEngineName = "Luna SDK";
				app_info.engineVersion = VK_MAKE_VERSION(0, 8, 0);
				app_info.apiVersion = g_vk_version;
				VkInstanceCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pApplicationInfo = &app_info;
				u32 glfw_extensions_count = 0;
				const c8** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
				Vector<const c8*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);
				g_enable_validation_layer = false;
#if defined(LUNA_RHI_DEBUG) || defined(LUNA_DEBUG)
				if (check_validation_layer_support())
				{
					g_enable_validation_layer = true;
				}
#endif
				if (g_enable_validation_layer)
				{
					extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
				create_info.enabledExtensionCount = (u32)extensions.size();
				create_info.ppEnabledExtensionNames = extensions.data();
				VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
				if (g_enable_validation_layer)
				{
					// Enable validation layer.
					g_enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
					init_debug_messenger_create_info(debug_create_info);
					create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
				}
				create_info.enabledLayerCount = (u32)g_enabled_layers.size();
				create_info.ppEnabledLayerNames = g_enabled_layers.data();
				luexp(encode_vk_result(vkCreateInstance(&create_info, nullptr, &g_vk_instance)));
				luexp(setup_debug_messanger());
				volkLoadInstance(g_vk_instance);
			}
			lucatchret;
			return ok;
		}
		void destroy_vk_instance()
		{
			if (g_vk_debug_messenger != VK_NULL_HANDLE)
			{
				auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vk_instance, "vkDestroyDebugUtilsMessengerEXT");
				if (func != nullptr) {
					func(g_vk_instance, g_vk_debug_messenger, nullptr);
				}
				g_vk_debug_messenger = VK_NULL_HANDLE;
			}
			if (g_vk_instance != VK_NULL_HANDLE)
			{
				vkDestroyInstance(g_vk_instance, nullptr);
				g_vk_instance = VK_NULL_HANDLE;
			}
			g_enabled_layers.clear();
			g_enabled_layers.shrink_to_fit();
		}
	}
}