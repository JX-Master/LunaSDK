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
        Context* g_current_context = nullptr;

        namespace State
        {
            LUNA_GUI_API StateKey<bool> clicked() { return {Name("gui.clicked"), false}; }
            LUNA_GUI_API StateKey<bool> right_clicked() { return {Name("gui.right_clicked"), false}; }
            LUNA_GUI_API StateKey<bool> double_clicked() { return {Name("gui.double_clicked"), false}; }
            LUNA_GUI_API StateKey<bool> hovered() { return {Name("gui.hovered"), false}; }
            LUNA_GUI_API StateKey<bool> active() { return {Name("gui.active"), false}; }
            LUNA_GUI_API StateKey<bool> focused() { return {Name("gui.focused"), false}; }
            LUNA_GUI_API StateKey<bool> open() { return {Name("gui.open"), true}; }
            LUNA_GUI_API StateKey<bool> value_changed() { return {Name("gui.value_changed"), false}; }
            LUNA_GUI_API StateKey<RectF> rect() { return {Name("gui.rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
            LUNA_GUI_API StateKey<RectF> clip_rect() { return {Name("gui.clip_rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
        }

        LUNA_GUI_API Ref<IContext> new_context(RHI::IDevice* device)
        {
            Ref<Context> ctx = new_object<Context>();
            ctx->m_device = device ? device : RHI::get_main_device();
            ctx->m_shape_draw_list = VG::new_shape_draw_list(ctx->m_device);
            return Ref<IContext>(ctx);
        }

        LUNA_GUI_API void set_current_context(IContext* context)
        {
            g_current_context = context ? (Context*)context->get_object() : nullptr;
        }

        LUNA_GUI_API IContext* get_current_context()
        {
            return g_current_context;
        }

        Context* require_current_context()
        {
            luassert_msg(g_current_context, "No current GUI context. Call IContext::begin_frame first.");
            return g_current_context;
        }

        struct ModuleImpl : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                register_boxed_type<Context>();
                impl_interface_for_type<Context, IContext>();
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
            static ModuleImpl m;
            return &m;
        }
    }
}
