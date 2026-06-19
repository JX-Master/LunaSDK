/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DragDrop.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Element.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Describes one drag-drop payload view.
        struct DragDropPayload
        {
            //! Payload type name.
            Name type;
            //! Payload data pointer.
            const void* data = nullptr;
            //! Payload data size in bytes.
            usize data_size = 0;
            //! Source element handle.
            ElementHandle source;
            //! Target element handle. This is valid for delivery payloads.
            ElementHandle target;
            //! Whether this payload is a delivered payload.
            bool delivery = false;

            //! Interprets the payload data as a typed object.
            //! @return Returns a typed pointer if the payload size matches `sizeof(T)`, otherwise returns `nullptr`.
            template <typename T>
            const T* data_as() const
            {
                return data_size == sizeof(T) ? (const T*)data : nullptr;
            }
        };
    }
}
