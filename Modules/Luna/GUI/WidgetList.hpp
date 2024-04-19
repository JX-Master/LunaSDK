/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetList.hpp
* @author JXMaster
* @date 2024/3/30
*/
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        //! Sets the widget ID for the current widget.
        //! Length: 2
        //! Data 0: The ID of the widget.
        // constexpr u32 CMD_ID = ascii_to_u32('I', 'D');
        // //! Begins one widget scope.
        // //! Length: 1
        // constexpr u32 CMD_BEGIN = ascii_to_u32('B', 'E', 'G', 'N');
        // //! Ends one widget scope.
        // //! Length: 1
        // constexpr u32 CMD_END = ascii_to_u32('E', 'N', 'D');
        // //! Adds one stack container widget to the list.
        // //! Length: 1
        // constexpr u32 CMD_STACK = ascii_to_u32('S', 'T', 'K');
        // //! Adds one text widget to the list.
        // //! Length: 1
        // //! Data 0: The index of the text.
        // constexpr u32 CMD_TEXT = ascii_to_u32('T', 'E', 'X', 'T');
        // //! Sets the position of the next item.
        // //! Length: 3
        // //! Data 0: x position.
        // //! Data 1: y position.
        // constexpr u32 CMD_SET_NEXT_POS_ALWAYS = ascii_to_u32('S', 'P', 'O', 'S');
        // //! Sets the position of the next item if it is added for the first time.
        // constexpr u32 CMD_SET_NEXT_POS_FIRST_TIME = ascii_to_u32('A', 'P', 'O', 'S');

        // Instruction format:
        //      Low                            High
        //      00000000 00000000 00000000 00000000
        // i0   |OpCode| 
        // iABC |OpCode| |  A   | |  B   | |  C   |
        // iABx |OpCode| |  A   | |      Bx       |
        // iAx  |OpCode| |           Ax           |

        enum class OpCode : u8
        {
            // Null operation.
            nop = 0,
            // Adds one layout widget used to arrange other widgets.
            // i0
            widget,
            // Adds one text widget used to display texts.
            // iAx
            // Ax: The index of the text to display.
            text,
            // Opens a new widget scope for child widgets.
            // i0
            begin,
            // Closes the most recently opened scope.
            // i0
            end,
            // Sets the widget anchor point.
            // iABC [D1 D2 D3 D4]
            // A: set condition (Condition).
            // B: set components (RectComponent).
            // D1 to D4 are f32 data specified by components (B) in left, top, right and bottom order.
            // Only components that are set in B will have corresponding data in D1 to D4, so the 
            // instruction length will vary from 4 to 20.
            // Default anthor: 0, 0, 1, 1.
            anchor,
            // Sets the widget rectangle.
            // iABC [D1 D2 D3 D4]
            // A: set condition (Condition).
            // B: set components (RectComponent).
            // D1 to D4 are f32 data specified by components (B) in left, top, right and bottom order.
            // Only components that are set in B will have corresponding data in D1 to D4, so the 
            // instruction length will vary from 4 to 20.
            // Default rect: 0, 0, 0, 0.
            rect,
            // Sets the widget color.
            // iABC D1
            // A: set condition (Condition).
            // B: the color to set (ColorType).
            // D1 is the color to set in RGBA8 form.
            color,
            // Sets the widget style.
            // iABC D1
            // A: set condition (Condition).
            // B: the style to set (StyleType).
            // D1 is the value to set in f32 form.
            style,
        };

        inline constexpr u32 make_command(OpCode op_code, u8 a, u8 b, u8 c)
        {
            return ((u8)op_code) | (((u32)a) << 8) | (((u32)b) << 16) | (((u32)c) << 24);
        }

        inline constexpr u32 make_command(OpCode op_code, u32 ax)
        {
            return ((u8)op_code) | (ax << 8);
        }

        inline u32 ftou32(f32 value)
        {
            return *reinterpret_cast<u32*>(&value);
        }

        inline f32 utof32(u32 value)
        {
            return *reinterpret_cast<f32*>(&value);
        }

        struct IWidgetList : virtual Interface
        {
            luiid("{c54b4dbd-4e59-452b-939a-07c820f79c05}");

            //! Clears all recorded widgets.
            virtual void reset() = 0;

            //! Gets the widget list buffer.
            virtual Vector<u32>& get_widget_buffer() = 0;

            virtual u32 add_text(const Name& text) = 0;

            virtual Name get_text(u32 index) = 0;

            //! Gets the widget ID for the current widget.
            //! The widget ID is calculated based on the current ID stack.
            //virtual u32 get_widget_id() = 0;

            //virtual void push_widget_id(u32 id) = 0;

            //virtual void pop_widget_id(u32 pop_count = 1) = 0;
        };

        LUNA_GUI_API Ref<IWidgetList> new_widget_list();
    }
}