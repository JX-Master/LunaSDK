/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.hpp
* @author JXMaster
* @date 2024/3/29
*/
#include <Luna/Runtime/Name.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct IWidgetList;

        enum class Condition : u8
        {
            always = 0,
            add = 1,
            replace = 2,
        };

        enum class RectComponent : u8
        {
            none = 0,
            left = 1,
            top = 2,
            right = 4,
            bottom = 8,
            all = left | top | right | bottom
        };

        enum class ColorType : u8
        {
            background = 0,
            border = 1,
            text = 2, // text color.
        };

        enum class StyleType : u8
        {
            text_size, // text height.
        };

        LUNA_GUI_API void begin_widget(IWidgetList* list);

        LUNA_GUI_API void end(IWidgetList* list);

        LUNA_GUI_API void set_anthor(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom, Condition condition = Condition::always, RectComponent components = RectComponent::all);

        LUNA_GUI_API void set_rect(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom, Condition condition = Condition::always, RectComponent components = RectComponent::all);

        LUNA_GUI_API void set_color(IWidgetList* list, ColorType color_type, u32 color, Condition condition = Condition::always);

        //! Adds one text widget.
        //! @param[in] text The text to draw.
        LUNA_GUI_API void text(IWidgetList* list, const Name& text);

        //! Sets widget style value.
        LUNA_GUI_API void set_style(IWidgetList* list, StyleType type, f32 value, Condition condition = Condition::always);
    }
}