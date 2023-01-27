/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.hpp
* @author JXMaster
* @date 2022/4/25
*/
#pragma once
#include "../VG.hpp"
#include <Runtime/TSAssert.hpp>

namespace Luna
{
	namespace VG
	{
		RV init_render_resources();
		void deinit_render_resources();

		extern const c8 FILL_SHADER_SOURCE_VS[];
		extern const c8 FILL_SHADER_SOURCE_PS[];
		extern usize FILL_SHADER_SOURCE_VS_SIZE;
		extern usize FILL_SHADER_SOURCE_PS_SIZE;

		struct FillShapeRenderer : IShapeRenderer
		{
			lustruct("RHI::FillShapeRenderer", "{3E50DDB9-C896-4B87-9000-BA8E5C7632BE}");
			luiimpl();
			lutsassert_lock();

			Ref<RHI::IResource> m_render_target;
			Ref<RHI::IRenderTargetView> m_rtv;
			u32 m_screen_width;
			u32 m_screen_height;

			Vector<Ref<RHI::IDescriptorSet>> m_desc_sets;
			Ref<RHI::IResource> m_cbs_resource;
			usize m_cbs_capacity;

			FillShapeRenderer() :
				m_screen_width(0),
				m_screen_height(0),
				m_cbs_capacity(0) {}

			RV init(RHI::IResource* render_target);

			void reset();

			RV set_render_target(RHI::IResource* render_target);

			RV render(
				RHI::ICommandBuffer* cmdbuf,
				RHI::IResource* shape_buffer,
				u32 num_points,
				RHI::IResource* vertex_buffer,
				u32 num_vertices,
				RHI::IResource* index_buffer,
				u32 num_indices,
				const ShapeDrawCall* draw_calls,
				u32 num_draw_calls
			);
		};
	}
}