/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.hpp
* @author JXMaster
* @date 2024/3/30
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/VG/ShapeDrawList.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct ContextIO
        {
            //! The width of the viewport.
            u32 width;
            //! The height of the viewport.
            u32 height;
        };

        struct IWidgetList;

        struct IContext : virtual Interface
        {
            luiid("{8d1a5f1d-d7f2-46a5-82e7-2b382af47a9e}");
            
            //! Gets the IO state that will be parsed in the next @ref update call.
            virtual ContextIO& get_io() = 0;

            //! Replaces the state of the context with a new widget list.
            //! @param[in] widget_list The new widget list to set for the context.
            virtual RV reset(IWidgetList* widget_list) = 0;

            //! Updates the internal state (like input, animation, etc) of the context.
            virtual RV update() = 0;

            //! Checks whether we should redraw the context due to render state change.
            virtual bool is_dirty() = 0;

            //! Sets the context to require one redraw forcibly.
            virtual void set_dirty() = 0;

            //! Renders the context.
            virtual RV render(VG::IShapeDrawList* draw_list) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context();
    }
}