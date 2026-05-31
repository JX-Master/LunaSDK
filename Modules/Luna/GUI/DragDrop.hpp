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
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        struct DragDropPayload
        {
            Name type;
            const void* data = nullptr;
            usize data_size = 0;
            ItemHandle source;
            ItemHandle target;
            bool preview = false;
            bool delivery = false;

            template <typename T>
            const T* data_as() const
            {
                return data_size == sizeof(T) ? (const T*)data : nullptr;
            }
        };

        LUNA_GUI_API bool begin_drag_drop_source(IContext* context, ItemHandle source, const Name& payload_type);
        LUNA_GUI_API void set_drag_drop_payload(IContext* context, const void* data, usize data_size);
        LUNA_GUI_API void end_drag_drop_source(IContext* context);

        LUNA_GUI_API bool begin_drag_drop_target(IContext* context, ItemHandle target, const Name& payload_type);
        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, const Name& payload_type);
        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, ItemHandle target, const Name& payload_type);
        LUNA_GUI_API void end_drag_drop_target(IContext* context);

        LUNA_GUI_API bool is_drag_drop_active(IContext* context);
        LUNA_GUI_API const DragDropPayload* get_drag_drop_payload(IContext* context);
    }
}
