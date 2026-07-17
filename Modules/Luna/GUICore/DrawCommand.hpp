/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DrawCommand.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Element.hpp"
#include "SDF.hpp"
#include <Luna/VG/TextArranger.hpp>

namespace Luna
{
    namespace VG
    {
        struct IShapeDrawList;
        struct IShapeBuffer;
    }

    namespace GUICore
    {
        //! Identifies one GUI Core draw command kind.
        enum class DrawCommandType : u8
        {
            //! Draws a solid rectangle.
            rect,
            //! Draws a rectangle with per-corner colors.
            gradient_rect,
            //! Draws a solid rounded rectangle.
            rounded_rect,
            //! Draws a line segment.
            line,
            //! Draws UTF-8 text.
            text,
            //! Draws a textured rectangle.
            image,
            //! Draws a VG shape command range.
            shape,
            //! Draws an analytic rounded-rectangle shadow.
            shadow,
            //! Draws one analytic SDF shape and color program pair.
            sdf,
            //! Pushes a clip rectangle.
            push_clip,
            //! Pops the current clip rectangle.
            pop_clip
        };

        //! Identifies the coordinate space used by a draw command rectangle.
        enum class DrawCommandRectReference : u8
        {
            //! The command rectangle is already in layer coordinates.
            layer,
            //! The command rectangle is resolved relative to the owning element layout rectangle.
            element
        };

        //! Identifies one point in an element's painter-order traversal.
        enum class DrawPhase : u8
        {
            //! Runs before the element's statically recorded commands and child elements.
            before_children,
            //! Runs after the element's statically recorded commands and child elements.
            after_children
        };

        //! Selects the traversal phases that invoke an element draw callback.
        enum class DrawPhaseFlag : u8
        {
            //! Disables the draw callback.
            none = 0x00,
            //! Invokes the callback before child elements are generated.
            before_children = 0x01,
            //! Invokes the callback after child elements are generated.
            after_children = 0x02
        };

        //! Called while GUI Core generates draw commands for one element.
        //! @param[in] context The context that owns @p element. The callback may call @ref IContext::draw to emit
        //! commands for the element currently being generated.
        //! @param[in] element The element being generated.
        //! @param[in] phase The painter-order traversal phase being generated.
        //! @param[in] userdata User data stored in @ref DrawConfig.
        //! @return Returns success or failure code.
        //! @remark Draw callbacks run after layout and input routing. They must not mutate the element tree, layout,
        //! interaction state, or application data. The callback and userdata must remain valid until draw command
        //! generation finishes.
        using DrawCallback = RV(*)(IContext* context, const ElementHandle& element, DrawPhase phase, void* userdata);

        //! Describes delayed draw behavior attached to one typeless element.
        struct DrawConfig
        {
            //! Optional human-readable callback name used by diagnostics.
            Name name;
            //! Draw callback. A null callback disables delayed drawing.
            DrawCallback callback = nullptr;
            //! User data passed to @ref callback. The caller owns this memory.
            void* userdata = nullptr;
            //! Traversal phases that invoke @ref callback.
            DrawPhaseFlag phases = DrawPhaseFlag::before_children;
        };

        //! Describes one vector shape stored in a VG shape buffer.
        struct ShapeDesc
        {
            //! The VG shape buffer that stores the shape command range.
            VG::IShapeBuffer* buffer = nullptr;
            //! Optional texture sampled through the shape. When this is not `nullptr`, the shape acts as the texture mask.
            RHI::ITexture* texture = nullptr;
            //! The first command point of the shape in @ref buffer.
            u32 first_command = 0;
            //! The number of command points in the shape.
            u32 num_commands = 0;
            //! The source shape bounds used to map shape coordinates to the destination rectangle.
            //! @remark Bounds are interpreted in GUI shape coordinates, where Y increases downward.
            RectF bounds = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        };

        //! Identifies how a shadow is applied to its source rectangle.
        enum class ShadowMode : u8
        {
            //! Draws the shadow outside and underneath the source shape.
            outer,
            //! Draws the shadow inside the source shape.
            inner
        };

        //! Describes one analytic rounded-rectangle shadow.
        struct ShadowDesc
        {
            //! Offset from the source rectangle in screen logical coordinates.
            Float2U offset = Float2U(0.0f);
            //! Standard deviation of the analytic shadow falloff in screen logical coordinates.
            //! A value of zero produces an anti-aliased hard shadow.
            f32 softness = 0.0f;
            //! Signed expansion applied to the source contour before evaluating the shadow.
            f32 spread = 0.0f;
            //! Selects an outer or inner shadow.
            ShadowMode mode = ShadowMode::outer;
        };

        //! One GUI-level draw command.
        //! @remark Commands are intentionally primitive and widget-free. High-level packages decide which commands to emit.
        struct DrawCommand
        {
            //! Command kind.
            DrawCommandType type = DrawCommandType::rect;
            //! Owning layer index, or @ref INVALID_LAYER when the command is not layer-scoped.
            u32 layer = INVALID_LAYER;
            //! Owning element index, or @ref INVALID_ELEMENT when the command is not element-scoped.
            u32 element = INVALID_ELEMENT;
            //! Destination rectangle or clip rectangle in layer coordinates.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Coordinate reference used by @ref rect.
            DrawCommandRectReference rect_reference = DrawCommandRectReference::layer;
            //! Element-relative rectangle scale in left, top, right, bottom order.
            //! @remark When @ref rect_reference is @ref DrawCommandRectReference::element, the resolved rectangle is
            //! `element.rect + rect + element.rect.size * rect_layout_scale`. Negative or zero resolved sizes keep
            //! the trailing inset semantics, so `rect = {4, 4, -8, -8}` means the element rect inset by four pixels.
            Float4U rect_layout_scale = Float4U(0.0f);
            //! Secondary point used by line commands.
            Float2U point1 = Float2U(0.0f);
            //! Primary color.
            Float4U color = Float4U(1.0f);
            //! Top-right color for gradient rectangle commands.
            Float4U color_top_right = Float4U(1.0f);
            //! Bottom-right color for gradient rectangle commands.
            Float4U color_bottom_right = Float4U(1.0f);
            //! Bottom-left color for gradient rectangle commands.
            Float4U color_bottom_left = Float4U(1.0f);
            //! Corner radius for rounded rectangles.
            f32 radius = 0.0f;
            //! Line width for line commands.
            f32 line_width = 1.0f;
            //! Font ID used by text commands.
            Name font;
            //! Font size used by text commands.
            f32 font_size = 16.0f;
            //! Horizontal text alignment used by text commands.
            VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin;
            //! Vertical text alignment used by text commands.
            VG::TextAlignment vertical_alignment = VG::TextAlignment::center;
            //! Text payload used by text commands.
            String text;
            //! Texture used by image commands.
            RHI::ITexture* texture = nullptr;
            //! Minimum texture coordinate used by image and shape commands.
            Float2U min_texcoord = Float2U(0.0f, 0.0f);
            //! Maximum texture coordinate used by image and shape commands.
            Float2U max_texcoord = Float2U(1.0f, 1.0f);
            //! Whether image and shape commands should use nearest-neighbor texture sampling.
            bool nearest_sampler = false;
            //! Shape payload used by shape commands.
            ShapeDesc shape;
            //! Shadow payload used by shadow commands. @ref rect, @ref radius and @ref color describe the source
            //! rounded rectangle and shadow color.
            ShadowDesc shadow;
            //! SDF payload used by SDF commands. @ref rect places the payload's local coordinate origin in the
            //! element or layer coordinate space selected by @ref rect_reference. Its resolved extent is also the
            //! finite raster domain when the color program does not enable outer clipping.
            SDFDrawDesc sdf;
        };
    }
}
