/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGuiTest.cpp
* @author JXMaster
* @date 2020/4/12
*/
#include <ImGui/ImGui.hpp>
#include <Runtime/Runtime.hpp>
#include <RHI/RHI.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Debug.hpp>

using namespace Luna;

void on_window_close(Window::IWindow* window)
{
	window->close();
}

void run()
{
	using namespace RHI;
	using namespace Window;
	Ref<IWindow> window = new_window("ImGui Demo", 0, 0, 0, 0, nullptr,
		WindowCreationFlag::default_size |
		WindowCreationFlag::position_center |
		WindowCreationFlag::minimizable |
		WindowCreationFlag::maximizable |
		WindowCreationFlag::resizable).get();
	window->get_close_event() += on_window_close;

	Ref<IDevice> dev = get_main_device();

	Ref<ICommandQueue> queue = dev->new_command_queue(CommandQueueType::graphic).get();

	Ref<ISwapChain> swap_chain = new_swap_chain(queue, window, SwapChainDesc(0, 0, Format::rgba8_unorm, 2)).get();

	Ref<ICommandBuffer> cmdbuf = queue->new_command_buffer().get();

	// Create back buffer.
	Ref<IResource> back_buffer;
	Ref<IRenderTargetView> back_buffer_rtv;
	u32 w = 0, h = 0;

	ImGuiUtils::set_active_window(window);

	Ref<IRenderTargetView> rt;

	while (true)
	{
		poll_events();

		if (window->is_closed())
		{
			break;
		}

		// Recreate the back buffer if needed.
		auto sz = window->get_size();
		auto ww = sz.x;
		auto wh = sz.y;
		if (!back_buffer || ww != w || wh != h)
		{
			swap_chain->resize_buffers(2, ww, wh, Format::unknown);
			f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			back_buffer = dev->new_resource(ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, ResourceUsageFlag::render_target, ww, wh, 1, 1),
				&ClearValue::as_color(Format::rgba8_unorm, clear_color)).get();
			back_buffer_rtv = dev->new_render_target_view(back_buffer).get();
			w = ww;
			h = wh;
			rt = dev->new_render_target_view(back_buffer).get();
		}

		ImGuiUtils::update_io();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		
		RenderPassDesc desc;
		desc.rtvs[0] = rt;
		desc.rt_load_ops[0] = LoadOp::clear;
		desc.rt_clear_values[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdbuf->begin_render_pass(desc);
		cmdbuf->end_render_pass();
		ImGuiUtils::render_draw_data(ImGui::GetDrawData(), cmdbuf, back_buffer_rtv);
		cmdbuf->submit();
		cmdbuf->wait();
		cmdbuf->reset();
		swap_chain->present(back_buffer, 0, 1);
		swap_chain->wait();
	}
}

int main()
{
	// Start modules.
	Luna::init();
	auto res = Luna::init_modules();
	if (failed(res))
	{
		debug_printf("Module init error: %s\n", explain(res.errcode()));
		lupanic();
		Luna::close();
		return 0;
	}
	run();
	Luna::close();
	return 0;
}