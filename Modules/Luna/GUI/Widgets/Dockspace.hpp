/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Dockspace.hpp
* @author JXMaster
* @date 2024/8/6
*/
#pragma once
#include "Widget.hpp"
#include "Container.hpp"
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace GUI
    {
        enum class DockNodeType : u8
        {
            widget = 0,
            horizontal,
            vertical,
        };

        struct BinaryDockNode;

        struct DockNodeBase
        {
            DockNodeType type;
            //! Computed in @ref Dockspace::layout. Will not be saved.
            OffsetRectF layout_rect;
            //! Set when building widget tree, Will not be saved.
            BinaryDockNode* parent = nullptr;

            DockNodeBase(DockNodeType type) :
                type(type) {}
        };

        //! Cast to this if `type` is @ref DockNodeType::widget.
        struct WidgetDockNode : DockNodeBase
        {
            struct WidgetItem
            {
                widget_id_t id;
                f32 tab_rect_left;
                f32 tab_rect_right;
            };
            //! The IDs of the containing widget.
            Vector<WidgetItem> widgets;
            u32 current_tab = 0;

            //! Computed in @ref Dockspace::layout. Will not be saved.
            OffsetRectF title_rect;
            //! Computed in @ref Dockspace::layout. Will not be saved.
            OffsetRectF widget_rect;

            WidgetDockNode() :
                DockNodeBase(DockNodeType::widget) {}
        };

        //! Cast to this if `type` is @ref DockNodeType::horizontal or @ref DockNodeType::vertical.
        struct BinaryDockNode : DockNodeBase
        {
            UniquePtr<DockNodeBase> first_child;
            UniquePtr<DockNodeBase> second_child;
            f32 second_offset = 0.5;

            BinaryDockNode(DockNodeType type) :
                DockNodeBase(type) {}
        };
        
        struct DockspaceState
        {
            lustruct("GUI::DockspaceState", "93e54463-c65e-4ed5-915b-bd58175efd29");

            // The node that fills the dockspace.
            UniquePtr<DockNodeBase> root;

            // The mouse click state. Use for docking nodes.
            WidgetDockNode* clicking_node = nullptr;
            u32 clicking_widget_index;
            Float2U clicking_pos;
            OffsetRectF clicking_node_rect;

            // The mouse dragging state. Use for docking nodes.
            bool dragging = false;
            Float2U dragging_mouse_pos; // Used to track mouse position when dragging dock.
            WidgetDockNode* dragging_dock_target = nullptr;
            u32 dragging_dock_side; // 0 - left, 1 - right, 2 - top, 3 - bottom, 4 - center.
        };

        struct Dockspace : Widget, virtual IContainer
        {
            lustruct("GUI::Dockspace", "2888349e-97af-484b-8f6e-6eab16284053");

            Vector<Ref<IWidget>> children;
            DockspaceState* state = nullptr;

            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV handle_event(IContext* ctx, object_t e, bool& handled) override;
            LUNA_GUI_API virtual RV update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;

            LUNA_GUI_API virtual void add_child(IWidget* child) override;
            LUNA_GUI_API virtual void get_children(Vector<IWidget*>& out_children) override;
            LUNA_GUI_API virtual usize get_num_children() override;

        };
    }
}