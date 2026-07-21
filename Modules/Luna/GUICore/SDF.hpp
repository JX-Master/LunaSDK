/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SDF.hpp
* @author JXMaster
* @date 2026/7/17
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Maximum number of instructions accepted in one SDF shape program.
        constexpr u32 SDF_MAX_SHAPE_INSTRUCTIONS = 64;

        //! Maximum number of scalar floats accepted in one SDF shape program.
        constexpr u32 SDF_MAX_SHAPE_FLOATS = 512;

        //! Maximum evaluation stack depth accepted in one SDF shape program.
        constexpr u32 SDF_MAX_SHAPE_STACK_DEPTH = 16;

        //! Maximum number of color stops accepted in one SDF gradient program.
        constexpr u32 SDF_MAX_COLOR_STOPS = 16;

        //! Maximum number of color instructions accepted in one SDF color program.
        constexpr u32 SDF_MAX_COLOR_INSTRUCTIONS = 8;

        //! Maximum number of scalar floats accepted in one SDF color program.
        constexpr u32 SDF_MAX_COLOR_FLOATS = 1024;

        //! Number of scalar floats in one renderer SDF program page.
        //! @remark One page occupies 16 MiB and no individual program may cross a page boundary.
        constexpr u32 SDF_PROGRAM_PAGE_FLOATS = 4 * 1024 * 1024;

        //! Mask selecting the base color instruction from an encoded color opcode.
        constexpr u32 SDF_COLOR_OPCODE_MASK = 0xFF;

        //! Encoded color-opcode bit enabling the inner-distance clip limit.
        constexpr u32 SDF_COLOR_INNER_CLIP_BIT = 0x100;

        //! Encoded color-opcode bit enabling the outer-distance clip limit.
        constexpr u32 SDF_COLOR_OUTER_CLIP_BIT = 0x200;

        //! Mask selecting all clip bits from an encoded color opcode.
        constexpr u32 SDF_COLOR_CLIP_MASK = SDF_COLOR_INNER_CLIP_BIT | SDF_COLOR_OUTER_CLIP_BIT;

        //! Identifies one SDF shape program instruction.
        enum class SDFShapeInstruction : u32
        {
            //! Axis-aligned rectangle primitive.
            rectangle = 1,
            //! Axis-aligned rounded rectangle primitive with one radius per corner.
            rounded_rectangle = 2,
            //! Circle primitive.
            circle = 3,
            //! Axis-aligned ellipse primitive.
            ellipse = 4,
            //! Capsule primitive between two points.
            capsule = 5,
            //! Prefix union operation with two child expressions.
            union_op = 16,
            //! Prefix intersection operation with two child expressions.
            intersection_op = 17,
            //! Prefix difference operation with two child expressions.
            difference_op = 18,
            //! Prefix exclusive-or operation with two child expressions.
            xor_op = 19
        };

        //! Identifies one SDF color program instruction.
        enum class SDFColorInstruction : u32
        {
            //! Uniform color paint.
            solid = 1,
            //! Linear gradient paint.
            linear_gradient = 2,
            //! Elliptical radial gradient paint.
            radial_gradient = 3,
            //! Conic gradient paint.
            conic_gradient = 4,
            //! Four-corner bilinear gradient paint.
            bilinear_gradient = 5,
            //! Unified analytic shadow paint.
            shadow = 6
        };

        //! Selects how coordinates outside the color-stop interval are resolved.
        enum class SDFGradientSpread : u32
        {
            //! Clamps coordinates to the first or last color stop.
            pad = 0,
            //! Repeats the color-stop interval in both directions.
            repeat = 1
        };

        //! Selects the signed-distance limits applied before a color instruction is evaluated.
        enum class SDFClipMode : u32
        {
            //! Applies no signed-distance limit. The raster mesh remains the finite draw domain.
            none = 0,
            //! Limits how far the draw extends into the shape interior.
            inner = SDF_COLOR_INNER_CLIP_BIT,
            //! Limits how far the draw extends outside the shape.
            outer = SDF_COLOR_OUTER_CLIP_BIT,
            //! Limits both the interior and exterior distances.
            inner_outer = SDF_COLOR_CLIP_MASK
        };

        //! Describes signed-distance clipping attached to one SDF color instruction.
        struct SDFClipDesc
        {
            //! Enabled signed-distance limits.
            SDFClipMode mode = SDFClipMode::none;
            //! Maximum distance extending into the shape when inner clipping is enabled.
            f32 inner_distance = 0.0f;
            //! Maximum distance extending outside the shape when outer clipping is enabled.
            f32 outer_distance = 0.0f;

            //! Creates a clip descriptor without signed-distance limits.
            static constexpr SDFClipDesc no_clip()
            {
                return SDFClipDesc { SDFClipMode::none, 0.0f, 0.0f };
            }

            //! Creates an inner-only clip descriptor.
            //! @param[in] distance Maximum distance extending into the shape.
            static constexpr SDFClipDesc inner(f32 distance)
            {
                return SDFClipDesc { SDFClipMode::inner, distance, 0.0f };
            }

            //! Creates an outer-only clip descriptor.
            //! @param[in] distance Maximum distance extending outside the shape.
            static constexpr SDFClipDesc outer(f32 distance)
            {
                return SDFClipDesc { SDFClipMode::outer, 0.0f, distance };
            }

            //! Creates a clip descriptor with independent inner and outer limits.
            //! @param[in] inner_distance Maximum distance extending into the shape.
            //! @param[in] outer_distance Maximum distance extending outside the shape.
            static constexpr SDFClipDesc inner_outer(f32 inner_distance, f32 outer_distance)
            {
                return SDFClipDesc { SDFClipMode::inner_outer, inner_distance, outer_distance };
            }

            //! Creates the conventional shape-fill clip descriptor.
            static constexpr SDFClipDesc fill()
            {
                return outer(0.0f);
            }

            //! Creates a centered stroke clip descriptor.
            //! @param[in] width Total stroke width.
            static constexpr SDFClipDesc stroke(f32 width)
            {
                return inner_outer(width * 0.5f, width * 0.5f);
            }
        };

        //! Describes one contiguous range in an SDF scalar-float program buffer.
        struct SDFBufferRange
        {
            //! Index of the first scalar float.
            u32 first_float = 0;
            //! Number of scalar floats in the range.
            u32 num_floats = 0;

            //! Checks whether the range contains at least one float.
            //! @return Returns `true` if this range is non-empty.
            bool valid() const { return num_floats != 0; }
        };

        //! Describes one validated SDF shape program.
        struct SDFShapeProgram
        {
            //! Shape-buffer float range.
            SDFBufferRange floats;
            //! Conservative local-space bounds of the shape expression.
            RectF bounds = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Maximum number of temporary distances required by reverse prefix evaluation.
            u32 max_stack_depth = 0;
            //! Number of shape and operation instructions.
            u32 num_instructions = 0;
        };

        //! Describes one validated SDF color program containing one or more paint instructions.
        struct SDFColorProgram
        {
            //! Color-buffer float range.
            SDFBufferRange floats;
            //! Number of consecutive color instructions in the program.
            u32 num_instructions = 0;
            //! Conservative left, top, right and bottom outsets required around the shape bounds.
            Float4U effect_outsets = Float4U(0.0f);
            //! Whether at least one instruction uses shape-derived bounds.
            bool uses_shape_bounds = false;
            //! Whether at least one instruction uses the submitted raster domain as its draw bounds.
            bool uses_raster_domain = false;
            //! Whether at least one instruction may paint beyond the source shape boundary.
            bool may_paint_outside = false;
        };

        //! Describes one color stop in an SDF gradient program.
        struct SDFGradientStop
        {
            //! Stop position. Stops must be sorted in non-decreasing order.
            f32 position = 0.0f;
            //! Color at this stop in non-premultiplied sRGB space.
            Float4U color = Float4U(1.0f);
            //! Relative interpolation midpoint between this stop and the next stop.
            //! Values outside `(0, 1)` select the default midpoint of `0.5`.
            f32 midpoint = 0.5f;
        };

        //! Describes one SDF draw by referencing one reusable shape and one or more color instructions.
        struct SDFDrawDesc
        {
            //! Validated shape program stored by the current context.
            SDFShapeProgram shape;
            //! Validated color program stored by the current context.
            SDFColorProgram color;
        };

        //! Appends one prefix boolean operation to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] operation One of the four boolean operation instructions.
        //! @remark Append both child expressions immediately after this opcode. Shape programs use prefix order.
        LUNA_GUICORE_API void sdf_shape_add_operation(Vector<f32>& floats, SDFShapeInstruction operation);

        //! Appends one rectangle primitive to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] rect Primitive bounds in local logical coordinates.
        LUNA_GUICORE_API void sdf_shape_add_rectangle(Vector<f32>& floats, const RectF& rect);

        //! Appends one rounded rectangle primitive to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] rect Primitive bounds in local logical coordinates.
        //! @param[in] corner_radii Corner radii in top-left, top-right, bottom-right and bottom-left order.
        LUNA_GUICORE_API void sdf_shape_add_rounded_rectangle(Vector<f32>& floats, const RectF& rect,
            const Float4U& corner_radii);

        //! Appends one circle primitive to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] center Circle center in local logical coordinates.
        //! @param[in] radius Circle radius.
        LUNA_GUICORE_API void sdf_shape_add_circle(Vector<f32>& floats, const Float2U& center, f32 radius);

        //! Appends one ellipse primitive to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] center Ellipse center in local logical coordinates.
        //! @param[in] radii Horizontal and vertical radii.
        LUNA_GUICORE_API void sdf_shape_add_ellipse(Vector<f32>& floats, const Float2U& center,
            const Float2U& radii);

        //! Appends one capsule primitive to an SDF shape program.
        //! @param[in,out] floats Destination shape floats.
        //! @param[in] point0 First center-line endpoint.
        //! @param[in] point1 Second center-line endpoint.
        //! @param[in] radius Capsule radius.
        LUNA_GUICORE_API void sdf_shape_add_capsule(Vector<f32>& floats, const Float2U& point0,
            const Float2U& point1, f32 radius);

        //! Appends one complete solid color instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] color Non-premultiplied sRGB color.
        //! @param[in] clip Signed-distance clip limits encoded with the instruction.
        //! @return Returns the appended float range, or an empty range for an invalid clip descriptor.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_solid(Vector<f32>& floats, const Float4U& color,
            const SDFClipDesc& clip = SDFClipDesc::fill());

        //! Appends one complete linear gradient color instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] start Gradient line start in local logical coordinates.
        //! @param[in] end Gradient line end in local logical coordinates.
        //! @param[in] stops Sorted gradient stops.
        //! @param[in] spread Outside-interval spread behavior.
        //! @param[in] clip Signed-distance clip limits encoded with the instruction.
        //! @return Returns the appended float range, or an empty range for invalid parameters.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_linear_gradient(Vector<f32>& floats,
            const Float2U& start, const Float2U& end, Span<const SDFGradientStop> stops,
            SDFGradientSpread spread = SDFGradientSpread::pad,
            const SDFClipDesc& clip = SDFClipDesc::fill());

        //! Appends one complete radial gradient color instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] center Gradient center in local logical coordinates.
        //! @param[in] radii Horizontal and vertical radii of the terminal ellipse.
        //! @param[in] stops Sorted gradient stops.
        //! @param[in] spread Outside-interval spread behavior.
        //! @param[in] clip Signed-distance clip limits encoded with the instruction.
        //! @return Returns the appended float range, or an empty range for invalid parameters.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_radial_gradient(Vector<f32>& floats,
            const Float2U& center, const Float2U& radii, Span<const SDFGradientStop> stops,
            SDFGradientSpread spread = SDFGradientSpread::pad,
            const SDFClipDesc& clip = SDFClipDesc::fill());

        //! Appends one complete conic gradient color instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] center Gradient center in local logical coordinates.
        //! @param[in] start_angle Clockwise start angle in radians from the positive X axis.
        //! @param[in] stops Sorted gradient stops.
        //! @param[in] spread Outside-interval spread behavior.
        //! @param[in] clip Signed-distance clip limits encoded with the instruction.
        //! @return Returns the appended float range, or an empty range for invalid parameters.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_conic_gradient(Vector<f32>& floats,
            const Float2U& center, f32 start_angle, Span<const SDFGradientStop> stops,
            SDFGradientSpread spread = SDFGradientSpread::pad,
            const SDFClipDesc& clip = SDFClipDesc::fill());

        //! Appends one complete four-corner bilinear gradient color instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] paint_rect Local-space rectangle used to normalize gradient coordinates.
        //! @param[in] top_left Top-left color in non-premultiplied sRGB space.
        //! @param[in] top_right Top-right color in non-premultiplied sRGB space.
        //! @param[in] bottom_right Bottom-right color in non-premultiplied sRGB space.
        //! @param[in] bottom_left Bottom-left color in non-premultiplied sRGB space.
        //! @param[in] clip Signed-distance clip limits encoded with the instruction.
        //! @return Returns the appended float range, or an empty range for an invalid clip descriptor.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_bilinear_gradient(Vector<f32>& floats,
            const RectF& paint_rect, const Float4U& top_left, const Float4U& top_right,
            const Float4U& bottom_right, const Float4U& bottom_left,
            const SDFClipDesc& clip = SDFClipDesc::fill());

        //! Appends one complete unified shadow instruction.
        //! @param[in,out] floats Destination color floats.
        //! @param[in] color Shadow color in non-premultiplied sRGB space.
        //! @param[in] offset Shadow offset in local logical coordinates.
        //! @param[in] softness Standard deviation of the analytic shadow falloff.
        //! @param[in] spread Signed contour expansion before evaluating the shadow.
        //! @param[in] clip Signed-distance limits applied to the result. Inner-only zero draws an outer shadow;
        //! outer-only zero draws an inner shadow.
        //! @return Returns the appended float range, or an empty range for invalid parameters.
        LUNA_GUICORE_API SDFBufferRange sdf_color_add_shadow(Vector<f32>& floats,
            const Float4U& color, const Float2U& offset, f32 softness, f32 spread = 0.0f,
            const SDFClipDesc& clip = SDFClipDesc::inner(0.0f));

        //! Validates one standalone SDF shape program.
        //! @param[in] floats Complete prefix shape expression.
        //! @param[out] out_program Receives derived range, bounds, instruction count, and stack depth when non-null.
        //! @return Returns success or a format error.
        LUNA_GUICORE_API RV validate_sdf_shape_program(Span<const f32> floats,
            SDFShapeProgram* out_program = nullptr);

        //! Validates one standalone SDF color program.
        //! @param[in] floats One or more complete consecutive color instructions.
        //! @param[out] out_program Receives the aggregate program metadata when non-null.
        //! @return Returns success or a format error.
        //! @remark A multi-instruction program applies all paints to the same shape in append order and
        //! composites each newer result over the accumulated result using premultiplied source-over blending.
        //! Non-shadow instructions without an outer clip are rejected in multi-instruction programs because
        //! their raster domain would otherwise be widened by the other effects.
        LUNA_GUICORE_API RV validate_sdf_color_program(Span<const f32> floats,
            SDFColorProgram* out_program = nullptr);

        //! Evaluates one standalone SDF shape program on the CPU.
        //! @param[in] floats Complete prefix shape expression.
        //! @param[in] point Local-space sample point.
        //! @return Returns a signed distance. Negative values are inside the shape.
        LUNA_GUICORE_API R<f32> evaluate_sdf_shape(Span<const f32> floats, const Float2U& point);

        //! Evaluates the ideal hard clip mask encoded in one SDF color instruction.
        //! @param[in] floats Exactly one complete color instruction.
        //! @param[in] distance Signed distance of the sample point from the source shape.
        //! @return Returns `1` when the point passes the clip limits, or `0` otherwise.
        //! @remark The renderer adds derivative-based anti-aliasing around enabled clip boundaries.
        LUNA_GUICORE_API R<f32> evaluate_sdf_clip(Span<const f32> floats, f32 distance);

        //! Evaluates the base color of one standalone SDF color instruction on the CPU.
        //! @param[in] floats Exactly one complete color instruction.
        //! @param[in] point Local-space sample point.
        //! @return Returns a non-premultiplied sRGB color. For a shadow instruction this returns the shadow color;
        //! shape-dependent shadow coverage is evaluated by the renderer.
        LUNA_GUICORE_API R<Float4U> evaluate_sdf_color(Span<const f32> floats,
            const Float2U& point);
    }
}
