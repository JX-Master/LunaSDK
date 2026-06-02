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
        enum class GizmoOperation : u32
        {
            translate = 0,
            rotate = 1,
            scale = 2,
            bounds = 3
        };

        enum class GizmoMode : u32
        {
            local = 0,
            world = 1
        };

        LUNA_GUI_API ItemHandle slider_float_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float2_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float3_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float4_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_int_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int2_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int3_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int4_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle combo(IContext* context, const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, f32* value);
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, f32* value);
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u8* value);
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u8* value);
        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u32* value);
        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u32* value);
        LUNA_GUI_API ItemHandle gizmo(IContext* context, const c8* label, Float4x4& world_matrix, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect,
            GizmoOperation operation, GizmoMode mode, f32 snap = 0.0f, bool enabled = true, bool orthographic = false,
            Float4x4* delta_matrix = nullptr, bool* is_mouse_hover = nullptr, bool* is_mouse_moving = nullptr, bool* edited = nullptr);
    }
}
