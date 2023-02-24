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
#include <Font/Font.hpp>
#include <Runtime/Math/Transform.hpp>
#include <Runtime/Math/Color.hpp>
#include <Runtime/Time.hpp>
#include <Runtime/File.hpp>
#include <Runtime/Debug.hpp>

namespace Luna
{
	Ref<Window::IWindow> g_window;

	Ref<RHI::ICommandQueue> g_command_queue;

	Ref<RHI::ISwapChain> g_swap_chain;
	Ref<RHI::IResource> g_screen_tex;
	Ref<RHI::ICommandBuffer> g_command_buffer;
	Ref<RHI::IRenderTargetView> g_rtv;

	Ref<VG::IFontAtlas> g_font_atlas;
	Ref<VG::ITextArranger> g_text_arranger;
	Ref<VG::IShapeDrawList> g_shape_draw_list;
	Ref<VG::IShapeRenderer> g_shape_renderer;

	u32 g_frame_count;

	f32 g_font_size;
	f32 g_font_size_increment;

	u64 g_last_frame_ticks;

	f64 g_arrange_time;
	f64 g_render_time;
	f64 g_frame_time;
}

using namespace Luna;

RV recreate_window_resources()
{
	using namespace RHI;
	lutry
	{
		auto sz = g_window->get_size();
		{
			f32 clear_value[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			ResourceDesc desc = ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, ResourceUsageFlag::render_target,
				sz.x, sz.y);
			luset(g_screen_tex, get_main_device()->new_resource(desc, &ClearValue::as_color(Format::rgba8_unorm, clear_value)));
			
			luset(g_rtv, get_main_device()->new_render_target_view(g_screen_tex));
		}
		{
			if (!g_swap_chain)
			{
				g_swap_chain = new_swap_chain(g_command_queue, g_window, SwapChainDesc({sz.x, sz.y, 2, Format::rgba8_unorm, true})).get();
			}
			else
			{
				g_swap_chain->reset({sz.x, sz.y, 2, Format::rgba8_unorm, true});
			}
		}
	}
	lucatchret;
	return ok;
}

const c8* sample_text = u8"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

const f32 min_size = 10.0f;
const f32 max_size = 300.0f;

void on_window_resize(Window::IWindow* window, u32 width, u32 height)
{
	lupanic_if_failed(recreate_window_resources());
	lupanic_if_failed(g_shape_renderer->set_render_target(g_screen_tex));
}

void on_window_close(Window::IWindow* window)
{
	window->close();
}


void init()
{
	lupanic_if_failed(init_modules());

	// register event.
	g_window = Window::new_window("Luna Vector Graphics Test", 0, 0, 0, 0, nullptr, Window::WindowCreationFlag::default_size |
		Window::WindowCreationFlag::position_center |
		Window::WindowCreationFlag::resizable |
		Window::WindowCreationFlag::minimizable |
		Window::WindowCreationFlag::maximizable).get();

	g_window->get_close_event() += on_window_close;
	g_window->get_framebuffer_resize_event() += on_window_resize;

	auto font = Font::get_default_font();

	g_shape_draw_list = VG::new_shape_draw_list();
	g_command_queue = RHI::get_main_device()->new_command_queue(RHI::CommandQueueType::graphic).get();
	lupanic_if_failed(recreate_window_resources());
	g_shape_renderer = VG::new_fill_shape_renderer(g_screen_tex).get();

	g_font_atlas = VG::new_font_atlas(font, 0);
	g_text_arranger = VG::new_text_arranger(g_font_atlas);
	g_command_buffer = g_command_queue->new_command_buffer().get();
	g_font_size = 30.0f;
	g_font_size_increment = 1;

	g_arrange_time = 0;
	g_render_time = 0;
	g_frame_time = 0;
	g_frame_count = 0;
}

void run()
{
	while (true)
	{
		//new_frame();
		Window::poll_events();
		if (g_window->is_closed()) break;

		auto sz = g_window->get_size();

		/*g_font_size += g_font_size_increment;
		if (g_font_size >= max_size)
		{
			g_font_size = max_size;
			g_font_size_increment = -g_font_size_increment;
		}
		else if (g_font_size <= min_size)
		{
			g_font_size = min_size;
			g_font_size_increment = -g_font_size_increment;
		}*/

		f64 time1 = ((f64)get_ticks() / get_ticks_per_second()) * 1000;
		c8 buf[64];
		
		g_text_arranger->set_font_size(50.0f);
		g_text_arranger->set_font_color(0xCCFFCCFF);
		g_text_arranger->add_text("Arrange time: ");
		sprintf(buf, "%f", g_arrange_time / (f64)g_frame_count);
		g_text_arranger->add_text(buf);
		g_text_arranger->add_text("ms.\n");
		g_text_arranger->add_text("Render time: ");
		sprintf(buf, "%f", g_render_time / (f64)g_frame_count);
		g_text_arranger->add_text(buf);
		g_text_arranger->add_text("ms.\n");
		g_text_arranger->add_text("Frame time: ");
		sprintf(buf, "%f", g_frame_time);
		g_text_arranger->add_text(buf);
		g_text_arranger->add_text("ms.\n");

		g_text_arranger->set_font_color(Color::white().rgba8());
		g_text_arranger->set_font_size(g_font_size);
		for (usize i = 0; i < 300; ++i)
		{
			g_text_arranger->add_text(sample_text);
		}
		g_text_arranger->add_text("\n");

		auto res = g_text_arranger->arrange(RectF(0, 0, sz.x, sz.y), VG::TextAlignment::center, VG::TextAlignment::center);
		if (!res.lines.empty())
		{
			g_text_arranger->commit(res, g_shape_draw_list);
			g_text_arranger->clear_text_buffer();
		}

		f64 time2 = ((f64)get_ticks() / get_ticks_per_second()) * 1000;
		g_arrange_time += time2 - time1;
		
		lupanic_if_failed(g_shape_draw_list->close());

		RHI::RenderPassDesc desc;
		desc.rtvs[0] = g_rtv;
		desc.rt_load_ops[0] = RHI::LoadOp::clear;
		desc.rt_clear_values[0] = Float4U{ 0.0f };
		g_command_buffer->begin_render_pass(desc);
		g_command_buffer->end_render_pass();

		auto dcs = g_shape_draw_list->get_draw_calls();

		g_shape_renderer->render(g_command_buffer, g_font_atlas->get_shape_atlas()->get_shape_resource().get(), g_font_atlas->get_shape_atlas()->get_shape_resource_size(),
			g_shape_draw_list->get_vertex_buffer(), g_shape_draw_list->get_vertex_buffer_size(),
			g_shape_draw_list->get_index_buffer(), g_shape_draw_list->get_index_buffer_size(),
			dcs.data(), (u32)dcs.size());
		
		g_command_buffer->submit();
		g_command_buffer->wait();

		f64 time3 = ((f64)get_ticks() / get_ticks_per_second()) * 1000;
		g_render_time += time3 - time2;

		g_swap_chain->present(g_screen_tex, 0);
		g_swap_chain->wait();
		g_command_buffer->reset();
		g_shape_renderer->reset();
		g_shape_draw_list->reset();

		u64 frame_ticks = get_ticks();

		g_frame_time = f64(frame_ticks - g_last_frame_ticks) / get_ticks_per_second() * 1000.0;
		g_last_frame_ticks = frame_ticks;

		++g_frame_count;
	}
}
void shutdown()
{
	g_window = nullptr;

	g_command_queue = nullptr;

	g_swap_chain = nullptr;
	g_screen_tex = nullptr;
	g_command_buffer = nullptr;
	g_rtv = nullptr;

	g_font_atlas = nullptr;
	g_text_arranger = nullptr;
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