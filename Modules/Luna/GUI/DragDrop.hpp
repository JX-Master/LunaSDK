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
        //! @addtogroup GUI GUI
        //! @{

        //! Describes the payload currently being dragged or accepted by a target.
        struct DragDropPayload
        {
            //! Payload type name. Source and target payload types must match explicitly.
            Name type;
            //! Pointer to payload data supplied by the source.
            const void* data = nullptr;
            //! Payload data size in bytes.
            usize data_size = 0;
            //! Source item handle.
            ItemHandle source;
            //! Target item handle that is previewing or accepting the payload.
            ItemHandle target;
            //! Whether this payload is being previewed by a compatible target.
            bool preview = false;
            //! Whether this payload has been delivered after the drag is released on a compatible target.
            bool delivery = false;

            //! Interprets the payload data as a typed object.
            //! @return Returns a typed pointer if the payload size matches `sizeof(T)`, otherwise returns `nullptr`.
            template <typename T>
            const T* data_as() const
            {
                return data_size == sizeof(T) ? (const T*)data : nullptr;
            }
        };

        //! Begins a drag-drop source scope for one item.
        //! @param[in] context The GUI context.
        //! @param[in] source The source item handle.
        //! @param[in] payload_type The payload type this source can provide.
        //! @return Returns `true` if the caller should build source preview content and set payload data.
        LUNA_GUI_API bool begin_drag_drop_source(IContext* context, ItemHandle source, const Name& payload_type);
        //! Sets payload data for the current drag-drop source scope.
        //! @param[in] context The GUI context.
        //! @param[in] data The payload data pointer.
        //! @param[in] data_size The payload data size in bytes.
        LUNA_GUI_API void set_drag_drop_payload(IContext* context, const void* data, usize data_size);
        //! Ends the current drag-drop source scope.
        //! @param[in] context The GUI context.
        LUNA_GUI_API void end_drag_drop_source(IContext* context);

        //! Begins a drag-drop target scope for one item.
        //! @param[in] context The GUI context.
        //! @param[in] target The target item handle.
        //! @param[in] payload_type The payload type this target accepts.
        //! @return Returns `true` if the target can preview or accept the current payload.
        LUNA_GUI_API bool begin_drag_drop_target(IContext* context, ItemHandle target, const Name& payload_type);
        //! Accepts the current drag-drop payload for the current target scope.
        //! @param[in] context The GUI context.
        //! @param[in] payload_type The payload type to accept.
        //! @return Returns payload information when the current payload type is compatible, otherwise returns `nullptr`.
        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, const Name& payload_type);
        //! Accepts the current drag-drop payload for a specific target.
        //! @param[in] context The GUI context.
        //! @param[in] target The target item handle.
        //! @param[in] payload_type The payload type to accept.
        //! @return Returns payload information when the current payload type and target are compatible, otherwise returns `nullptr`.
        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, ItemHandle target, const Name& payload_type);
        //! Ends the current drag-drop target scope.
        //! @param[in] context The GUI context.
        LUNA_GUI_API void end_drag_drop_target(IContext* context);

        //! Checks whether a drag-drop operation is active.
        //! @param[in] context The GUI context.
        //! @return Returns `true` if a payload is being dragged.
        LUNA_GUI_API bool is_drag_drop_active(IContext* context);
        //! Gets the current drag-drop payload.
        //! @param[in] context The GUI context.
        //! @return Returns the active payload, or `nullptr` if no drag-drop operation is active.
        LUNA_GUI_API const DragDropPayload* get_drag_drop_payload(IContext* context);

        //! @}
    }
}
