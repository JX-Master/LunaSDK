/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Thread.hpp>

namespace Luna
{
	Ref<Window::IWindow> g_window;

	Ref<RHI::ISwapChain> g_swap_chain;
	Ref<RHI::ICommandBuffer> g_command_buffer;
	u32 g_command_queue;

	Ref<VG::IShapeDrawList> g_shape_draw_list;
	Ref<VG::IShapeRenderer> g_shape_renderer;
}

using namespace Luna;

RV recreate_window_resources(u32 width, u32 height)
{
	using namespace RHI;
	lutry
	{
		if (width && height)
		{
			if (!g_swap_chain)
			{
				g_swap_chain = get_main_device()->new_swap_chain(g_command_queue, g_window, SwapChainDesc({ width, height, 2, Format::bgra8_unorm, true})).get();
			}
			else
			{
				luexp(g_swap_chain->reset({width, height, 2, Format::bgra8_unorm, true}));
			}
		}
	}
	lucatchret;
	return ok;
}

void on_window_resize(Window::IWindow* window, u32 width, u32 height)
{
	lupanic_if_failed(recreate_window_resources(width, height));
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
	auto sz = g_window->get_size();

	g_window->get_close_event().add_handler(on_window_close);
	g_window->get_framebuffer_resize_event().add_handler(on_window_resize);

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

	lupanic_if_failed(recreate_window_resources(sz.x, sz.y));
	g_shape_renderer = VG::new_fill_shape_renderer(g_swap_chain->get_current_back_buffer().get()).get();
	g_command_buffer = dev->new_command_buffer(g_command_queue).get();

	/*g_shape_atlas = VG::new_shape_atlas();
    RectF rect = RectF(0.0f, 0.0f, 50.0f, 50.0f);
	g_shape_atlas->add_shape({ points.data(), points.size() }, &rect);

	points.clear();
	{
		using namespace VG::ShapeBuilder;
		move_to(points, 10.0f, 20.0f);
		circle_to(points, 10.0f, 90.0f, -270.0f);
	}
	g_shape_atlas->add_shape({ points.data(), points.size() }, nullptr);*/
}

void run()
{
	while (true)
	{
		//new_frame();
		Window::poll_events();

		if (g_window->is_closed()) break;
		if (g_window->is_minimized())
		{
			sleep(100);
			continue;
		}

		Vector<f32> points;
		{
			using namespace VG::ShapeBuilder;
			move_to(points, 10.0f, 50.0f);
			line_to(points, 40.0f, 50.0f);
			circle_to(points, 10.0f, 90.0f, 0.0f);
			line_to(points, 50.0f, 10.0f);
			circle_to(points, 10.0f, 0.0f, -90.0f);
			line_to(points, 10.0f, 0.0f);
			circle_to(points, 10.0f, -90.0f, -180.0f);
			line_to(points, 0.0f, 40.0f);
			circle_to(points, 10.0f, 180.0f, 90.0f);
		}
		u32 offset1, size1;
		offset1 = g_shape_draw_list->add_shape_points({ points.data(), points.size() });
		size1 = (u32)points.size();
		points.clear();
		{
			using namespace VG::ShapeBuilder;
			move_to(points, 10.0f, 20.0f);
			circle_to(points, 10.0f, 90.0f, -270.0f);
		}
		u32 offset2, size2;
		offset2 = g_shape_draw_list->add_shape_points({ points.data(), points.size() });
		size2 = (u32)points.size();

		RectF rect = RectF(0.0f, 0.0f, 50.0f, 50.0f);

		g_shape_draw_list->draw_shape(offset1, size1, Float2(100.0f, 100.0f), Float2U(500.0f, 500.0f), Float2U(rect.offset_x, rect.offset_y),
			Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height));

		rect = RectF(0.0f, 0.0f, 20.0f, 20.0f);

		g_shape_draw_list->draw_shape(offset2, size2, Float2(550.0f, 100.0f), Float2U(560.0f, 110.0f), Float2U(rect.offset_x, rect.offset_y),
			Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height));

		lupanic_if_failed(g_shape_draw_list->close());

		RHI::RenderPassDesc desc;
		auto texture = g_swap_chain->get_current_back_buffer().get();
		desc.color_attachments[0] = RHI::ColorAttachment(texture, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
		g_command_buffer->begin_render_pass(desc);
		g_command_buffer->end_render_pass();

		auto dcs = g_shape_draw_list->get_draw_calls();

		g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get());
		g_shape_renderer->render(g_command_buffer, g_shape_draw_list->get_vertex_buffer(), g_shape_draw_list->get_index_buffer(),  { dcs.data(), (u32)dcs.size() });

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
