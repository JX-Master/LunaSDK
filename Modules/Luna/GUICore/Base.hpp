/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Hash.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/KeyCode.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Font/Font.hpp>

#ifndef LUNA_GUICORE_API
#define LUNA_GUICORE_API
#endif

namespace Luna
{
    //! @addtogroup GUICore GUI Core
    //! @{
    namespace GUICore
    {
        //! Stable identifier type used by GUI Core elements, layers and state objects.
        using id_t = u64;

        //! The invalid element index value.
        constexpr u32 INVALID_ELEMENT = U32_MAX;

        //! The invalid layer index value.
        constexpr u32 INVALID_LAYER = U32_MAX;

        //! Default data scope used when no explicit scope has been pushed.
        constexpr id_t DEFAULT_DATA_SCOPE = 14695981039346656037ull;

        //! Mixes a numeric local ID into a data scope.
        //! @param[in] scope The parent data scope. Passing zero uses @ref DEFAULT_DATA_SCOPE.
        //! @param[in] local_id The local ID inside the scope.
        //! @return Returns the stable ID for the scoped item.
        inline id_t make_scoped_id(id_t scope, id_t local_id)
        {
            id_t seed = scope ? scope : DEFAULT_DATA_SCOPE;
            id_t ret = memhash64(&local_id, sizeof(local_id), seed);
            return ret ? ret : DEFAULT_DATA_SCOPE;
        }

        //! Mixes a string local ID into a data scope.
        //! @param[in] scope The parent data scope. Passing zero uses @ref DEFAULT_DATA_SCOPE.
        //! @param[in] local_name The local string ID inside the scope.
        //! @return Returns the stable ID for the scoped item.
        inline id_t make_scoped_id(id_t scope, const c8* local_name)
        {
            id_t seed = scope ? scope : DEFAULT_DATA_SCOPE;
            id_t ret = strhash<id_t>(local_name ? local_name : "", seed);
            return ret ? ret : DEFAULT_DATA_SCOPE;
        }

        //! Describes one GUI Core frame.
        struct FrameDesc
        {
            //! The logical screen size used by layout and input positions.
            Float2U screen_size = Float2U(0.0f);
            //! The render target size in physical pixels.
            UInt2U framebuffer_size = UInt2U(0, 0);
            //! The host DPI scale for this frame.
            f32 dpi_scale = 1.0f;
            //! The elapsed time since the previous frame, in seconds.
            f32 delta_time = 1.0f / 60.0f;
        };

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

        //! Describes one input event sent to a GUI Core context.
        //! @remark Positions use screen logical coordinates whose origin is the top-left corner of @ref FrameDesc::screen_size.
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
            //! Reads clipboard text into `out_text`.
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

        //! Describes one font registered in a GUI Core context.
        struct FontDesc
        {
            //! The font file object. The context keeps a reference to registered font files.
            Font::IFontFile* font = nullptr;
            //! The font face index inside @ref font.
            u32 font_index = 0;
        };

        //! Controls automatic state cleanup in a GUI Core context.
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

        //! Hash functor for GUI Core stable IDs.
        struct IdHash
        {
            //! Hashes one GUI Core ID.
            //! @param[in] value The value to hash.
            //! @return Returns the hash value.
            usize operator()(id_t value) const
            {
                return hash<u64>()(value);
            }
        };

        //! Builds a stable state identifier from an owner ID and a state object type GUID.
        //! @param[in] owner_id The element, layer or subsystem ID that owns the state.
        //! @param[in] state_type The GUID of the state object type.
        //! @return Returns the generated state identifier.
        LUNA_GUICORE_API id_t make_state_id(id_t owner_id, const Guid& state_type);

        //! Builds a stable state identifier from an owner ID and a boxed state object type.
        //! @param[in] owner_id The element, layer or subsystem ID that owns the state.
        //! @return Returns the generated state identifier.
        template <typename T>
        id_t make_state_id(id_t owner_id)
        {
            return make_state_id(owner_id, Meta::StructMetaData<T>::__guid);
        }
    }
    //! @}
}
