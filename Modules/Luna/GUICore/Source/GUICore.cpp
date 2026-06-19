/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUICore.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUICORE_API LUNA_EXPORT
#include "GUICore.hpp"
#include "GUICore.meta.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
        namespace
        {
            constexpr u64 fnv_offset = 14695981039346656037ull;
            constexpr u64 fnv_prime = 1099511628211ull;

            u64 hash_state_bytes(const void* data, usize size, u64 h = fnv_offset)
            {
                const byte_t* p = (const byte_t*)data;
                for(usize i = 0; i < size; ++i)
                {
                    h ^= (u64)p[i];
                    h *= fnv_prime;
                }
                return h;
            }

            u64 hash_state_u64(u64 value, u64 h = fnv_offset)
            {
                return hash_state_bytes(&value, sizeof(value), h);
            }
        }

        LUNA_GUICORE_API id_t make_state_id(id_t owner_id, const Guid& state_type)
        {
            u64 h = hash_state_u64(owner_id);
            h = hash_state_u64(state_type.high, h);
            h = hash_state_u64(state_type.low, h);
            return h ? h : 1;
        }

        struct GUICoreModule : Module
        {
            virtual const c8* get_name() override { return "GUICore"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                Meta::register_GUICore_types();
                return ok;
            }
        };
    }

    LUNA_GUICORE_API Ref<GUICore::IContext> GUICore::new_context()
    {
        return new_object<GUICore::Context>();
    }

    LUNA_GUICORE_API RV GUICore::replay_input_events(IContext* context, const DebugInfo& info)
    {
        if(!context)
        {
            return BasicError::bad_arguments();
        }
        context->add_input_events(Span<const InputEvent>(info.input_events.data(), info.input_events.size()));
        return ok;
    }

    LUNA_GUICORE_API void GUICore::push_debug_frame(DebugFrameTimeline& timeline, const DebugInfo& info, usize max_frames)
    {
        timeline.frames.push_back(info);
        if(max_frames && timeline.frames.size() > max_frames)
        {
            usize erase_count = timeline.frames.size() - max_frames;
            timeline.frames.erase(timeline.frames.begin(), timeline.frames.begin() + erase_count);
        }
        timeline.cursor = timeline.frames.empty() ? 0 : timeline.frames.size() - 1;
    }

    LUNA_GUICORE_API const GUICore::DebugInfo* GUICore::current_debug_frame(const DebugFrameTimeline& timeline)
    {
        if(timeline.frames.empty())
        {
            return nullptr;
        }
        usize cursor = min(timeline.cursor, timeline.frames.size() - 1);
        return &timeline.frames[cursor];
    }

    LUNA_GUICORE_API const GUICore::DebugInfo* GUICore::seek_debug_frame(DebugFrameTimeline& timeline, usize index)
    {
        if(timeline.frames.empty())
        {
            timeline.cursor = 0;
            return nullptr;
        }
        timeline.cursor = min(index, timeline.frames.size() - 1);
        return &timeline.frames[timeline.cursor];
    }

    LUNA_GUICORE_API const GUICore::DebugInfo* GUICore::step_debug_frame(DebugFrameTimeline& timeline, isize delta)
    {
        if(timeline.frames.empty())
        {
            timeline.cursor = 0;
            return nullptr;
        }
        if(delta < 0)
        {
            usize abs_delta = (usize)(-delta);
            timeline.cursor = abs_delta > timeline.cursor ? 0 : timeline.cursor - abs_delta;
        }
        else
        {
            timeline.cursor = min(timeline.cursor + (usize)delta, timeline.frames.size() - 1);
        }
        return &timeline.frames[timeline.cursor];
    }

    LUNA_GUICORE_API void GUICore::clear_debug_frames(DebugFrameTimeline& timeline)
    {
        timeline.frames.clear();
        timeline.cursor = 0;
    }

    LUNA_GUICORE_API Module* GUICore::module_gui_core()
    {
        static GUICore::GUICoreModule m;
        return &m;
    }
}
