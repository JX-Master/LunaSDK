/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDragDrop.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static Ref<CoreDragDropBuildState> drag_drop_build_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreDragDropBuildState>(0);
            Ref<CoreDragDropBuildState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreDragDropBuildState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        LUNA_GUI_API void set_drag_drop_source_types(GUICore::IContext* context, const GUICore::ElementHandle& source,
            Span<const Name> types)
        {
            luassert(context);
            context->set_drag_drop_source_types(source, types);
        }

        LUNA_GUI_API void set_drag_drop_target_types(GUICore::IContext* context, const GUICore::ElementHandle& target,
            Span<const Name> types)
        {
            luassert(context);
            context->set_drag_drop_target_types(target, types);
        }

        LUNA_GUI_API bool begin_drag_drop_source(GUICore::IContext* context, const GUICore::ElementHandle& source,
            const Name& payload_type)
        {
            luassert(context);
            if(!source.id || payload_type.empty())
            {
                return false;
            }
            Name type = payload_type;
            context->set_drag_drop_source_types(source, Span<const Name>(&type, 1));

            const GUICore::DragDropPayload* active_payload = context->get_drag_drop_payload();
            bool active_from_source = active_payload && active_payload->source.id == source.id &&
                active_payload->type == payload_type;
            GUICore::InteractionState interaction = context->get_interaction_state(source.id);
            if(!interaction.active && !active_from_source)
            {
                return false;
            }

            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            state->source_scope_open = true;
            state->source = source;
            state->source_payload_type = payload_type;
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDragDropBuildState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
            return true;
        }

        LUNA_GUI_API void set_drag_drop_payload(GUICore::IContext* context, const void* data, usize data_size)
        {
            luassert(context);
            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            if(!state->source_scope_open || !state->source.id || state->source_payload_type.empty())
            {
                return;
            }
            lupanic_if_failed(context->start_drag_drop(state->source, state->source_payload_type, data, data_size));
        }

        LUNA_GUI_API void end_drag_drop_source(GUICore::IContext* context)
        {
            luassert(context);
            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            state->source_scope_open = false;
            state->source = GUICore::ElementHandle();
            state->source_payload_type.reset();
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDragDropBuildState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
        }

        LUNA_GUI_API bool begin_drag_drop_target(GUICore::IContext* context, const GUICore::ElementHandle& target,
            const Name& payload_type)
        {
            luassert(context);
            if(!target.id || payload_type.empty())
            {
                return false;
            }
            Name type = payload_type;
            context->set_drag_drop_target_types(target, Span<const Name>(&type, 1));

            const GUICore::DragDropPayload* active_payload = context->get_drag_drop_payload();
            if(!active_payload || active_payload->type != payload_type)
            {
                return false;
            }

            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            state->target_scope_open = true;
            state->target = target;
            state->target_payload_type = payload_type;
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDragDropBuildState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
            return true;
        }

        LUNA_GUI_API const GUICore::DragDropPayload* accept_drag_drop_payload(GUICore::IContext* context,
            const Name& payload_type)
        {
            luassert(context);
            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            if(!state->target_scope_open || !state->target.id || state->target_payload_type != payload_type)
            {
                return nullptr;
            }
            return context->get_drag_drop_delivery(state->target, payload_type);
        }

        LUNA_GUI_API void end_drag_drop_target(GUICore::IContext* context)
        {
            luassert(context);
            Ref<CoreDragDropBuildState> state = drag_drop_build_state(context);
            state->target_scope_open = false;
            state->target = GUICore::ElementHandle();
            state->target_payload_type.reset();
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDragDropBuildState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
        }

        LUNA_GUI_API RV start_drag_drop(GUICore::IContext* context, const GUICore::ElementHandle& source,
            const Name& payload_type, const void* data, usize data_size)
        {
            luassert(context);
            return context->start_drag_drop(source, payload_type, data, data_size);
        }

        LUNA_GUI_API void clear_drag_drop(GUICore::IContext* context)
        {
            luassert(context);
            context->clear_drag_drop();
        }

        LUNA_GUI_API bool is_drag_drop_active(GUICore::IContext* context)
        {
            luassert(context);
            return context->is_drag_drop_active();
        }

        LUNA_GUI_API const GUICore::DragDropPayload* get_drag_drop_payload(GUICore::IContext* context)
        {
            luassert(context);
            return context->get_drag_drop_payload();
        }

        LUNA_GUI_API GUICore::ElementHandle hit_test_drag_drop_target(GUICore::IContext* context,
            const Name& payload_type, const Float2U& screen_position)
        {
            luassert(context);
            return context->hit_test_drag_drop_target(payload_type, screen_position);
        }

        LUNA_GUI_API const GUICore::DragDropPayload* accept_drag_drop_payload(GUICore::IContext* context,
            const GUICore::ElementHandle& target, const Name& payload_type)
        {
            luassert(context);
            return context->get_drag_drop_delivery(target, payload_type);
        }
    }
}
