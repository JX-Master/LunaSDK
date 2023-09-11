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
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Log.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

RV start()
{

	return ok;
}

void draw()
{
	auto cb = get_command_buffer();
	cb->resource_barrier({},
		{
			{get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
		});
	RenderPassDesc render_pass;
	render_pass.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, Color::blue_violet());
	cb->begin_render_pass(render_pass);
	cb->end_render_pass();
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
		log_error("RHITest1_FillBackBuffer", "%s", explain(r.errcode()));
	}
	else run_app();
	Luna::close();
	return 0;
}