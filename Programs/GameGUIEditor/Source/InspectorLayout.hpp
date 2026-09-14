/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InspectorLayout.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../Service/Authoring.hpp"
#include <Luna/GameGUI/BuiltInNodes.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            inline bool flex_stretches_child_size(const AuthoringNodeRecord& parent,
                const Name& size)
            {
                if(parent.type != GameGUI::get_flex_node_type()) return false;
                Name alignment = parent.properties["cross_alignment"].str();
                if(alignment == Name("start") || alignment == Name("center") ||
                    alignment == Name("end") || alignment == Name("space_between") ||
                    alignment == Name("space_around") || alignment == Name("space_evenly"))
                    return false;
                // Omitted or unrecognized values use the runtime's default Stretch alignment.
                bool horizontal = parent.properties["axis"].str() == Name("x");
                return (size == Name("width") && !horizontal) ||
                    (size == Name("height") && horizontal);
            }
        }
    }
}
