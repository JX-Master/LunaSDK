/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LayoutUtils.hpp
* @author JXMaster
* @date 2024/7/22
*/
#pragma once
#include <Luna/Runtime/Math/Math.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset);
    }
}