/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DragDrop.hpp
* @author JXMaster
* @date 2026/5/26
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        struct GUIDragDropPayload
        {
            Name type;
            const void* data = nullptr;
            usize data_size = 0;
            GUIItemHandle source;
            GUIItemHandle target;
            bool preview = false;
            bool delivery = false;

            template <typename _Ty>
            const _Ty* data_as() const
            {
                return data_size == sizeof(_Ty) ? (const _Ty*)data : nullptr;
            }
        };

        LUNA_GUI_API bool BeginDragDropSource(GUIItemHandle source, const Name& payload_type);
        LUNA_GUI_API void SetDragDropPayload(const void* data, usize data_size);
        LUNA_GUI_API void EndDragDropSource();

        LUNA_GUI_API bool BeginDragDropTarget(GUIItemHandle target, const Name& payload_type);
        LUNA_GUI_API const GUIDragDropPayload* AcceptDragDropPayload(const Name& payload_type);
        LUNA_GUI_API const GUIDragDropPayload* AcceptDragDropPayload(GUIItemHandle target, const Name& payload_type);
        LUNA_GUI_API void EndDragDropTarget();

        LUNA_GUI_API bool IsDragDropActive();
        LUNA_GUI_API const GUIDragDropPayload* GetDragDropPayload();
    }
}
