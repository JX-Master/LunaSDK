/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Input.hpp
* @author JXMaster
* @date 2026/6/28
*/
#pragma once
#include "Base.hpp"
#include <Luna/Runtime/KeyCode.hpp>
#include <Luna/Runtime/String.hpp>

namespace Luna
{
    //! @addtogroup GUICore GUI Core
    //! @{
    namespace GUICore
    {
        //! Identifies one input event kind accepted by GUI Core contexts.
        enum class InputEventType : u8
        {
            //! The pointer entered the GUI screen or interactive surface.
            pointer_enter,
            //! The pointer left the GUI screen or interactive surface.
            pointer_leave,
            //! The pointer position changed.
            pointer_move,
            //! A pointer button was pressed.
            pointer_down,
            //! A pointer button was released.
            pointer_up,
            //! The pointer wheel or trackpad scrolling value changed.
            pointer_wheel,
            //! A keyboard key was pressed.
            key_down,
            //! A keyboard key was released.
            key_up,
            //! UTF-8 text input was committed by the host platform or input method.
            text_utf8,
            //! A directional navigation command was produced by the host input mapping.
            navigation_dpad,
            //! A forward or backward focus traversal command was produced by the host input mapping.
            navigation_move,
            //! A confirmation command was produced by the host input mapping.
            navigation_confirm,
            //! A back or cancel command was produced by the host input mapping.
            navigation_back,
            //! The GUI screen gained input focus.
            focus,
            //! The GUI screen lost input focus.
            blur
        };

        //! Identifies one pointer button.
        enum class PointerButton : u8
        {
            //! Primary pointer button.
            left,
            //! Secondary pointer button.
            right,
            //! Middle pointer button.
            middle,
            //! First additional pointer button.
            extra1,
            //! Second additional pointer button.
            extra2
        };

        //! Bit flags describing active keyboard modifiers.
        enum class KeyModifierFlag : u8
        {
            //! No modifier key is active.
            none = 0x00,
            //! Control key on Windows/Linux, or the platform-specific control key when applicable.
            ctrl = 0x01,
            //! Shift key.
            shift = 0x02,
            //! Alt or Option key.
            alt = 0x04,
            //! System command key, such as Command on macOS.
            system = 0x08
        };

        //! Identifies one directional navigation command.
        enum class NavigationDirection : u8
        {
            //! Move focus toward the left.
            left,
            //! Move focus toward the right.
            right,
            //! Move focus upward.
            up,
            //! Move focus downward.
            down
        };

        //! Identifies one sequential focus traversal command.
        enum class NavigationMove : u8
        {
            //! Move focus to the next focusable element.
            forward,
            //! Move focus to the previous focusable element.
            backward
        };

        //! Describes one input event sent to a GUI Core context.
        //! @remark Positions use screen logical coordinates whose origin is the top-left corner of @ref FrameDesc::screen_size.
        struct InputEvent
        {
            //! The kind of this input event.
            InputEventType type = InputEventType::pointer_move;
            //! The host-defined input device identifier preserved with this event.
            //! @remark Current GUI Core routing uses one shared pointer and keyboard state. It does not distinguish
            //! simultaneous devices.
            u64 device_id = 0;
            //! The host-defined pointer identifier preserved with this event.
            //! @remark Current GUI Core routing supports one pointer stream. This field does not yet create
            //! independent hover, capture or button state for multiple pointers.
            u64 pointer_id = 0;
            //! The pointer position in screen logical coordinates.
            Float2U position = Float2U(0.0f);
            //! Wheel or trackpad scrolling delta in logical units.
            Float2U wheel_delta = Float2U(0.0f);
            //! The pointer button affected by pointer button events.
            PointerButton button = PointerButton::left;
            //! The key affected by keyboard events.
            KeyCode key = KeyCode::unknown;
            //! Keyboard modifiers active when this event was produced.
            KeyModifierFlag modifiers = KeyModifierFlag::none;
            //! Direction payload for @ref InputEventType::navigation_dpad events.
            NavigationDirection navigation_direction = NavigationDirection::right;
            //! Move payload for @ref InputEventType::navigation_move events.
            NavigationMove navigation_move = NavigationMove::forward;
            //! UTF-8 text payload for @ref InputEventType::text_utf8 events.
            String text;
        };

        //! Reports the element that currently requests platform text input.
        struct TextInputState
        {
            //! Whether any element requests platform text input.
            bool active = false;
            //! The active text input rectangle in screen logical coordinates.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! The current UTF-8 byte cursor offset used by the active text input element.
            i32 cursor = 0;
        };

        //! Clipboard callbacks used by higher-level text editing controls.
        //! @remark GUI Core does not depend on Window. Window-backed and in-game hosts should provide platform
        //! clipboard operations through this structure when clipboard shortcuts are desired.
        struct ClipboardIO
        {
            //! User data passed back to clipboard callbacks.
            void* userdata = nullptr;
            //! Reads clipboard text into @p out_text.
            RV(*get_text)(String& out_text, void* userdata) = nullptr;
            //! Writes UTF-8 clipboard text.
            RV(*set_text)(const c8* text, usize size, void* userdata) = nullptr;
        };

        //! Describes one input event after it has been routed to a target element.
        //! @remark @ref event keeps the original screen logical coordinates. @ref element_position is an additional
        //! target-local coordinate produced by the input router for pointer events.
        struct RoutedInputEvent
        {
            //! The original input event.
            InputEvent event;
            //! Whether @ref element_position contains a meaningful pointer position.
            bool has_element_position = false;
            //! Pointer position relative to the target element layout rectangle's top-left corner.
            Float2U element_position = Float2U(0.0f);
        };
    }
    //! @}
}
