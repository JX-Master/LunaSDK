/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorViews.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include "Base.hpp"
#include "EditorState.hpp"
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/Runtime/Math/Transform.hpp>

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Transform operation performed by editor gizmo views.
        enum class GizmoOperation : u32
        {
            //! Translate the selected transform.
            translate = 0,
            //! Rotate the selected transform.
            rotate = 1,
            //! Scale the selected transform.
            scale = 2,
            //! Edit transform bounds.
            bounds = 3
        };

        //! Coordinate mode used by editor gizmo views.
        enum class GizmoMode : u32
        {
            //! Edit in local object coordinates.
            local = 0,
            //! Edit in world coordinates.
            world = 1
        };

        //! Adds a scalar float slider paired with a text input editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component float slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float2_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component float slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float3_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component float slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float4_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! Adds a scalar integer slider paired with a text input editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component integer slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int2_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component integer slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int3_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component integer slider paired with text input editors directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int4_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! Adds an editor-style combo box view directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle combo(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            i32* current_item, Span<const c8*> items, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! Adds a three-channel color edit view for f32 values directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, f32* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-channel color edit view for f32 values directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, f32* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-channel color edit view for u8 values directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, u8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-channel color edit view for u8 values directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, u8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-channel color edit view for RGBA8 values directly to a GUI Core context. Alpha is forced to 255.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, u32* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-channel color edit view for RGBA8 values directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, u32* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! @}
    }
}
