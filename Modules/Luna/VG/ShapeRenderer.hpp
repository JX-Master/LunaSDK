/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.hpp
* @author JXMaster
* @date 2023/9/27
*/
#pragma once
#include "ShapeDrawList.hpp"
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>

namespace Luna
{
    namespace VG
    {
        //! Holds resources that used to render shape draw calls.
		struct IShapeRenderer : virtual Interface
		{
			luiid("{C0FBD0AE-B7F6-4A82-A59B-B1115ACCBD94}");

			virtual void reset() = 0;

			virtual RV set_render_target(RHI::ITexture* render_target) = 0;

			virtual RV render(
				RHI::ICommandBuffer* cmdbuf,
                RHI::IBuffer* vertex_buffer,
                RHI::IBuffer* index_buffer,
				Span<const ShapeDrawCall> draw_calls,
				Float4x4U* transform_matrix = nullptr
			) = 0;
		};

		LUNA_VG_API R<Ref<IShapeRenderer>> new_fill_shape_renderer(RHI::ITexture* render_target);
    }
}