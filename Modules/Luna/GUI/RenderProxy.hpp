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
        struct Node;
        struct NodeRenderContext;
        struct NodeRenderState;

        using RenderProxyDrawFunc = void(*)(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void* userdata);

        struct RenderProxyDesc
        {
            RenderProxyDrawFunc draw = nullptr;
            RenderProxyDrawFunc draw_after_children = nullptr;
            void* userdata = nullptr;

            bool valid() const
            {
                return draw || draw_after_children;
            }
        };
    }
}
