/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHITestBed.cpp
* @author JXMaster
* @date 2020/8/2
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RHI_TESTBED_API LUNA_EXPORT
#include "RHITestBed.hpp"
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Debug.hpp>
#include <Runtime/Log.hpp>
#include <Window/Window.hpp>

namespace Luna
{
	namespace RHITestBed
	{
		using namespace RHI;
		using namespace Window;

		void on_window_resize(IWindow* window, u32 width, u32 height);
		void on_window_close(IWindow* window);

		RV(*m_init_func)() = nullptr;
		void(*m_close_func)() = nullptr;
		void(*m_draw_func)() = nullptr;
		void(*m_resize_func)(u32 new_width, u32 new_height) = nullptr;

		Ref<ICommandQueue> m_queue;
		Ref<IWindow> m_window;
		Ref<ISwapChain> m_swap_chain;
		Ref<IResource> m_back_buffer;
		Ref<ICommandBuffer> m_command_buffer;

		LUNA_RHI_TESTBED_API void register_init_func(RV(*init_func)())
		{
			m_init_func = init_func;
		}
		LUNA_RHI_TESTBED_API void register_close_func(void(*close_func)())
		{
			m_close_func = close_func;
		}
		LUNA_RHI_TESTBED_API void register_draw_func(void(*draw_func)())
		{
			m_draw_func = draw_func;
		}
		LUNA_RHI_TESTBED_API void register_resize_func(void(*resize_func)(u32 new_width, u32 new_height))
		{
			m_resize_func = resize_func;
		}
		void on_window_resize(IWindow* window, u32 width, u32 height)
		{
			if(width && height)
			{
				// resize back buffer.
				lupanic_if_failed(m_swap_chain->reset({width, height, 2, Format::rgba8_unorm, true}));
				m_back_buffer = get_main_device()->new_resource(ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm,
					ResourceUsageFlag::render_target | ResourceUsageFlag::shader_resource, width, height, 1, 1), nullptr).get();
				if (m_resize_func) m_resize_func(width, height);
			}
		}
		void on_window_close(IWindow* window)
		{
			window->close();
		}
		RV init()
		{
			lutry
			{
				set_log_std_enabled(true);
				luset(m_queue, get_main_device()->new_command_queue(CommandQueueType::graphic));
				luset(m_window, new_window("RHI Test", WindowDisplaySettings::as_windowed(), WindowCreationFlag::resizable));
				m_window->get_close_event() += on_window_close;
				m_window->get_framebuffer_resize_event() += on_window_resize;
				luset(m_swap_chain, new_swap_chain(m_queue, m_window, SwapChainDesc({0, 0, 2, Format::rgba8_unorm, true})));
				auto sz = m_window->get_size();
				luset(m_back_buffer, get_main_device()->new_resource(ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm,
					ResourceUsageFlag::render_target | ResourceUsageFlag::shader_resource, sz.x, sz.y, 1, 1), nullptr));
				luset(m_command_buffer, m_queue->new_command_buffer());

				u32 adapter_index = 0;
				auto res = RHI::get_adapter_desc(adapter_index);
				while (succeeded(res))
				{
					log_info("RHITest", "Adapter %u", adapter_index);
					auto& desc = res.get();
					log_info("RHITest", "Name: %s", desc.name);
					log_info("RHITest", "Shared Memory: %.4f MB", (f64)desc.shared_memory / (f64)1_mb);
					log_info("RHITest", "Dedicated Memory: %.4f MB", (f64)desc.local_memory / (f64)1_mb);
					if (test_flags(desc.flags, RHI::GraphicAdapterFlag::software))
					{
						log_info("RHITest", "Software simulated.");
					}
					if (test_flags(desc.flags, RHI::GraphicAdapterFlag::uma))
					{
						log_info("RHITest", "Universal memory architecture.");
					}
					log_info("RHITest", "====================");
					++adapter_index;
					res = RHI::get_adapter_desc(adapter_index);
				}
			}
			lucatchret;
			return ok;
		}
		LUNA_RHI_TESTBED_API RV run()
		{
			auto r = init();
			if (failed(r)) return r.errcode();
			if (m_init_func)
			{
				auto r = m_init_func();
				if (failed(r))
				{
					log_error("%s", explain(r.errcode()));
					debug_break();
				}
			}
			while (true)
			{
				Window::poll_events();
				if (m_window->is_closed()) break;

				lupanic_if_failed(m_command_buffer->reset());

				if (m_draw_func) m_draw_func();

				lupanic_if_failed(m_swap_chain->present(m_back_buffer, 0));
				m_swap_chain->wait();
			}
			if (m_close_func) m_close_func();
			m_queue.reset();
			m_window.reset();
			m_swap_chain.reset();
			m_back_buffer.reset();
			m_command_buffer.reset();
			return ok;
		}
		LUNA_RHI_TESTBED_API RHI::IResource* get_back_buffer()
		{
			return m_back_buffer;
		}
		LUNA_RHI_TESTBED_API RHI::ICommandBuffer* get_command_buffer()
		{
			return m_command_buffer;
		}
		LUNA_RHI_TESTBED_API Window::IWindow* get_window()
		{
			return m_window;
		}
	}

	StaticRegisterModule testbed_module("RHITestBed", "RHI", nullptr, nullptr);
}