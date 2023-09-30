/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DrawList.hpp
* @author JXMaster
* @date 2023/9/26
*/
#pragma once

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/RHI/Buffer.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/RHI/CommandBuffer.hpp>

namespace Luna
{
    namespace VG
    {
        struct DrawCall
        {
            //! The index of the first shape point of this draw call.
            u32 first_point;
            //! The number of shape points of this draw call.
            u32 num_points;
            //! The origin point for this draw call.
			Float2U origin_point;
            //! The rotation for this draw call.
			f32 rotation;
			//! The clip rect for this draw call.
			RectI clip_rect;
        };

        struct IDrawList : virtual Interface
        {
            luiid("{9853f471-89e1-4e49-9f14-fef1cfd30bb2}");
            
            virtual RV reset() = 0;

            virtual RHI::IBuffer* get_shape_buffer() = 0;

            virtual void set_shape_buffer(RHI::IBuffer* shape_buffer) = 0;

            virtual RHI::ITexture* get_texture() = 0;

            virtual void set_texture(RHI::ITexture* texture) = 0;

            virtual RHI::SamplerDesc get_sampler() = 0;

            virtual void set_sampler(const RHI::SamplerDesc& desc) = 0;

            virtual u32 add_shape_point(f32 point) = 0;

            virtual u32 add_shape_points(Span<f32> points) = 0;

            virtual void add_draw_call(const DrawCall& draw_call) = 0;

            virtual RV render(RHI::ICommandBuffer* cmdbuf) = 0;
        };
    }
}