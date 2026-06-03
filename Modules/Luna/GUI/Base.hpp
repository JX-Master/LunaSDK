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
        using id_t = u64;

        enum class InputEventType : u8
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

        enum class PointerButton : u8
        {
            left,
            right,
            middle,
            extra1,
            extra2
        };

        enum class Key : u16
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

        enum class KeyModifierFlag : u8
        {
            none = 0x00,
            ctrl = 0x01,
            shift = 0x02,
            alt = 0x04,
            system = 0x08
        };

        enum class TextAlignment : u8
        {
            begin,
            center,
            end
        };

        struct FrameDesc
        {
            Float2U surface_size = Float2U(0.0f);
            UInt2U framebuffer_size = UInt2U(0, 0);
            f32 dpi_scale = 1.0f;
            f32 delta_time = 1.0f / 60.0f;
        };

        struct InputEvent
        {
            InputEventType type = InputEventType::pointer_move;
            u64 device_id = 0;
            u64 pointer_id = 0;
            Float2U position = Float2U(0.0f);
            Float2U wheel_delta = Float2U(0.0f);
            PointerButton button = PointerButton::left;
            Key key = Key::unknown;
            KeyModifierFlag modifiers = KeyModifierFlag::none;
            String text;
        };

        struct TextInputState
        {
            bool active = false;
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            i32 cursor = 0;
        };

        struct ClipboardIO
        {
            void* userdata = nullptr;
            RV(*get_text)(String& out_text, void* userdata) = nullptr;
            RV(*set_text)(const c8* text, usize size, void* userdata) = nullptr;
        };

        struct ItemHandle
        {
            object_t context = nullptr;
            id_t id = 0;
            u64 generation = 0;
        };

        enum class StateLifetime : u8
        {
            current_frame,
            next_frame,
            process,
            persistent
        };

        LUNA_GUI_API id_t make_state_id(id_t owner_id, const Guid& state_type);

        template <typename T>
        id_t make_state_id(id_t owner_id)
        {
            return make_state_id(owner_id, T::__guid);
        }

        template <typename T>
        struct StateKey
        {
            Name name;
            T default_value;
        };
    }
}
