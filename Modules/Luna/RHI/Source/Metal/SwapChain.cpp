/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.mm
* @author JXMaster
* @date 2023/8/3
*/
#include "SwapChain.hpp"
#include <Luna/Window/Cocoa/CocoaWindow.hpp>
namespace Luna
{
    namespace RHI
    {
        void SwapChain::init_metal_layer(const SwapChainDesc& desc)
        {
            AutoreleasePool pool;
            m_metal_layer = retain(CA::MetalLayer::layer());
            m_metal_layer->setDevice(m_device->m_device.get());
            m_metal_layer->setFramebufferOnly(true);
            m_metal_layer->setPixelFormat(encode_pixel_format(desc.format));
            CGSize size;
            size.width = desc.width;
            size.height = desc.height;
            m_metal_layer->setDrawableSize(size);
            bind_layer_to_window(m_window, m_metal_layer.get(), desc.buffer_count);
        }
        RV SwapChain::init(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc)
        {
            m_window = window;
            m_desc = desc;
            m_command_queue_index = command_queue_index;
            auto framebuffer_size = m_window->get_framebuffer_size();
			m_desc.width = m_desc.width == 0 ? framebuffer_size.x : m_desc.width;
			m_desc.height = m_desc.height == 0 ? framebuffer_size.y : m_desc.height;
            m_desc.format = m_desc.format == Format::unknown ? Format::bgra8_unorm_srgb : m_desc.format;
            init_metal_layer(m_desc);
            return ok;
        }
        R<ITexture*> SwapChain::get_current_back_buffer()
        {
            if(!m_current_back_buffer)
            {
                AutoreleasePool pool;
                m_current_drawable = retain(m_metal_layer->nextDrawable());
                MTL::Texture* texture = m_current_drawable->texture();
                Ref<Texture> tex = new_object<Texture>();
                tex->m_device = m_device;
                tex->m_texture = retain(texture);
                tex->m_desc.type = TextureType::tex2d;
                tex->m_desc.format = decode_pixel_format(texture->pixelFormat());
                tex->m_desc.width = (u32)texture->width();
                tex->m_desc.height = (u32)texture->height();
                tex->m_desc.depth = (u32)texture->depth();
                tex->m_desc.array_size = (u32)texture->arrayLength();
                tex->m_desc.mip_levels = (u32)texture->mipmapLevelCount();
                tex->m_desc.sample_count = (u32)texture->sampleCount();
                tex->m_desc.usages = decode_texture_usage(texture->usage(), false);
                tex->m_desc.flags = ResourceFlag::none;
                m_current_back_buffer = move(tex);
            }
            return m_current_back_buffer.get();
        }
        RV SwapChain::present()
        {
            lutry
            {
                if(!m_current_back_buffer)
                {
                    luexp(get_current_back_buffer());
                }
                AutoreleasePool pool;
                MTL::CommandQueue* queue = m_device->m_queues[m_command_queue_index].queue.get();
                MTL::CommandBuffer* buffer = queue->commandBuffer();
                buffer->presentDrawable(m_current_drawable.get());
                buffer->commit();
                m_current_back_buffer.reset();
                m_current_drawable.reset();
            }
            lucatchret;
            return ok;
        }
        RV SwapChain::reset(const SwapChainDesc& desc)
        {
            SwapChainDesc new_desc = desc;
            if(!new_desc.width) new_desc.width = m_desc.width;
            if(!new_desc.height) new_desc.height = m_desc.height;
            if(!new_desc.buffer_count) new_desc.buffer_count = m_desc.buffer_count;
            if(new_desc.format == Format::unknown) new_desc.format = m_desc.format;
            if(new_desc.buffer_count == m_desc.buffer_count && 
                new_desc.format == m_desc.format &&
                new_desc.vertical_synchronized == m_desc.vertical_synchronized)
            {
                CGSize size;
                size.width = new_desc.width;
                size.height = new_desc.height;
                m_metal_layer->setDrawableSize(size);
            }
            else
            {
                m_current_back_buffer.reset();
                m_current_drawable.reset();
                m_metal_layer.reset();
                init_metal_layer(m_desc);
            }
            m_desc = new_desc;
            return ok;
        }
    }
}
