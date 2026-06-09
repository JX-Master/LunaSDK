/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"
#include "InputNodes.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        struct [[Luna::struct("{14C55BCE-735A-4F3C-8B96-AE6743C5797B}")]] InputTextNode : Node
        {
            String* value = nullptr;

            InputTextNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{1832CB45-7F7E-483D-B665-25940619DF56}")]] SliderFloatNode : Node
        {
            NumericBinding binding;

            SliderFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{7BBCB122-B2A5-4C13-850D-590D21610C93}")]] SliderIntNode : Node
        {
            NumericBinding binding;

            SliderIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{BEE71E3F-890A-452F-9EB6-16FD9D605B29}")]] InputFloatNode : Node
        {
            NumericBinding binding;

            InputFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{0B504420-16B0-437A-9454-8D340C60275C}")]] InputIntNode : Node
        {
            NumericBinding binding;

            InputIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{1528F7FB-101A-4673-AFDA-24C1D011FA41}")]] DragFloatNode : Node
        {
            NumericBinding binding;

            DragFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{68CE96E0-8C8B-4DF5-A1AD-7DD6DC5E18DE}")]] DragIntNode : Node
        {
            NumericBinding binding;

            DragIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual LayoutMetrics measure() const override;
        };
    }
}
