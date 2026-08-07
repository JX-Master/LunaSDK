/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Performance.hpp
* @author JXMaster
* @date 2026/7/12
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Runtime counters collected by one GUI Core context.
        struct PerformanceCounters
        {
            //! Context generation of the frame represented by these counters.
            u32 frame_generation = 0;
            //! Number of layers in the frame.
            u32 layer_count = 0;
            //! Number of elements in the frame.
            u32 element_count = 0;
            //! Number of input events queued for the frame.
            u32 input_event_count = 0;
            //! Number of input events delivered to routed elements.
            u32 delivered_input_event_count = 0;
            //! Number of elements that participate in hit testing.
            u32 interactable_count = 0;
            //! Number of commands in the latest generated draw command stream.
            u32 draw_command_count = 0;
            //! Number of draw callback invocations during the latest draw command generation pass.
            u32 draw_callback_count = 0;
            //! Number of states stored in the context after cleanup.
            u32 state_count = 0;
            //! Number of styles stored in the context.
            u32 style_count = 0;
            //! Number of style entry schemas registered in the context.
            u32 style_schema_count = 0;
            //! Time spent clearing expired state objects during the latest @ref IContext::begin_frame call, in milliseconds.
            f64 state_gc_ms = 0.0;
            //! Time spent routing input events during the latest @ref IContext::route_input call, in milliseconds.
            f64 input_route_ms = 0.0;
            //! Time spent generating the latest draw command stream through
            //! @ref IContext::generate_draw_commands, in milliseconds.
            f64 draw_generate_ms = 0.0;
        };
    }
}
