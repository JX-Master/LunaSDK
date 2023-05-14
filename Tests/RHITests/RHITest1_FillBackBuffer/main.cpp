/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Debug.hpp>
#include <Runtime/Math/Color.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

RV start()
{

	return ok;
}

void draw()
{
	auto rtv = get_main_device()->new_render_target_view(get_back_buffer()).get();

	auto cb = get_command_buffer();
	cb->resource_barrier({},
		{
			{get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
		});
	cb->attach_device_object(rtv);
	RenderPassDesc render_pass;
	render_pass.color_attachments[0] = rtv;
	render_pass.color_load_ops[0] = LoadOp::dont_care;
	render_pass.color_store_ops[0] = StoreOp::store;
	cb->begin_render_pass(render_pass);
	auto clear_color = Color::blue_violet();
	cb->clear_color_attachment(0, clear_color.m, {});
	cb->end_render_pass();
	cb->resource_barrier({},
		{
			{get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::color_attachment_write, TextureStateFlag::present, ResourceBarrierFlag::none}
		});
	lupanic_if_failed(cb->submit({}, {}, true));
	cb->wait();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
}

void run_app()
{
	register_init_func(start);
	register_close_func(cleanup);
	register_resize_func(resize);
	register_draw_func(draw);
	lupanic_if_failed(run());
}

int main()
{
	if (!Luna::init()) return 0;
	auto r = init_modules();
	if (failed(r))
	{
		debug_printf("%s", explain(r.errcode()));
	}
	else run_app();
	Luna::close();
	return 0;
}