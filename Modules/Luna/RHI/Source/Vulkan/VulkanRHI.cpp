/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file VulkanRHI.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include "Common.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include "../RHI.hpp"
#include "VulkanDevice.hpp"
#include <Luna/Window/Window.hpp>
#include "Instance.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFence.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanQueryHeap.hpp"
#include "VulkanResource.hpp"
#include "VulkanSampler.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanSwapChain.hpp"
#include "RHI.meta.generated.hpp"
namespace Luna
{
    namespace RHI
    {
        RV render_api_init()
        {
            VkSurfaceKHR dummy_surface = VK_NULL_HANDLE;
            Ref<Window::IWindow> dummy_window;
            lutry
            {
                Meta::register_RHI_types();

                luexp(create_vk_instance());
                luexp(init_physical_devices());
                lulet(main_physical_device, select_main_physical_device());
                luset(g_main_device, new_device(main_physical_device));
            }
            lucatchret;
            return ok;
        }
        void render_api_close()
        {
            g_main_device.reset();
            clear_physical_devices();
            destroy_vk_instance();
        }
    }
}
