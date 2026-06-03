/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Views.hpp
* @author JXMaster
* @date 2026/6/2
*/
#pragma once
#include "State.hpp"
#include <Luna/Runtime/Math/Transform.hpp>

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Transform operation performed by @ref gizmo.
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

        //! Coordinate mode used by @ref gizmo.
        enum class GizmoMode : u32
        {
            //! Edit in local object coordinates.
            local = 0,
            //! Edit in world coordinates.
            world = 1
        };

        //! Adds a scalar float slider paired with a text input editor.
        LUNA_GUI_API ItemHandle slider_float_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a two-component float slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_float2_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a three-component float slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_float3_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a four-component float slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_float4_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a scalar integer slider paired with a text input editor.
        LUNA_GUI_API ItemHandle slider_int_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a two-component integer slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_int2_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a three-component integer slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_int3_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a four-component integer slider paired with text input editors.
        LUNA_GUI_API ItemHandle slider_int4_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a combo box view backed by an item index.
        LUNA_GUI_API ItemHandle combo(IContext* context, const c8* label, i32* current_item, Span<const c8*> items);
        //! Adds a three-channel color edit view for f32 values in the 0-1 range.
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, f32* value);
        //! Adds a four-channel color edit view for f32 values in the 0-1 range.
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, f32* value);
        //! Adds a three-channel color edit view for u8 values in the 0-255 range.
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u8* value);
        //! Adds a four-channel color edit view for u8 values in the 0-255 range.
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u8* value);
        //! Adds a three-channel color edit view for a packed RGBA8 value.
        //! @remark Alpha is forced to 255 by ColorEdit3.
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u32* value);
        //! Adds a four-channel color edit view for a packed RGBA8 value.
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u32* value);
        //! Adds an editor gizmo view over a scene viewport.
        //! @param[in] context The GUI context.
        //! @param[in] label The gizmo label used for ID generation.
        //! @param[in,out] world_matrix The object transform edited by the gizmo.
        //! @param[in] view The camera view matrix.
        //! @param[in] projection The camera projection matrix.
        //! @param[in] viewport_rect The scene viewport rectangle in screen coordinates.
        //! @param[in] operation The transform operation.
        //! @param[in] mode The transform coordinate mode.
        //! @param[in] snap Optional snap step. A value of 0 disables snapping.
        //! @param[in] enabled Whether the gizmo accepts input.
        //! @param[in] orthographic Whether the projection is orthographic.
        //! @param[out] delta_matrix Optional transform delta written when editing.
        //! @param[out] is_mouse_hover Optional flag set when the gizmo is hovered.
        //! @param[out] is_mouse_moving Optional flag set when the gizmo is actively moving.
        //! @param[out] edited Optional flag set when the transform changed this frame.
        //! @return Returns the gizmo item handle.
        LUNA_GUI_API ItemHandle gizmo(IContext* context, const c8* label, Float4x4& world_matrix, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect,
            GizmoOperation operation, GizmoMode mode, f32 snap = 0.0f, bool enabled = true, bool orthographic = false,
            Float4x4* delta_matrix = nullptr, bool* is_mouse_hover = nullptr, bool* is_mouse_moving = nullptr, bool* edited = nullptr);

        //! @}
    }
}
