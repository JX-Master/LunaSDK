/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeDrawList.hpp
* @author JXMaster
* @date 2023/9/27
*/
#pragma once
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/RHI/DescriptorSet.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! Describes one vertex to be drawn for the glyph.
		struct Vertex
		{
			//! The position of the vertex.
			Float2U position;
			//! The shape coordinate of the vertex for mapping the shape
			//! commands.
			Float2U shapecoord;
			//! The texture coordinate of the vertex for sampling
			//! the attached resources.
			Float2U texcoord;
			//! An additional color that can be used to tint the vertex.
			u32 color;
			//! The offset of the first command for this shape in the shape buffer.
			u32 begin_command;
			//! The number of commands (f32 values) used for this shape.
			u32 num_commands;
		};

		struct ShapeDrawCall
		{
			//! The shape buffer bind to this draw call.
			RHI::IBuffer* shape_buffer;
			//! The texture bind to this draw call. May be `nullptr`.
			RHI::ITexture* texture;
			//! The attached sampler for this draw call.
			RHI::SamplerDesc sampler;
			//! The fist index to draw for this draw call.
			u32 base_index;
			//! The number of indices to draw for this draw call.
			u32 num_indices;
			//! The origin point for this draw call.
			Float2U origin_point;
			//! The rotation for this draw call.
			f32 rotation;
		};

		//! Represents a draw list that contains shapes to be drawn.
		struct IShapeDrawList : virtual Interface
		{
			luiid("{14F1CA71-7B2D-4072-A2EE-DFD64B62FCD5}");

			//! Resets the draw list. The call clears all shapes recorded, but retains their memory
			//! and resources, so they can be reused for new shapes.
			virtual void reset() = 0;

			virtual Vector<f32>& get_shape_points() = 0;

			//! Sets the shape buffer used for the following draw calls.
			virtual void set_shape_buffer(RHI::IBuffer* shape_buffer) = 0;

			//! Gets the current shape buffer.
			virtual RHI::IBuffer* get_shape_buffer() = 0;

			//! Sets the texture to be sampled when rendering the succeeding shapes.
			//! @param[in] tex The texture to set. Specify `nullptr` is allowed, which behaves the same as applying one white texture with all components set to
			//! 1.0f.
			//! @remark The draw list only stores the texture and its state as-is and provides it to the renderer 
			//! when the draw list is processed by the renderer. It does not do any validation to the texture and 
			//! its states. It is the user and renderer's responsibility to validate the texture and its state.
			//! 
			//! The draw list has texture being set to `nullptr` after reset.
			virtual void set_texture(RHI::ITexture* tex) = 0;

			//! Gets the currently set texture, returns `nullptr` if no texture is set.
			virtual RHI::ITexture* get_texture() = 0;

			//! Sets the sampler state to be used when sampling bound textures.
			//! Specify `nullptr` to reset the sampler state to initial settings.
			virtual void set_sampler(const RHI::SamplerDesc* desc) = 0;

			//! Gets the sampler state currently set.
			virtual RHI::SamplerDesc get_sampler() = 0;

			//! Sets the origin point for the following draw calls.
			//! The origin point is relative to the origin point of the canvas. 
			//! The origin point of the canvas is at the bottom-left corner. The x axis points to right and the y axis points to up.
			//! 
			//! The origin point is (0,0) when the draw list has been reset.
			//! @param[in] offset The x and y offset in pixels.
			virtual void set_origin(const Float2& origin) = 0;

			//! Gets the origin point for the following draw calls.
			virtual Float2 get_origin() = 0;

			//! Sets the rotation for the following draw calls, the rotation is relative to the set origin point.
			//! The rotation is specified in clockwise degrees.
			virtual void set_rotation(f32 degrees) = 0;

			//! Gets the rotation for the following draw calls.
			virtual f32 get_rotation() = 0;

			//! Draws one shape by submitting vertices and indices directly.
			virtual void draw_shape_raw(Span<const Vertex> vertices, Span<const u32> indices) = 0;

			//! Draws one shape. The shape is drawn by adding one draw rect (two triangles) to the list.
			virtual void draw_shape(u32 begin_command, u32 num_commands,
				const Float2U& min_position, const Float2U& max_position,
				const Float2U& min_shapecoord, const Float2U& max_shapecoord,
				u32 color = 0xFFFFFFFF,
				const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)
				) = 0;

			//! Finishes the shape recording and generates draw calls that can be passed to RHI.
			virtual RV close() = 0;

			virtual RHI::IBuffer* get_vertex_buffer() = 0;

			virtual u32 get_vertex_buffer_size() = 0;

			virtual RHI::IBuffer* get_index_buffer() = 0;

			virtual u32 get_index_buffer_size() = 0;

			virtual Vector<ShapeDrawCall> get_draw_calls() = 0;
		};

		LUNA_VG_API Ref<IShapeDrawList> new_shape_draw_list(RHI::IDevice* device = nullptr);
    }
}