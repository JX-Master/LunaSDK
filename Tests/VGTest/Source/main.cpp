/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <VG/VG.hpp>
#include <Runtime/Math/Transform.hpp>
#include <Runtime/Math/Color.hpp>
#include <Runtime/Time.hpp>
#include <Runtime/File.hpp>
#include <Runtime/Debug.hpp>

namespace Luna
{
	Ref<Window::IWindow> g_window;

	Ref<RHI::ISwapChain> g_swap_chain;
	Ref<RHI::ICommandBuffer> g_command_buffer;
	u32 g_command_queue;

	Ref<VG::IShapeAtlas> g_shape_atlas;
	Ref<VG::IShapeDrawList> g_shape_draw_list;
	Ref<VG::IShapeRenderer> g_shape_renderer;
}

using namespace Luna;

RV recreate_window_resources()
{
	using namespace RHI;
	lutry
	{
		auto sz = g_window->get_size();
		{
			if (!g_swap_chain)
			{
				g_swap_chain = get_main_device()->new_swap_chain(g_command_queue, g_window, SwapChainDesc({sz.x, sz.y, 2, Format::bgra8_unorm, true})).get();
			}
			else
			{
				g_swap_chain->reset({sz.x, sz.y, 2, Format::bgra8_unorm, true});
			}
		}
	}
	lucatchret;
	return ok;
}

void on_window_resize(Window::IWindow* window, u32 width, u32 height)
{
	lupanic_if_failed(recreate_window_resources());
	lupanic_if_failed(g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get()));
}

void on_window_close(Window::IWindow* window)
{
	window->close();
}

void init()
{
	lupanic_if_failed(init_modules());
	// register event.
	g_window = Window::new_window("Luna Vector Graphics Test", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::resizable).get();

	g_window->get_close_event() += on_window_close;
	g_window->get_framebuffer_resize_event() += on_window_resize;

	g_shape_draw_list = VG::new_shape_draw_list();

	auto dev = RHI::get_main_device();

	g_command_queue = U32_MAX;
	u32 num_queues = dev->get_num_command_queues();
	for (u32 i = 0; i < num_queues; ++i)
	{
		auto desc = dev->get_command_queue_desc(i);
		if (desc.type == RHI::CommandQueueType::graphics)
		{
			g_command_queue = i;
			break;
		}
	}

	lupanic_if_failed(recreate_window_resources());
	g_shape_renderer = VG::new_fill_shape_renderer(g_swap_chain->get_current_back_buffer().get()).get();
	g_command_buffer = dev->new_command_buffer(g_command_queue).get();
	g_shape_atlas = VG::new_shape_atlas();

	// Window.
	VG::ShapeBuilder builder;
	builder.move_to(10.0f, 50.0f);
	builder.line_to(40.0f, 50.0f);
	builder.circle_to(10.0f, 90.0f, 0.0f);
	builder.line_to(50.0f, 10.0f);
	builder.circle_to(10.0f, 0.0f, -90.0f);
	builder.line_to(10.0f, 0.0f);
	builder.circle_to(10.0f, -90.0f, -180.0f);
	builder.line_to(0.0f, 40.0f);
	builder.circle_to(10.0f, 180.0f, 90.0f);

	g_shape_atlas = VG::new_shape_atlas();
	g_shape_atlas->add_shape({ builder.points.data(), builder.points.size() }, &RectF(0.0f, 0.0f, 50.0f, 50.0f));

	builder.points.clear();
	builder.move_to(10.0f, 20.0f);
	builder.circle_to(10.0f, 90.0f, -270.0f);
	g_shape_atlas->add_shape({ builder.points.data(), builder.points.size() }, nullptr);
}

void run()
{
	while (true)
	{
		//new_frame();
		Window::poll_events();

		if (g_window->is_closed()) break;

		auto sz = g_window->get_size();
		
		g_shape_draw_list->set_shape_atlas(g_shape_atlas);
		usize offset, size;
		RectF rect;
		
		g_shape_atlas->get_shape(0, &offset, &size, &rect);
		g_shape_draw_list->draw_shape(offset, size, Float2(100.0f, 100.0f), Float2U(500.0f, 500.0f), Float2U(rect.offset_x, rect.offset_y),
			Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height));

		g_shape_atlas->get_shape(1, &offset, &size, &rect);
		g_shape_draw_list->draw_shape(offset, size, Float2(550.0f, 100.0f), Float2U(560.0f, 110.0f), Float2U(rect.offset_x, rect.offset_y),
			Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height));

		lupanic_if_failed(g_shape_draw_list->close());

		RHI::RenderPassDesc desc;
		auto texture = g_swap_chain->get_current_back_buffer().get();
		desc.color_attachments[0] = RHI::ColorAttachment(texture, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
		g_command_buffer->begin_render_pass(desc);
		g_command_buffer->end_render_pass();

		auto dcs = g_shape_draw_list->get_draw_calls();

		g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get());
		g_shape_renderer->render(g_command_buffer, g_shape_atlas->get_shape_resource().get(), g_shape_atlas->get_shape_resource_size(),
			g_shape_draw_list->get_vertex_buffer(), g_shape_draw_list->get_vertex_buffer_size(),
			g_shape_draw_list->get_index_buffer(), g_shape_draw_list->get_index_buffer_size(),
			dcs.data(), (u32)dcs.size());

		g_command_buffer->resource_barrier({},
		{
			{g_swap_chain->get_current_back_buffer().get(), RHI::SubresourceIndex(0, 0), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
		});
		
		g_command_buffer->submit({}, {}, true);
		g_command_buffer->wait();

		g_swap_chain->present();
		g_command_buffer->reset();
		g_shape_renderer->reset();
		g_shape_draw_list->reset();
	}
}
void shutdown()
{
	g_window = nullptr;

	g_swap_chain = nullptr;
	g_command_buffer = nullptr;
	g_shape_atlas = nullptr;
	g_shape_draw_list = nullptr;
	g_shape_renderer = nullptr;
}
int main()
{
	Luna::init();
	::init();
	run();
	::shutdown();
	Luna::close();
}