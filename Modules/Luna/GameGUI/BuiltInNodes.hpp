/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file BuiltInNodes.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "Node.hpp"

namespace Luna
{
    namespace GameGUI
    {
        //! Gets the built-in Flex node type ID.
        LUNA_GAME_GUI_API Guid get_flex_node_type();
        //! Gets the built-in Canvas node type ID.
        LUNA_GAME_GUI_API Guid get_canvas_node_type();
        //! Gets the built-in Panel node type ID.
        LUNA_GAME_GUI_API Guid get_panel_node_type();
        //! Gets the built-in Text node type ID.
        LUNA_GAME_GUI_API Guid get_text_node_type();
        //! Gets the built-in Button node type ID.
        LUNA_GAME_GUI_API Guid get_button_node_type();
        //! Gets the built-in AssetInstance node type ID.
        LUNA_GAME_GUI_API Guid get_asset_instance_node_type();
        //! Registers all GameGUI built-in node types.
        LUNA_GAME_GUI_API RV register_builtin_node_types();
    }
}
