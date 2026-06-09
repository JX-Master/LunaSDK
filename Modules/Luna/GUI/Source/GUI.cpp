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
#include "GUI.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    namespace GUI
    {
        namespace State
        {
            LUNA_GUI_API StateKey<bool> clicked() { return {Name("gui.clicked"), false}; }
            LUNA_GUI_API StateKey<bool> right_clicked() { return {Name("gui.right_clicked"), false}; }
            LUNA_GUI_API StateKey<bool> double_clicked() { return {Name("gui.double_clicked"), false}; }
            LUNA_GUI_API StateKey<bool> hovered() { return {Name("gui.hovered"), false}; }
            LUNA_GUI_API StateKey<bool> active() { return {Name("gui.active"), false}; }
            LUNA_GUI_API StateKey<bool> focused() { return {Name("gui.focused"), false}; }
            LUNA_GUI_API StateKey<bool> enabled() { return {Name("gui.enabled"), true}; }
            LUNA_GUI_API StateKey<bool> open() { return {Name("gui.open"), true}; }
            LUNA_GUI_API StateKey<bool> value_changed() { return {Name("gui.value_changed"), false}; }
            LUNA_GUI_API StateKey<RectF> rect() { return {Name("gui.rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
            LUNA_GUI_API StateKey<RectF> clip_rect() { return {Name("gui.clip_rect"), RectF(0.0f, 0.0f, 0.0f, 0.0f)}; }
        }

        LUNA_GUI_API id_t make_state_id(id_t owner_id, const Guid& state_type)
        {
            u64 h = hash_u64(owner_id);
            h = hash_u64(state_type.high, h);
            h = hash_u64(state_type.low, h);
            return h ? h : 1;
        }

        LUNA_GUI_API Ref<IContext> new_context(RHI::IDevice* device)
        {
            Ref<Context> ctx = new_object<Context>();
            ctx->m_device = device ? device : RHI::get_main_device();
            ctx->m_shape_draw_list = VG::new_shape_draw_list(ctx->m_device);
            return Ref<IContext>(ctx);
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
                Meta::register_GUI_types();
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
