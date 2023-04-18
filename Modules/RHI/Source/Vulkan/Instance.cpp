// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file Instance.cpp
* @author JXMaster
* @date 2023/4/16
*/
#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include <Runtime/Log.hpp>

namespace Luna
{
	namespace RHI
	{
		//! The global Vulkan instance.
		VkInstance g_vk_instance;
		VkDebugUtilsMessengerEXT g_vk_debug_messenger = VK_NULL_HANDLE;

		bool g_enable_validation_layers = false;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		)
		{
			LogMessage msg;
			switch (messageType)
			{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				msg.sender = "Vulkan::General"; break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				msg.sender = "Vulkan::Validation"; break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				msg.sender = "Vulkan::Performance"; break;
			}
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				msg.verbosity = LogVerbosity::verbose; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				msg.verbosity = LogVerbosity::info; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				msg.verbosity = LogVerbosity::warning; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				msg.verbosity = LogVerbosity::error; break;
			default:
				lupanic(); break;
			}
			msg.message = pCallbackData->pMessage;
			log(msg);
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

			for (const c8* layer_name : VK_ENABLED_LAYERS)
			{
				bool layer_found = false;
				for (u32 i = 0; i < layer_count; ++i)
				{
					if (strcmp(layer_name, available_layers[i].layerName) == 0)
					{
						layer_found = true;
						break;
					}
				}
				if (!layer_found)
				{
					return false;
				}
			}
			return true;
		}

		static RV setup_debug_messanger()
		{
			lutry
			{
				if (g_enable_validation_layers)
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
			g_vk_instance = VK_NULL_HANDLE;
			lutry
			{
				VkApplicationInfo app_info{};
				app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				app_info.pApplicationName = "Luna Engine";
				app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				app_info.pEngineName = "Luna Engine";
				app_info.engineVersion = VK_MAKE_VERSION(0, 8, 0);
				app_info.apiVersion = VK_API_VERSION_1_2;
				VkInstanceCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pApplicationInfo = &app_info;
				u32 glfw_extensions_count = 0;
				const c8** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
				Vector<const c8*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);
				g_enable_validation_layers = false;
#ifdef LUNA_RHI_DEBUG
				if (check_validation_layer_support())
				{
					g_enable_validation_layers = true;
				}
#endif
				if (g_enable_validation_layers)
				{
					extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
				create_info.enabledExtensionCount = (u32)extensions.size();
				create_info.ppEnabledExtensionNames = extensions.data();
				VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
				if (g_enable_validation_layers)
				{
					// Enable validation layer.
					create_info.enabledLayerCount = (u32)NUM_VK_ENABLED_LAYERS;
					create_info.ppEnabledLayerNames = VK_ENABLED_LAYERS;
					init_debug_messenger_create_info(debug_create_info);
					create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
				}
				else
				{
					create_info.enabledLayerCount = 0;
					create_info.pNext = nullptr;
				}
				luexp(encode_vk_result(vkCreateInstance(&create_info, nullptr, &g_vk_instance)));
				luexp(setup_debug_messanger());
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
		}
	}
}