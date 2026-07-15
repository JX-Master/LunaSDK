/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Editor.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include "Base.hpp"
#include "EditorState.hpp"
#include "EditorWidgets.hpp"
#include "EditorViews.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! Editor-style immediate API package that builds directly into GUICore.
        //! @remark This header is the preferred entry point for new code that targets the ADR-0004 GUI Core
        //! architecture. It exposes editor-style widgets and views without including the legacy GUI runtime
        //! context, description tree or render proxy headers.
        //! @{
        //! @}
    }
}
