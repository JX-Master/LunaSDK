/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SwapChain.cpp
* @author JXMaster
* @date 2022/10/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#include "SwapChain.hpp"
#include "Instance.hpp"
#include "SurfaceBind.hpp"
#include <Luna/Runtime/StackAllocator.hpp>

#ifdef LUNA_PLATFORM_ANDROID
#include <Luna/Window/Android/AndroidWindow.hpp>
#endif

#ifdef LUNA_RHI_DEBUG
#include <Luna/Runtime/Log.hpp>
#endif

namespace Luna
{
    namespace RHI
    {
        inline R<VkColorSpaceKHR> encode_color_space(ColorSpace color_space)
        {
            switch(color_space)
            {
                case ColorSpace::srgb:          return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                case ColorSpace::scrgb_linear:  return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
                case ColorSpace::bt2020:        return VK_COLOR_SPACE_HDR10_ST2084_EXT;
                case ColorSpace::display_p3:    return VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT;
                default: return RHIError::color_space_not_supported();
            }
        }
        inline ColorSpace decode_color_space(VkColorSpaceKHR color_space)
        {
            switch(color_space)
            {
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:          return ColorSpace::srgb;
                case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:    return ColorSpace::scrgb_linear;
                case VK_COLOR_SPACE_HDR10_ST2084_EXT:            return ColorSpace::bt2020;
                case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:    return ColorSpace::display_p3;
                default: lupanic(); return ColorSpace::unspecified;
            }
        }
#if defined(LUNA_RHI_DEBUG)
        const c8* print_vk_colorspace(VkColorSpaceKHR color_space)
        {
            switch(color_space)
            {
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
                case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
                case VK_COLOR_SPACE_HDR10_ST2084_EXT: return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
                default: lupanic(); return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
            }
        }
        const c8* print_vk_format(VkFormat format)
        {
            switch(format)
            {
                case VK_FORMAT_UNDEFINED:                return "VK_FORMAT_UNDEFINED";
                case VK_FORMAT_R8_UNORM:                 return "VK_FORMAT_R8_UNORM";
                case VK_FORMAT_R8_SNORM:                 return "VK_FORMAT_R8_SNORM";
                case VK_FORMAT_R8_UINT:                  return "VK_FORMAT_R8_UINT";
                case VK_FORMAT_R8_SINT:                  return "VK_FORMAT_R8_SINT";
                case VK_FORMAT_R16_UNORM:                return "VK_FORMAT_R16_UNORM";
                case VK_FORMAT_R16_SNORM:                return "VK_FORMAT_R16_SNORM";
                case VK_FORMAT_R16_UINT:                 return "VK_FORMAT_R16_UINT";
                case VK_FORMAT_R16_SINT:                 return "VK_FORMAT_R16_SINT";
                case VK_FORMAT_R16_SFLOAT:               return "VK_FORMAT_R16_SFLOAT";
                case VK_FORMAT_R8G8_UNORM:               return "VK_FORMAT_R8G8_UNORM";
                case VK_FORMAT_R8G8_SNORM:               return "VK_FORMAT_R8G8_SNORM";
                case VK_FORMAT_R8G8_UINT:                return "VK_FORMAT_R8G8_UINT";
                case VK_FORMAT_R8G8_SINT:                return "VK_FORMAT_R8G8_SINT";
                case VK_FORMAT_R32_UINT:                 return "VK_FORMAT_R32_UINT";
                case VK_FORMAT_R32_SINT:                 return "VK_FORMAT_R32_SINT";
                case VK_FORMAT_R32_SFLOAT:               return "VK_FORMAT_R32_SFLOAT";
                case VK_FORMAT_R16G16_UNORM:             return "VK_FORMAT_R16G16_UNORM";
                case VK_FORMAT_R16G16_SNORM:             return "VK_FORMAT_R16G16_SNORM";
                case VK_FORMAT_R16G16_UINT:              return "VK_FORMAT_R16G16_UINT";
                case VK_FORMAT_R16G16_SINT:              return "VK_FORMAT_R16G16_SINT";
                case VK_FORMAT_R16G16_SFLOAT:            return "VK_FORMAT_R16G16_SFLOAT";
                case VK_FORMAT_R8G8B8A8_UNORM:           return "VK_FORMAT_R8G8B8A8_UNORM";
                case VK_FORMAT_R8G8B8A8_SRGB:            return "VK_FORMAT_R8G8B8A8_SRGB";
                case VK_FORMAT_R8G8B8A8_SNORM:           return "VK_FORMAT_R8G8B8A8_SNORM";
                case VK_FORMAT_R8G8B8A8_UINT:            return "VK_FORMAT_R8G8B8A8_UINT";
                case VK_FORMAT_R8G8B8A8_SINT:            return "VK_FORMAT_R8G8B8A8_SINT";
                case VK_FORMAT_B8G8R8A8_UNORM:           return "VK_FORMAT_B8G8R8A8_UNORM";
                case VK_FORMAT_B8G8R8A8_SRGB:            return "VK_FORMAT_B8G8R8A8_SRGB";
                case VK_FORMAT_R32G32_UINT:              return "VK_FORMAT_R32G32_UINT";
                case VK_FORMAT_R32G32_SINT:              return "VK_FORMAT_R32G32_SINT";
                case VK_FORMAT_R32G32_SFLOAT:            return "VK_FORMAT_R32G32_SFLOAT";
                case VK_FORMAT_R16G16B16A16_UNORM:       return "VK_FORMAT_R16G16B16A16_UNORM";
                case VK_FORMAT_R16G16B16A16_SNORM:       return "VK_FORMAT_R16G16B16A16_SNORM";
                case VK_FORMAT_R16G16B16A16_UINT:        return "VK_FORMAT_R16G16B16A16_UINT";
                case VK_FORMAT_R16G16B16A16_SINT:        return "VK_FORMAT_R16G16B16A16_SINT";
                case VK_FORMAT_R16G16B16A16_SFLOAT:      return "VK_FORMAT_R16G16B16A16_SFLOAT";
                case VK_FORMAT_R32G32B32_UINT:           return "VK_FORMAT_R32G32B32_UINT";
                case VK_FORMAT_R32G32B32_SINT:           return "VK_FORMAT_R32G32B32_SINT";
                case VK_FORMAT_R32G32B32_SFLOAT:         return "VK_FORMAT_R32G32B32_SFLOAT";
                case VK_FORMAT_R32G32B32A32_UINT:        return "VK_FORMAT_R32G32B32A32_UINT";
                case VK_FORMAT_R32G32B32A32_SINT:        return "VK_FORMAT_R32G32B32A32_SINT";
                case VK_FORMAT_R32G32B32A32_SFLOAT:      return "VK_FORMAT_R32G32B32A32_SFLOAT";
                case VK_FORMAT_R5G6B5_UNORM_PACK16:      return "VK_FORMAT_R5G6B5_UNORM_PACK16";
                case VK_FORMAT_A1R5G5B5_UNORM_PACK16:    return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
                case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
                case VK_FORMAT_A2B10G10R10_UINT_PACK32:  return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
                case VK_FORMAT_B10G11R11_UFLOAT_PACK32:  return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
                case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:   return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
                case VK_FORMAT_D16_UNORM:                return "VK_FORMAT_D16_UNORM";
                case VK_FORMAT_D32_SFLOAT:               return "VK_FORMAT_D32_SFLOAT";
                case VK_FORMAT_D24_UNORM_S8_UINT:        return "VK_FORMAT_D24_UNORM_S8_UINT";
                case VK_FORMAT_D32_SFLOAT_S8_UINT:       return "VK_FORMAT_D32_SFLOAT_S8_UINT";
                case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
                case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:      return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
                case VK_FORMAT_BC2_UNORM_BLOCK:          return "VK_FORMAT_BC2_UNORM_BLOCK";
                case VK_FORMAT_BC2_SRGB_BLOCK:           return "VK_FORMAT_BC2_SRGB_BLOCK";
                case VK_FORMAT_BC3_UNORM_BLOCK:          return "VK_FORMAT_BC3_UNORM_BLOCK";
                case VK_FORMAT_BC3_SRGB_BLOCK:           return "VK_FORMAT_BC3_SRGB_BLOCK";
                case VK_FORMAT_BC4_UNORM_BLOCK:          return "VK_FORMAT_BC4_UNORM_BLOCK";
                case VK_FORMAT_BC4_SNORM_BLOCK:          return "VK_FORMAT_BC4_SNORM_BLOCK";
                case VK_FORMAT_BC5_UNORM_BLOCK:          return "VK_FORMAT_BC5_UNORM_BLOCK";
                case VK_FORMAT_BC5_SNORM_BLOCK:          return "VK_FORMAT_BC5_SNORM_BLOCK";
                case VK_FORMAT_BC6H_SFLOAT_BLOCK:        return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
                case VK_FORMAT_BC6H_UFLOAT_BLOCK:        return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
                case VK_FORMAT_BC7_UNORM_BLOCK:          return "VK_FORMAT_BC7_UNORM_BLOCK";
                case VK_FORMAT_BC7_SRGB_BLOCK:           return "VK_FORMAT_BC7_SRGB_BLOCK";
                default: lupanic(); return "VK_FORMAT_UNDEFINED";
            }
        }
#endif

        R<VkSurfaceFormatKHR> choose_swap_surface_format(const Vector<VkSurfaceFormatKHR>& available_formats, Format desired_format, ColorSpace desired_color_space)
        {
            for (const auto& format : available_formats)
            {
                if(desired_format != Format::unknown)
                {
                    VkFormat desired_vk_format = encode_format(desired_format);
                    if(desired_vk_format != format.format)
                    {
                        continue;
                    }
                }
                if(desired_color_space != ColorSpace::unspecified)
                {
                    auto res = encode_color_space(desired_color_space);
                    if(failed(res)) return res.errcode();
                    VkColorSpaceKHR desired_vk_color_space = res.get();
                    if(desired_vk_color_space != format.colorSpace)
                    {
                        continue;
                    }
                }
                return format;
            }
#if defined(LUNA_RHI_DEBUG)
            log_error("RHI", "The specified pixel format for swap chain is not supported.");
            if(desired_format != Format::unknown)
            {
                log_error("RHI", "Requested format: %s", print_vk_format(encode_format(desired_format)));
            }
            if(desired_color_space != ColorSpace::unspecified)
            {
                log_error("RHI", "Requested color spce: %s", print_vk_colorspace(encode_color_space(desired_color_space).get()));
            }
            log_error("RHI", "Supported formats:");
            for(auto& format : available_formats)
            {
                log_error("RHI", "    {%s, %s}", print_vk_format(format.format), print_vk_colorspace(format.colorSpace));
            }
#endif
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
#ifdef LUNA_PLATFORM_ANDROID
                Window::IAndroidWindow* android_window = query_interface<Window::IAndroidWindow>(window);
                m_native_window = (ANativeWindow*)android_window->get_native_window();
#endif
                luset(m_surface, new_surface_from_window(g_vk_instance, m_window));
                luexp(create_swap_chain(desc));
                VkFenceCreateInfo fence_create_info{};
                fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fence_create_info.flags = 0;
                luexp(encode_vk_result(m_device->m_funcs.vkCreateFence(m_device->m_device, &fence_create_info, nullptr, &m_acqure_fence)));
            }
            lucatchret;
            return ok;
        }
        RV SwapChain::wait_until_queue_empty()
        {
            MutexGuard guard(m_queue.queue_mtx);
            return encode_vk_result(m_device->m_funcs.vkQueueWaitIdle(m_queue.queue));
        }
        RV SwapChain::create_swap_chain(const SwapChainDesc& desc)
        {
            StackAllocator salloc;
            lutry
            {
                // Free old swap chain images.
                if(m_swap_chain)
                {
                    luexp(wait_until_queue_empty());
                    m_swap_chain_images.clear();
                }
#ifdef LUNA_PLATFORM_ANDROID
                {
                    // Check if we need to recreate surface. 
                    // This happens when native window is terminated and recreated (because app is switched out or screen is locked).
                    Window::IAndroidWindow* android_window = query_interface<Window::IAndroidWindow>(m_window);
                    ANativeWindow* native_window = (ANativeWindow*)android_window->get_native_window();
                    if(native_window != m_native_window)
                    {
                        if(m_swap_chain != VK_NULL_HANDLE)
                        {
                            m_device->m_funcs.vkDestroySwapchainKHR(m_device->m_device, m_swap_chain, nullptr);
                            m_swap_chain = VK_NULL_HANDLE;
                        }
                        if (m_surface != VK_NULL_HANDLE)
                        {
                            vkDestroySurfaceKHR(g_vk_instance, m_surface, nullptr);
                            m_surface = VK_NULL_HANDLE;
                        }
                        luset(m_surface, new_surface_from_window(g_vk_instance, m_window));
                        m_native_window = native_window;
                    }
                }
#endif
                VkSwapchainKHR old_swap_chain = m_swap_chain;
                m_desc = desc;
                auto framebuffer_size = m_window->get_framebuffer_size();
                m_desc.width = m_desc.width == 0 ? framebuffer_size.x : m_desc.width;
                m_desc.height = m_desc.height == 0 ? framebuffer_size.y : m_desc.height;
                if (!test_flags(m_queue.desc.flags, CommandQueueFlag::presenting))
                {
                    return set_error(BasicError::not_supported(), "The specified command queue for creating swap chain does not have presenting support");
                }
                auto surface_info = get_physical_device_surface_info(m_device->m_physical_device, m_surface);
                lulet(surface_format, choose_swap_surface_format(surface_info.formats, m_desc.format, m_desc.color_space));
                auto present_mode = choose_present_mode(surface_info.present_modes, m_desc.vertical_synchronized);
                lulet(extent, choose_swap_extent(surface_info.capabilities, m_desc));
                if (m_desc.buffer_count == 0)
                {
                    m_desc.buffer_count = surface_info.capabilities.minImageCount;
                }
                if (m_desc.buffer_count < surface_info.capabilities.minImageCount || m_desc.buffer_count > surface_info.capabilities.maxImageCount)
                {
                    return set_error(BasicError::not_supported(),
                        "The specified buffer count is not supported by the current window. Specified buffer count is %u, supported range is %u-%u",
                        m_desc.buffer_count, surface_info.capabilities.minImageCount, surface_info.capabilities.maxImageCount);
                }
                m_desc.format = decode_format(surface_format.format);
                m_desc.color_space = decode_color_space(surface_format.colorSpace);
                VkSwapchainCreateInfoKHR create_info{};
                create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                create_info.surface = m_surface;
                create_info.minImageCount = m_desc.buffer_count;
                create_info.imageFormat = surface_format.format;
                create_info.imageColorSpace = surface_format.colorSpace;
                create_info.imageExtent = extent;
                create_info.imageArrayLayers = 1;
                create_info.imageUsage = surface_info.capabilities.supportedUsageFlags;
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                create_info.queueFamilyIndexCount = 0;
                create_info.pQueueFamilyIndices = nullptr;
                create_info.preTransform = surface_info.capabilities.currentTransform;
                create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
                create_info.presentMode = present_mode;
                create_info.clipped = VK_TRUE;
                create_info.oldSwapchain = old_swap_chain;
                luexp(encode_vk_result(m_device->m_funcs.vkCreateSwapchainKHR(m_device->m_device, &create_info, nullptr, &m_swap_chain)));
                if(old_swap_chain)
                {
                    m_device->m_funcs.vkDestroySwapchainKHR(m_device->m_device, old_swap_chain, nullptr);
                }
                u32 image_count;
                luexp(encode_vk_result(m_device->m_funcs.vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, nullptr)));
                VkImage* images = (VkImage*)salloc.allocate(sizeof(VkImage) * image_count);
                luexp(encode_vk_result(m_device->m_funcs.vkGetSwapchainImagesKHR(m_device->m_device, m_swap_chain, &image_count, images)));
                TextureDesc tex_desc = TextureDesc::tex2d(
                    m_desc.format,
                    TextureUsageFlag::color_attachment,
                    m_desc.width,
                    m_desc.height,
                    1, 1, 1, ResourceFlag::none
                );
                for (u32 i = 0; i < image_count; ++i)
                {
                    auto res = new_object<ImageResource>();
                    res->m_device = m_device;
                    res->m_desc = tex_desc;
                    res->m_image = images[i];
                    res->m_global_states.emplace_back();
                    res->m_is_image_externally_managed = true;
                    m_swap_chain_images.push_back(res);
                }
                m_back_buffer_fetched = false;
                m_reset_suggested = false;
            }
            lucatchret;
            return ok;
        }
        SwapChain::~SwapChain()
        {
            m_swap_chain_images.clear();
            if (m_swap_chain != VK_NULL_HANDLE)
            {
                m_device->m_funcs.vkDestroySwapchainKHR(m_device->m_device, m_swap_chain, nullptr);
                m_swap_chain = VK_NULL_HANDLE;
            }
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
        SwapChainSurfaceTransform SwapChain::get_surface_transform()
        {
            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physical_device, m_surface, &capabilities);
            switch(capabilities.currentTransform)
            {
                case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR: return SwapChainSurfaceTransform::unspecified;
                case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR: return SwapChainSurfaceTransform::identity;
                case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR: return SwapChainSurfaceTransform::rotate_90;
                case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR: return SwapChainSurfaceTransform::rotate_180;
                case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR: return SwapChainSurfaceTransform::rotate_270;
                case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR: return SwapChainSurfaceTransform::horizontal_mirror;
                case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR: return SwapChainSurfaceTransform::horizontal_mirror_rotate_90;
                case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR: return SwapChainSurfaceTransform::horizontal_mirror_rotate_180;
                case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR: return SwapChainSurfaceTransform::horizontal_mirror_rotate_270;
                default: lupanic();
            }
            return SwapChainSurfaceTransform::unspecified;
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
                VkResult res = m_device->m_funcs.vkQueuePresentKHR(m_queue.queue, &present_info);
                if(res == VK_SUBOPTIMAL_KHR)
                {
                    m_reset_suggested = true;
                }
                luexp(encode_vk_result(res));
                m_back_buffer_fetched = false;
            }
            lucatchret;
            return ok;
        }
        bool SwapChain::reset_suggested()
        {
#ifdef LUNA_PLATFORM_ANDROID
            Window::IAndroidWindow* android_window = query_interface<Window::IAndroidWindow>(m_window);
            ANativeWindow* native_window = (ANativeWindow*)android_window->get_native_window();
            if(native_window != m_native_window)
            {
                m_reset_suggested = true;
            }
#endif
            return m_reset_suggested;
        }
        RV SwapChain::reset(const SwapChainDesc& desc)
        {
            // Wait for all presenting calls to finish.
            lutry
            {
                auto new_desc = desc;
                if (new_desc.width == 0)
                {
                    new_desc.width = m_desc.width;
                }
                if (new_desc.height == 0)
                {
                    new_desc.height = m_desc.height;
                }
                if (new_desc.format == Format::unknown)
                {
                    new_desc.format = m_desc.format;
                }
                if (new_desc.color_space == ColorSpace::unspecified)
                {
                    new_desc.color_space = m_desc.color_space;
                }
                if (new_desc.buffer_count == 0)
                {
                    new_desc.buffer_count = m_desc.buffer_count;
                }
                luexp(create_swap_chain(new_desc));
            }
            lucatchret;
            return ok;
        }
    }
}