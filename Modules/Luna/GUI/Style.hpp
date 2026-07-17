/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Style.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! The built-in editor style used when an element does not bind an explicit style.
        inline constexpr const c8* DEFAULT_STYLE_NAME = "gui.editor.default";

        //! Selects the color palette of the built-in editor style.
        enum class ColorTheme : u8
        {
            //! Uses the light editor palette.
            light,
            //! Uses the dark editor palette.
            dark
        };

        //! Selects the input-density metrics of the built-in editor style.
        enum class InputMode : u8
        {
            //! Uses compact controls intended for mouse and keyboard input.
            pointer,
            //! Uses larger controls and hit targets intended for touch input.
            touch
        };

        //! Describes one configured instance of the built-in editor style.
        struct DefaultStyleDesc
        {
            //! Color palette used by the style.
            ColorTheme color_theme = ColorTheme::light;
            //! Input-density metrics used by the style.
            InputMode input_mode = InputMode::touch;
            //! Accent color used to derive hover, pressed, subtle, disabled and focus colors.
            Float4U accent = Float4U(0.890f, 0.310f, 0.349f, 1.0f);
            //! Registered font used by controls and text.
            Name font = Name("default");
        };

        //! Registers style schemas consumed by the editor GUI package.
        //! @param[in] context The GUI Core context that stores schemas and styles.
        //! @remark This also initializes @ref DEFAULT_STYLE_NAME with @ref DefaultStyleDesc defaults.
        LUNA_GUI_API void register_style_schemas(GUICore::IContext* context);

        //! Configures one editor style from a color theme, input mode and accent color.
        //! @param[in] context The GUI Core context that stores the style.
        //! @param[in] style The style name to define or update.
        //! @param[in] desc The style configuration.
        LUNA_GUI_API void configure_style(GUICore::IContext* context, const Name& style,
            const DefaultStyleDesc& desc = DefaultStyleDesc());

        //! Reconfigures the built-in editor style used by elements without an explicit style binding.
        //! @param[in] context The GUI Core context that stores the style.
        //! @param[in] desc The style configuration.
        //! @remark Existing elements resolve the updated values during draw-command generation. Layout should be
        //! recalculated when @ref DefaultStyleDesc::input_mode changes.
        LUNA_GUI_API void set_default_style(GUICore::IContext* context,
            const DefaultStyleDesc& desc = DefaultStyleDesc());
    }
}
