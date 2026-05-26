/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"
#include "GUIDrawList.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    namespace GUI
    {
        GUIContext* g_current_context = nullptr;

        namespace GUIState
        {
            LUNA_GUI_API GUIStateKey<bool> clicked() { return {Name("gui.clicked"), false}; }
            LUNA_GUI_API GUIStateKey<bool> right_clicked() { return {Name("gui.right_clicked"), false}; }
            LUNA_GUI_API GUIStateKey<bool> double_clicked() { return {Name("gui.double_clicked"), false}; }
            LUNA_GUI_API GUIStateKey<bool> hovered() { return {Name("gui.hovered"), false}; }
            LUNA_GUI_API GUIStateKey<bool> active() { return {Name("gui.active"), false}; }
            LUNA_GUI_API GUIStateKey<bool> focused() { return {Name("gui.focused"), false}; }
            LUNA_GUI_API GUIStateKey<bool> open() { return {Name("gui.open"), true}; }
            LUNA_GUI_API GUIStateKey<bool> value_changed() { return {Name("gui.value_changed"), false}; }
            LUNA_GUI_API GUIStateKey<RectF> rect() { return {Name("gui.rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
            LUNA_GUI_API GUIStateKey<RectF> clip_rect() { return {Name("gui.clip_rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
        }

        LUNA_GUI_API Ref<IGUIContext> new_context(RHI::IDevice* device)
        {
            Ref<GUIContext> ctx = new_object<GUIContext>();
            ctx->m_device = device ? device : RHI::get_main_device();
            ctx->m_shape_draw_list = VG::new_shape_draw_list(ctx->m_device);
            return Ref<IGUIContext>(ctx);
        }

        LUNA_GUI_API void set_current_context(IGUIContext* context)
        {
            g_current_context = context ? (GUIContext*)context->get_object() : nullptr;
        }

        LUNA_GUI_API IGUIContext* get_current_context()
        {
            return g_current_context;
        }

        GUIContext* require_current_context()
        {
            luassert_msg(g_current_context, "No current GUI context. Call IGUIContext::begin_frame first.");
            return g_current_context;
        }

        struct GUIModule : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                register_boxed_type<GUIContext>();
                impl_interface_for_type<GUIContext, IGUIContext>();
                register_boxed_type<DrawList>();
                impl_interface_for_type<DrawList, IDrawList>();
                return ok;
            }
            virtual void on_close() override {}
        };
    }

    namespace GUI
    {
        LUNA_GUI_API Module* module_gui()
        {
            static GUIModule m;
            return &m;
        }
    }
}
