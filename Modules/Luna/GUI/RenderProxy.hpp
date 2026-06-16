/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RenderProxy.hpp
* @author JXMaster
* @date 2026/6/3
*/
#pragma once
#include "Style.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        struct Node;
        struct NodeRenderContext;
        struct NodeRenderState;

        //! Draw callback used by render proxies.
        //! @param[in] ctx The render context for issuing draw commands and accessing state.
        //! @param[in] node The node being rendered.
        //! @param[in] rect The node rectangle in layer coordinates.
        //! @param[in] clip_rect The clip rectangle in layer coordinates.
        //! @param[in] state The dynamic render state for this node.
        //! @param[in] userdata User data stored in @ref RenderProxyDesc.
        using RenderProxyDrawFunc = void(*)(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void* userdata);

        //! Describes one style entry that may be read by a render proxy.
        //! @remark Style usage is owned by the render proxy rather than by the node type, because replacing the render proxy can
        //! change which style entries affect a node.
        struct StyleEntryDesc
        {
            //! Style entry name.
            Name name;
            //! Expected value type.
            StyleValueType type = StyleValueType::f32_4;
            //! Fallback value used by the render proxy when the entry cannot be resolved from the node style.
            StyleValue default_value;
            //! Optional human-readable display name.
            const c8* display_name = nullptr;
            //! Optional UI category for editors and debug tools.
            const c8* category = nullptr;
            //! Optional description for editors and debug tools.
            const c8* description = nullptr;
        };

        //! Describes how one node should be rendered.
        //! @remark A render proxy does not own state or lifetime. It may read node properties, node style entries and
        //! context state objects to implement custom visuals and visual animations.
        struct RenderProxyDesc
        {
            //! Callback invoked before rendering child nodes.
            RenderProxyDrawFunc draw = nullptr;
            //! Callback invoked after rendering child nodes.
            RenderProxyDrawFunc draw_after_children = nullptr;
            //! User data passed to render proxy callbacks.
            void* userdata = nullptr;
            //! Style entries used by this render proxy.
            const StyleEntryDesc* style_entries = nullptr;
            //! Number of entries in @ref style_entries.
            usize num_style_entries = 0;

            //! Checks whether this render proxy contains at least one callback.
            //! @return Returns `true` if this proxy can render something.
            bool valid() const
            {
                return draw || draw_after_children;
            }
        };

        //! @}
    }
}
