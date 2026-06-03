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
#include <Luna/Runtime/KeyCode.hpp>
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
        //! @addtogroup GUI GUI
        //! @{

        //! The stable identifier type used by GUI nodes, layers and state objects.
        using id_t = u64;

        //! Identifies one input event kind accepted by GUI contexts.
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

        //! Bit flags describing active keyboard modifiers for one input event or current input state.
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

        //! Aligns text inside the rectangle passed to text rendering helpers.
        enum class TextAlignment : u8
        {
            //! Align to the beginning edge of the axis.
            begin,
            //! Align to the center of the axis.
            center,
            //! Align to the ending edge of the axis.
            end
        };

        //! Describes one GUI frame and the screen being built and rendered.
        struct FrameDesc
        {
            //! The logical GUI screen size. Input positions and layout values are expressed in this coordinate space.
            Float2U surface_size = Float2U(0.0f);
            //! The render target size in physical pixels.
            UInt2U framebuffer_size = UInt2U(0, 0);
            //! The DPI scale applied by the host screen.
            f32 dpi_scale = 1.0f;
            //! The elapsed time since the previous frame, in seconds.
            f32 delta_time = 1.0f / 60.0f;
        };

        //! Describes one input event sent to a GUI context.
        //! @remark Positions use screen logical coordinates, whose origin is the top-left corner of @ref FrameDesc::surface_size.
        struct InputEvent
        {
            //! The kind of this input event.
            InputEventType type = InputEventType::pointer_move;
            //! The host-defined input device identifier.
            u64 device_id = 0;
            //! The host-defined pointer identifier, used for multi-pointer input sources.
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
            //! UTF-8 text payload for @ref InputEventType::text_utf8 events.
            String text;
        };

        //! Reports the text editing widget that currently owns platform text input.
        struct TextInputState
        {
            //! Whether any text input widget is active.
            bool active = false;
            //! The active text input rectangle in screen logical coordinates.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! The current UTF-8 byte cursor offset used by the active text input widget.
            i32 cursor = 0;
        };

        //! Clipboard callbacks used by text editing widgets.
        //! @remark The GUI module does not depend on Window. Window-backed and in-game hosts should provide platform
        //! clipboard operations through this structure when clipboard shortcuts are desired.
        struct ClipboardIO
        {
            //! User data passed back to clipboard callbacks.
            void* userdata = nullptr;
            //! Reads clipboard text into `out_text`.
            RV(*get_text)(String& out_text, void* userdata) = nullptr;
            //! Writes UTF-8 clipboard text.
            RV(*set_text)(const c8* text, usize size, void* userdata) = nullptr;
        };

        //! Handle returned by widget APIs for querying item state after building or submitting a frame.
        struct ItemHandle
        {
            //! The context that created this handle.
            object_t context = nullptr;
            //! The stable item identifier.
            id_t id = 0;
            //! The context generation in which this handle was produced.
            u64 generation = 0;
        };

        //! Controls automatic state cleanup in a GUI context.
        enum class StateLifetime : u8
        {
            //! Clears the state at the next @ref IContext::begin_frame call.
            current_frame,
            //! Clears the state if it is not refreshed for the next frame.
            next_frame,
            //! Keeps the state until @ref IContext::clear_state is called or the process exits.
            process,
            //! Reserves persistent storage semantics for future implementation.
            persistent
        };

        //! Builds a stable state identifier from an owner ID and a state object type GUID.
        //! @param[in] owner_id The widget, layer or subsystem ID that owns the state.
        //! @param[in] state_type The GUID of the state object type.
        //! @return Returns the generated state identifier.
        LUNA_GUI_API id_t make_state_id(id_t owner_id, const Guid& state_type);

        //! Builds a stable state identifier from an owner ID and a boxed state object type.
        //! @param[in] owner_id The widget, layer or subsystem ID that owns the state.
        //! @return Returns the generated state identifier.
        template <typename T>
        id_t make_state_id(id_t owner_id)
        {
            return make_state_id(owner_id, T::__guid);
        }

        //! Describes one typed item query key.
        template <typename T>
        struct StateKey
        {
            //! The key name used in the item query state bag.
            Name name;
            //! The value returned when the key is absent or the handle is no longer valid.
            T default_value;
        };

        //! @}
    }
}
