/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Context.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Measures the content size required by a flex layout element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The flex container element.
        //! @param[in] available_content_size Available parent content-box size.
        //! @param[in] userdata Pointer to @ref FlexLayoutDesc owned by the caller.
        //! @return Returns content-box minimum, desired and maximum sizes for the flex container.
        //! @remark Install this function into @ref LayoutConfig::measure_callback together with
        //! @ref layout_flex when an element should hug its flex children on `fit` axes.
        LUNA_GUICORE_API MeasureResult measure_flex(IContext* context, const ElementHandle& element,
            const Float2U& available_content_size, void* userdata);

        //! Applies one flex layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] userdata Pointer to @ref FlexLayoutDesc owned by the caller.
        //! @return Returns success or failure code.
        //! @remark Flex layout first measures child subtrees to derive minimum, desired and maximum sizes, then
        //! distributes free or missing main-axis space with flex grow and shrink constraints before arranging
        //! children recursively through @ref IContext::apply_layout.
        LUNA_GUICORE_API RV layout_flex(IContext* context, const ElementHandle& element, const RectF& rect, void* userdata);

        //! Applies one row-major grid layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] userdata Pointer to @ref GridLayoutDesc owned by the caller.
        //! @return Returns success or failure code.
        //! @remark Grid layout is intended for tiled content such as asset browsers and icon palettes. It does not
        //! provide scrolling by itself; callers can combine it with scroll viewport primitives.
        LUNA_GUICORE_API RV layout_grid(IContext* context, const ElementHandle& element, const RectF& rect, void* userdata);

        //! Applies one anchor-based canvas layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] userdata Pointer to @ref CanvasLayoutDesc owned by the caller.
        //! @return Returns success or failure code.
        //! @remark Canvas layout is designed for freely positioned overlay content such as viewports, gizmo overlays,
        //! HUD elements and editor chrome. Child placement data belongs to the parent layout algorithm, not to
        //! the typeless child element itself.
        LUNA_GUICORE_API RV layout_canvas(IContext* context, const ElementHandle& element, const RectF& rect, void* userdata);

        //! Applies one scroll viewport layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The viewport element whose direct children will be arranged as scroll content.
        //! @param[in] rect The viewport rectangle in layer coordinates.
        //! @param[in] userdata Pointer to @ref ScrollViewportLayoutDesc owned by the caller.
        //! @return Returns success or failure code.
        //! @remark This primitive only translates and clips content. It does not implement scrollbar rendering,
        //! scroll input handling or offset clamping.
        LUNA_GUICORE_API RV layout_scroll_viewport(IContext* context, const ElementHandle& element, const RectF& rect,
            void* userdata);

        //! Applies one table track layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The table element whose direct children will be arranged by cell attachments.
        //! @param[in] rect The table rectangle in layer coordinates.
        //! @param[in] userdata Pointer to @ref TableLayoutDesc owned by the caller.
        //! @return Returns success or failure code.
        //! @remark Table layout uses explicit cell attachments instead of child order. This keeps the core primitive
        //! compatible with virtualized row submission and editor-authored table descriptions.
        LUNA_GUICORE_API RV layout_table(IContext* context, const ElementHandle& element, const RectF& rect, void* userdata);
    }
}
