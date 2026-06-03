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
                register_abstract_struct_type<Node>({});
                register_struct_type<RootNode>({}, typeof<Node>());
                register_struct_type<HLayoutNode>({}, typeof<Node>());
                register_struct_type<VLayoutNode>({}, typeof<Node>());
                register_struct_type<ScrollViewNode>({}, typeof<Node>());
                register_struct_type<WindowNode>({}, typeof<Node>());
                register_struct_type<PopupNode>({}, typeof<Node>());
                register_struct_type<TooltipNode>({}, typeof<Node>());
                register_struct_type<MenuBarNode>({}, typeof<Node>());
                register_struct_type<TableLayoutNode>({}, typeof<Node>());
                register_struct_type<GridLayoutNode>({}, typeof<Node>());
                register_struct_type<CanvasLayoutNode>({}, typeof<Node>());
                register_struct_type<DockSpaceNode>({}, typeof<Node>());
                register_struct_type<TabBarNode>({}, typeof<Node>());
                register_struct_type<ButtonNode>({}, typeof<Node>());
                register_struct_type<TextNode>({}, typeof<Node>());
                register_struct_type<SelectableNode>({}, typeof<Node>());
                register_struct_type<CheckboxNode>({}, typeof<Node>());
                register_struct_type<ToggleSwitchNode>({}, typeof<Node>());
                register_struct_type<CollapsingHeaderNode>({}, typeof<Node>());
                register_struct_type<TreeNodeNode>({}, typeof<Node>());
                register_struct_type<RadioButtonNode>({}, typeof<Node>());
                register_struct_type<TabItemNode>({}, typeof<Node>());
                register_struct_type<InputTextNode>({}, typeof<Node>());
                register_struct_type<SliderFloatNode>({}, typeof<Node>());
                register_struct_type<SliderIntNode>({}, typeof<Node>());
                register_struct_type<InputFloatNode>({}, typeof<Node>());
                register_struct_type<InputIntNode>({}, typeof<Node>());
                register_struct_type<DragFloatNode>({}, typeof<Node>());
                register_struct_type<DragIntNode>({}, typeof<Node>());
                register_struct_type<ColorPickerNode>({}, typeof<Node>());
                register_struct_type<MenuItemNode>({}, typeof<Node>());
                register_struct_type<MenuSeparatorNode>({}, typeof<Node>());
                register_struct_type<ButtonGroupNode>({}, typeof<Node>());
                register_struct_type<HitBoxNode>({}, typeof<Node>());
                register_struct_type<ImageNode>({}, typeof<Node>());
                register_struct_type<DrawRectNode>({}, typeof<Node>());
                register_struct_type<DrawCircleNode>({}, typeof<Node>());
                register_struct_type<DrawLineNode>({}, typeof<Node>());
                register_struct_type<DrawTextNode>({}, typeof<Node>());
                register_struct_type<DrawImageNode>({}, typeof<Node>());
                register_struct_type<ItemQueryState>({});
                register_struct_type<CustomState>({});
                register_struct_type<DisclosureState>({});
                register_struct_type<InteractionState>({});
                register_struct_type<ScrollState>({});
                register_struct_type<NumericInteractionState>({});
                register_struct_type<ColorPickerInteractionState>({});
                register_struct_type<TableResizeInteractionState>({});
                register_struct_type<ScrollbarInteractionState>({});
                register_struct_type<DockInteractionState>({});
                register_struct_type<TabInteractionState>({});
                register_struct_type<TooltipInteractionState>({});
                register_struct_type<BuildHintState>({});
                register_struct_type<SwitchAnimationState>({});
                register_struct_type<ButtonAnimationState>({});
                register_struct_type<ButtonGroupAnimationState>({});
                register_struct_type<TabBarAnimationState>({});
                register_struct_type<DockSpaceState>({});
                register_struct_type<InputEditState>({});
                register_struct_type<TableLayoutState>({});
                register_struct_type<TabBarState>({});
                register_struct_type<TabBuildState>({});
                register_struct_type<ColorPickerState>({});
                register_struct_type<PopupAnchorState>({});
#ifdef LUNA_GUI_ENABLE_DEBUG
                register_struct_type<DebugInspectorState>({});
#endif
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
