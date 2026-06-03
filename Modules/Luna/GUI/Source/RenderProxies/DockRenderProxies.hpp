/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include <Luna/GUI/Description.hpp>

namespace Luna
{
    namespace GUI
    {
        void draw_dock_panel_chrome(NodeRenderContext& ctx, const Node& node, const NodeRenderState& state);
        void draw_dock_space_splitters(NodeRenderContext& ctx, const Node& node, const NodeRenderState& state);
    }
}
