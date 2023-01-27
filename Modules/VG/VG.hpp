/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VG.hpp
* @author JXMaster
* @date 2022/4/17
* @brief Vector Graphics (VG) module is used to render GPU-based 2D vector graphics. Such graphics are usually used to present
* in-game GUIs.
*/
#pragma once
#include <Runtime/Interface.hpp>
#include <Runtime/Math/Matrix.hpp>
#include <Runtime/Vector.hpp>
#include <RHI/RHI.hpp>
#include <Font/Font.hpp>
#include <Runtime/Math/Color.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
	namespace VG
	{
		//! Begins one new path.
		//! 0: The x coordinates of the initial position.
		//! 1: The y coordinates of the initial position.
		//! The former path will be closed when one begin command is detected.
		constexpr f32 COMMAND_MOVE_TO = 1.0f;
		//! Draws one line to the specified point.
		//! Data points:
		//! 0: The x coordinates of the target position.
		//! 1: The y coordinates of the target position.
		constexpr f32 COMMAND_LINE_TO = 2.0f;
		//! Draws a quadratic Belzier curve to the specified point.
		//! Data points:
		//! 0: The x coordinates of the curve control point.
		//! 1: The y coordinates of the curve control point.
		//! 2: The x coordinates of the target position.
		//! 3: The y coordinates of the target position.
		constexpr f32 COMMAND_CURVE_TO = 3.0f;

		/*
			Circle drawing commands.
					 90
					  y
					  ^
					  |
				Q2	  |		Q1
					  |
		180	-------------------->x 0
					  |
				Q3	  |		Q4
					  |
					  |
					 270

			All circle drawing commands take three data points:
			0: The radius of the circle.
			1: The beginning angle of the circle in degrees.
			2: The end angle of the circle in degrees.
			If the end angle is greater than the beginning angle, the circle is drawn counter-clockwisly,
		    otherwise, the circle is drawn clockwisly.
		*/

		//! Draws a circle part in the first quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [0, 90].
		//! 2: The end angle of the circle in degrees. The value should be in [0, 90].
		constexpr f32 COMMAND_CIRCLE_Q1 = 4.0f;
		//! Draws a circle part in the second quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [90, 180].
		//! 2: The end angle of the circle in degrees. The value should be in [90, 180].
		constexpr f32 COMMAND_CIRCLE_Q2 = 5.0f;
		//! Draws a circle part in the third quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [180, 270].
		//! 2: The end angle of the circle in degrees. The value should be in [180, 270].
		constexpr f32 COMMAND_CIRCLE_Q3 = 6.0f;
		//! Draws a circle part in the fourth quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [270, 360].
		//! 2: The end angle of the circle in degrees. The value should be in [270, 360].
		constexpr f32 COMMAND_CIRCLE_Q4 = 7.0f;

		//! One utility structure for building shapes from commands.
		//! A shape is a vector graphic composed by contours (closed paths). Shapes are defined in 
		//! shape coordinates, and can be scaled when rendering.
		struct ShapeBuilder
		{
			Vector<f32> points;

			void move_to(f32 x, f32 y) { points.insert(points.end(), { COMMAND_MOVE_TO, x, y }); }
			void line_to(f32 x, f32 y) { points.insert(points.end(), { COMMAND_LINE_TO, x, y }); }
			void curve_to(f32 cx, f32 cy, f32 x, f32 y) { points.insert(points.end(), { COMMAND_CURVE_TO, cx, cy, x, y }); }
			void circle_to(f32 radius, f32 begin, f32 end)
			{
				// round to positive.
				if (begin < 0 || end < 0)
				{
					f32 m = min(begin, end);
					i32 round = i32(-m / 360.0f) + 1;
					m = 360.0f * (f32)round;
					begin += m;
					end += m;
				}
				f32 cur = begin;
				if (end > begin)
				{
					while (cur < end)
					{
						i32 quad_count = i32(cur / 90.0f);
						i32 quad = quad_count % 4;
						f32 next = min(end, (quad_count + 1) * 90.0f);
						points.insert(points.end(), { COMMAND_CIRCLE_Q1 + (f32)quad, radius, cur, next });
						cur = next;
					}
				}
				else if(end < begin)
				{
					while (cur > end)
					{
						i32 quad_count = i32(cur / 90.0f);
						if (quad_count * 90.0f == cur) --quad_count;
						i32 quad = quad_count % 4;
						f32 next = max(end, quad_count * 90.0f);
						points.insert(points.end(), { COMMAND_CIRCLE_Q1 + (f32)quad, radius, cur, next });
						cur = next;
					}
				}
			}
		};

		struct ShapeDesc
		{
			usize command_offset;
			usize num_commands;
			RectF bounding_rect;
		};

		//! Represents a collection of multiple shape data.
		//! Shape atlas is similar to texture atlas used in traditional glyph rendering, but represents every shape (or glyph)
		//! as commands rather than pre-rendered pixels, so that they can be scaled without any blurring and aliasing.
		struct IShapeAtlas : virtual Interface
		{
			luiid("{1EB34768-6775-458D-ADAF-07CD2D2F7918}");

			//! Clears data in the shape atlas.
			virtual void clear() = 0;

			//! Gets the shape command buffer data.
			virtual const f32* get_command_buffer_data() = 0;

			//! Gets the shape command buffer size.
			virtual usize get_command_buffer_size() = 0;

			//! Appends new shape at the end of the shape buffer.
			//! @param[in] commands A pointer to the command data of the shape. The first command must be `COMMAND_MOVE_TO`.
			//! @param[in] num_commands The number of commands (f32 values) of the shape.
			//! @param[in] bounding_rect The bounding rect of the shape. If this is `nullptr`, the bounding rect will be calculated 
			//! from shape commands.
			//! @return Returns the index of the shape.
			virtual usize add_shape(const f32* commands, usize num_commands, const RectF* bounding_rect) = 0;

			//! Adds multiple shapes in one call.
			//! @param[in] commands A pointer to the command data of the shapes.
			//! @param[in|out] shapes A pointer to an array of shape descriptors you want to add.
			//! @param[in] num_shapes The number of shape descriptors you want to add.
			//! @return Returns the shape index of the first added shape.
			//! @remark The shape descriptors are used for both input and output. When the shape descriptor is provided, `command_offset` and `num_commands` 
			//! identify the shape command range in `commands`, and `bounding_rect` provides the bounding rect for the shape. If `bounding_rect` is 
			//! `RectF(0, 0, 0, 0)`, the system computes the bounding rect of the shape. Shape commands cannot overlap or padded, `shapes[i].command_offset + shapes[i].num_commands` 
			//! must be equal to `shapes[i + 1].command_offset`.
			//! When the function returns, every shape descriptor provided is modified, so that `command_offset` and `num_commands` represents the shape command range
			//! in the internal command buffer, and `bounding_rect` is the computed shape bounding rect.
			virtual usize add_shapes(const f32* commands, ShapeDesc* shapes, usize num_shapes) = 0;

			//! Copies shapes from another shape atlas to this shape atlas. The new shapes will be added to the end of the shape buffer.
			//! @param[in] src The shape atlas to copy shapes from.
			//! @param[in] start_shape_index The index of the first shape to copy.
			//! @param[in] num_shapes Number of shapes to copy.
			virtual usize copy_shapes(IShapeAtlas* src, usize start_shape_index, usize num_shapes) = 0;

			//! Remove shapes from the shape atlas.
			//! @param[in] start_shape_index The index of the first shape to remove.
			//! @param[in] num_shapes Number of shapes to remove.
			//! @remark Removing shapes causes all succeeding shapes to subtract `num_shapes` from their indices, their command offsets will also be modified.
			virtual void remove_shapes(usize start_shape_index, usize num_shapes) = 0;

			//! Gets the number of shapes in the command buffer.
			virtual usize count_shapes() = 0;

			//! Queries the information about the specified shape.
			//! @param[in] index The index of the shape. The shape index must be in [0, count_shapes()).
			//! @param[out] data_offset If not `nullptr`, returns the offset of the first command of the shape in the command buffer.
			//! @param[out] data_size If not `nullptr`, returns the number of commands of the shape data.
			//! @param[out] bounding_rect If not `nullptr`, returns the bounding rect of the shape.
			virtual void get_shape(usize index, usize* data_offset, usize* data_size, RectF* bounding_rect) = 0;

			//! Gets the shape buffer resource. This call flushes shape commands so that they will be uploaded to GPU before this call returns.
			virtual R<RHI::IResource*> get_shape_resource() = 0;

			//! Gets the shape buffer resource size in bytes. Returns 0 if failed to create shape resource.
			virtual usize get_shape_resource_size() = 0;
		};

		//! Creates a new empty shape atlas.
		LUNA_VG_API Ref<IShapeAtlas> new_shape_atlas();

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
			//! The shape atlas bind to this draw call.
			IShapeAtlas* atlas;
			//! The texture bind to this draw call. May be `nullptr`.
			RHI::IResource* texture;
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
			//! The clip rect for this draw call.
			RectI clip_rect;
		};

		//! Represents a draw list that contains shapes to be drawn.
		struct IShapeDrawList : virtual Interface
		{
			luiid("{14F1CA71-7B2D-4072-A2EE-DFD64B62FCD5}");

			//! Resets the draw list. The call clears all shapes recorded, but retains their memory
			//! and resources, so they can be reused for new shapes.
			virtual void reset() = 0;

			//! Makes the following drawing commands not to be appended to the draw calls that are created before this method
			//! is called.
			//! 
			//! @remark An draw list manages a series of draw calls. Each of the draw calls is bound to a specific pipeline state 
			//! set (like texture, sampler, transform and so on) and will be sent to the GPU as one real draw call. All elements 
			//! that shares the same draw pipeline state will be batched into one draw call, when a new shape is added to the draw list, 
			//! the draw list finds the draw call that matches the current pipeline setting and appends this shape's primitives into the 
			//! draw call. If no draw call is found, the draw list creates a new draw call and pushes the new shape into the new draw call. 
			//! 
			//! This sometimes can be a problem because after batching, one shape that is drawn lately may be batched into one draw call that 
			//! is created early, thus gets drawn early by GPU. If two shapes are batched into the same draw call (because they have exactly the same
			//! drawing settings), this will not be a problem, because the hardware guarantees the later added shapes are always drawn later. But if 
			//! two shapes are not batched into the same draw call (because they have different drawing settings), the drawing order of the shapes depends on 
			//! the drawing order of their draw calls, which is sometimes unpredictable.
			//! 
			//! When `drawcall_barrier` is called, the draw list prevents the later added shapes to be batched to previously created draw calls,
			//! so that they will be guaranteed to be drawn after all draw calls before `drawcall_barrier` get drawn. Using this, the draw list assumes that all 
			//! commands drawn later have chances to occlude previously drawn elements, so there is no chance of batching them with previously draw calls 
			//! (even if they do have).
			//! 
			//! `drawcall_barrier` will be called by draw list internally when another draw list is appended to this draw list by calling `append_draw_list`.
			virtual void drawcall_barrier() = 0;

			//! Sets the shape atlas used for the following draw calls.
			virtual void set_shape_atlas(IShapeAtlas* atlas) = 0;

			//! Gets the current shape atlas.
			virtual IShapeAtlas* get_shape_atlas() = 0;

			//! Sets the texture to be sampled when rendering the succeeding shapes.
			//! @param[in] tex The texture to set. Specify `nullptr` is allowed, which behaves the same as applying one white texture with all components set to
			//! 1.0f.
			//! @remark The draw list only stores the texture and its state as-is and provides it to the renderer 
			//! when the draw list is processed by the renderer. It does not do any validation to the texture and 
			//! its states. It is the user and renderer's responsibility to validate the texture and its state.
			//! 
			//! The draw list has texture being set to `nullptr` after reset.
			virtual void set_texture(RHI::IResource* tex) = 0;

			//! Gets the currently set texture, returns `nullptr` if no texture is set.
			virtual RHI::IResource* get_texture() = 0;

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

			//! Sets the clip rect for the following calls, any draw call that goes out of clip region will be clipped.
			//! @param[in] clip_rect The clip rect to set. The rect position is relative to the point set by `set_origin`.
			//! Default clip rect is (0,0,0,0) and it should mean no clip for the renderer.
			//! 
			//! The clip rect will not be rotated by `set_rotation`.
			virtual void set_clip_rect(const RectI& clip_rect = RectI(0, 0, 0, 0)) = 0;

			//! Gets the clip rect for the following calls.
			virtual RectI get_clip_rect() = 0;

			//! Appends another draw list to this draw list.
			virtual void append_draw_list(IShapeDrawList* draw_list) = 0;

			//! Draws one shape by submitting vertices and indices directly.
			virtual void draw_shape_raw(const Vertex* vertices, u32 num_vertices, const u32* indices, u32 num_indices) = 0;

			//! Draws one shape. The shape is drawn by adding one draw rect (two triangles) to the list.
			virtual void draw_shape(u32 begin_command, u32 num_commands,
				const Float2U& min_position, const Float2U& max_position,
				const Float2U& min_shapecoord, const Float2U& max_shapecoord,
				u32 color = 0xFFFFFFFF,
				const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)
				) = 0;

			//! Finishes the shape recording and generates draw calls that can be passed to RHI.
			virtual RV close() = 0;

			virtual RHI::IResource* get_vertex_buffer() = 0;

			virtual u32 get_vertex_buffer_size() = 0;

			virtual RHI::IResource* get_index_buffer() = 0;

			virtual u32 get_index_buffer_size() = 0;

			virtual Vector<ShapeDrawCall> get_draw_calls() = 0;
		};

		LUNA_VG_API Ref<IShapeDrawList> new_shape_draw_list();

		struct IFontAtlas : virtual Interface
		{
			luiid("{FCDB9053-448B-4E7D-BC94-B67A7E81081A}");

			virtual void clear() = 0;

			//! Gets the shape atlas attached.
			virtual IShapeAtlas* get_shape_atlas() = 0;

			//! Gets the font bound to this font atlas.
			virtual Font::IFontFile* get_font(u32* index) = 0;

			//! Sets the font bound to this font atlas. The will reset the font atlas.
			virtual void set_font(Font::IFontFile* font, u32 index) = 0;

			virtual usize get_glyph_shape_index(u32 codepoint) = 0;

			virtual void get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing) = 0;
		
			virtual f32 scale_for_pixel_height(f32 pixels) = 0;

			virtual void get_vmetrics(i32* ascent, i32* descent, i32* line_gap) = 0;
		
			virtual i32 get_kern_advance(u32 ch1, u32 ch2) = 0;
		};

		LUNA_VG_API Ref<IFontAtlas> new_font_atlas(Font::IFontFile* font, u32 index);

		enum class TextAlignment : u8
		{
			begin = 1,
			center = 2,
			end = 3,
		};

		struct TextGlyphArrangeResult
		{
			RectF bounding_rect;

			//! The orgin point offset of this glyph relative to the beginning of the current
			//! line.
			f32 origin_offset;

			//! The advance length of the glyph.
			//! This is not equal to `bounding_rect.width` (or `bounding_rect.height` in vertical 
			//! line), because some characters may take more spaces than necessary for paddings.
			f32 advance_length;

			//! The UTF-32 character codepoint of the glyph.
			c32 character;

			//! The index of this glyph in the text buffer.
			u32 index;
		};

		struct TextLineArrangeResult
		{
			//! The bounding rect of the line.
			RectF bounding_rect;

			//! The offset of the baseline of this line. The offset is relative to the starting edge of the
			//! text's bounding box.
			f32 baseline_offset;

			//! The ascent value (units from baseline to the top of the character) of this line.
			f32 ascent;

			//! The decent value (units from baseline to the bottom of the character, typically negative) of this line.
			f32 decent;

			//! The line gap of this line. The final line gap is determined by the greater line_gap
			//! value of two adjacent lines.
			f32 line_gap;

			Vector<TextGlyphArrangeResult> glyphs;
		};

		struct TextArrangeResult
		{
			//! The real bounding rect occupied by the text. This may be smaller than 
			//! the bounding rect specified.
			RectF bounding_rect;

			//! True if the bounding rect is too small to hold all text specified.
			bool overflow;

			Vector<TextLineArrangeResult> lines;
		};

		struct ITextArranger : virtual Interface
		{
			luiid("{EB049D67-134C-4F84-A912-99A8AC406847}");

			virtual void reset() = 0;

			virtual void clear_text_buffer() = 0;

			virtual IFontAtlas* get_font() = 0;

			virtual void set_font(IFontAtlas* font) = 0;

			virtual color_u32 get_font_color() = 0;

			virtual void set_font_color(color_u32 color) = 0;

			virtual f32 get_font_size() = 0;

			virtual void set_font_size(f32 size) = 0;

			virtual f32 get_char_span() = 0;

			//! Sets the spen between the last character and the next character. The character span value takes effect 
			//! until it is changed again.
			virtual void set_char_span(f32 span) = 0;

			virtual f32 get_line_span() = 0;

			//! Sets the line span between the current line and the next line. The line span value takes effect until it is 
			//! changed again.
			virtual void set_line_span(f32 span) = 0;

			virtual void add_text(const c8* text) = 0;

			virtual void add_text_region(const c8* text, usize text_len) = 0;

			virtual TextArrangeResult arrange(const RectF& bounding_rect,
				TextAlignment line_alignment, TextAlignment glyph_alignment) = 0;

			virtual void commit(const TextArrangeResult& result, IShapeDrawList* draw_list) = 0;
		};

		LUNA_VG_API Ref<ITextArranger> new_text_arranger(IFontAtlas* initial_font);

		//! Holds resources that used to render shape draw calls.
		struct IShapeRenderer : virtual Interface
		{
			luiid("{C0FBD0AE-B7F6-4A82-A59B-B1115ACCBD94}");

			virtual void reset() = 0;

			virtual RV set_render_target(RHI::IResource* render_target) = 0;

			virtual RV render(
				RHI::ICommandBuffer* cmdbuf,
				RHI::IResource* shape_buffer,
				u32 num_points,
				RHI::IResource* vertex_buffer,
				u32 num_vertices,
				RHI::IResource* index_buffer,
				u32 num_indices,
				const ShapeDrawCall* draw_calls,
				u32 num_draw_calls
			) = 0;
		};

		LUNA_VG_API R<Ref<IShapeRenderer>> new_fill_shape_renderer(RHI::IResource* render_target);
	}
}