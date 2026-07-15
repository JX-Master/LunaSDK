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
        //! Registers style schemas consumed by the editor GUI package.
        //! @param[in] context The GUI Core context that stores schemas and styles.
        LUNA_GUI_API void register_style_schemas(GUICore::IContext* context);
    }
}
