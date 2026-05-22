/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Any.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/RHI/RHI.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        using GUIID = u64;

        enum class GUIInputEventType : u8
        {
            pointer_enter,
            pointer_leave,
            pointer_move,
            pointer_down,
            pointer_up,
            pointer_wheel,
            key_down,
            key_up,
            text_utf8,
            focus,
            blur
        };

        enum class GUIPointerButton : u8
        {
            left,
            right,
            middle,
            extra1,
            extra2
        };

        enum class GUIKey : u16
        {
            unknown,
            tab,
            left,
            right,
            up,
            down,
            enter,
            esc,
            backspace,
            del,
            space,
            a,
            b,
            c,
            d,
            e,
            f,
            g,
            h,
            i,
            j,
            k,
            l,
            m,
            n,
            o,
            p,
            q,
            r,
            s,
            t,
            u,
            v,
            w,
            x,
            y,
            z
        };

        enum class GUIKeyModifierFlag : u8
        {
            none = 0x00,
            ctrl = 0x01,
            shift = 0x02,
            alt = 0x04,
            system = 0x08
        };

        struct GUIFrameDesc
        {
            Float2U surface_size = Float2U(0.0f);
            UInt2U framebuffer_size = UInt2U(0, 0);
            f32 dpi_scale = 1.0f;
            f32 delta_time = 1.0f / 60.0f;
        };

        struct GUIInputEvent
        {
            GUIInputEventType type = GUIInputEventType::pointer_move;
            u64 device_id = 0;
            u64 pointer_id = 0;
            Float2U position = Float2U(0.0f);
            Float2U wheel_delta = Float2U(0.0f);
            GUIPointerButton button = GUIPointerButton::left;
            GUIKey key = GUIKey::unknown;
            GUIKeyModifierFlag modifiers = GUIKeyModifierFlag::none;
            String text;
        };

        struct GUIItemHandle
        {
            object_t context = nullptr;
            GUIID id = 0;
            u64 generation = 0;
        };

        template <typename _Ty>
        struct GUIStateKey
        {
            Name name;
            _Ty default_value;
        };
    }
}
