#pragma once
#include <Luna/GUI/GUI.hpp>
#include "DemoCustomNode.generated.hpp"

namespace Luna
{
    GUI::RenderProxyDesc demo_custom_node_render_proxy();

    struct [[Luna::struct("{A7A8030D-AAD4-4374-B967-74AF3DAD0A4D}")]] DemoCustomNode : GUI::Node
    {
        DemoCustomNode()
        {
            render_proxy = demo_custom_node_render_proxy();
        }

        virtual Guid type_guid() const override
        {
            return Meta::StructMetaData<DemoCustomNode>::__guid;
        }

        virtual Ref<GUI::Node> clone() const override
        {
            return new_object<DemoCustomNode>(*this);
        }

        virtual GUI::LayoutMetrics measure() const override
        {
            GUI::LayoutMetrics metrics;
            metrics.min_size = Float2U(160.0f, 34.0f);
            metrics.preferred_size = Float2U(260.0f, 38.0f);
            metrics.max_size = Float2U(F32_MAX, 38.0f);
            return metrics;
        }
    };
}
