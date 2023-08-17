/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.hpp
* @author JXMaster
* @date 2023/8/3
*/
#pragma once
#include "Device.hpp"
#include "Resource.hpp"

namespace Luna
{
    namespace RHI
    {
        struct SwapChain : ISwapChain
        {
            lustruct("RHI::SwapChain", "{b0aba649-630a-44f7-9053-24711a974505}");
            luiimpl();

            Ref<Device> m_device;

            NSPtr<CA::MetalLayer> m_metal_layer;
            Ref<Window::IWindow> m_window;
            SwapChainDesc m_desc;

            Ref<Texture> m_current_back_buffer;
            NSPtr<CA::MetalDrawable> m_current_drawable;

            u32 m_command_queue_index;

            void init_metal_layer(const SwapChainDesc& desc);

            RV init(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { }
            virtual Window::IWindow* get_window() override { return m_window; }
            virtual SwapChainDesc get_desc() override { return m_desc; }
            virtual R<ITexture*> get_current_back_buffer() override;
            virtual RV present() override;
            virtual RV reset(const SwapChainDesc& desc) override;
        };

        void bind_layer_to_window(Window::IWindow* window, CA::MetalLayer* layer, u32 buffer_count);
    }
}