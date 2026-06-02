/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"
#include "InputNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ColorPickerNode : Node
        {
            lustruct("GUI::ColorPickerNode", "{BE28EC1C-7058-43E0-A970-FCFB1B2629FC}");

            ColorBinding binding;

            ColorPickerNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool uses_context_render() const override;
            virtual LayoutMetrics measure() const override;
        };
    }
}
