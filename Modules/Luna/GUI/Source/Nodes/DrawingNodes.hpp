/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"
#include <Luna/VG/ShapeBuffer.hpp>
#include "DrawingNodes.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        struct [[Luna::struct("{B1BC2CF2-66A6-4BED-90C8-0DD87F9B5AA9}")]] HitBoxNode : Node
        {
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{FBF64EAB-E6FE-4C06-BAE3-DB51389D8BC4}")]] ImageNode : Node
        {
            Ref<RHI::ITexture> image;
            ImageFlag flags = ImageFlag::none;

            ImageNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{33EDF8A1-B3C9-405F-B819-89677F77A299}")]] ShapeNode : Node
        {
            Ref<VG::IShapeBuffer> buffer;
            Ref<RHI::ITexture> texture;
            u32 first_command = 0;
            u32 num_commands = 0;
            RectF bounds;

            ShapeNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{BD3F1D72-125E-47D3-8EF3-DC2D3B252586}")]] DrawRectNode : Node
        {
            Float4U color = Float4U(1.0f);
            f32 radius = 0.0f;

            DrawRectNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{84F5D227-F548-4BC0-9940-690D49F255C7}")]] DrawCircleNode : Node
        {
            Float4U color = Float4U(1.0f);

            DrawCircleNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{B3BD0E9B-64C2-4953-87BC-17338FAAE22A}")]] DrawLineNode : Node
        {
            Float2U begin = Float2U(0.0f);
            Float2U end = Float2U(0.0f);
            Float4U color = Float4U(1.0f);
            f32 width = 1.0f;

            DrawLineNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{904753AE-7E39-42BC-B249-F03393F9255B}")]] DrawTextNode : Node
        {
            Float4U color = Float4U(1.0f);
            f32 font_size = 16.0f;
            TextAlignment horizontal_alignment = TextAlignment::begin;
            TextAlignment vertical_alignment = TextAlignment::center;

            DrawTextNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
        struct [[Luna::struct("{BD105DA6-0ABF-4A86-BA0E-313EFBEB5233}")]] DrawImageNode : Node
        {
            Ref<RHI::ITexture> image;
            Float4U color = Float4U(1.0f);
            ImageFlag flags = ImageFlag::none;

            DrawImageNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;
        };
    }
}
