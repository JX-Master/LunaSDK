/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUICoreTest.hpp
* @author JXMaster
* @date 2026/7/1
*/
#pragma once
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/VG/TextArranger.hpp>

namespace Luna::GUICoreTest
{
    inline constexpr f32 SHEET_WIDTH = 1580.0f;
    inline constexpr f32 SHEET_HEIGHT = 1080.0f;

    enum : GUICore::id_t
    {
        ID_SCREEN_ROOT = 1,
        ID_SHEET = 2,
        ID_HEADER = 10,
        ID_FRAME = 20,
        ID_ELEMENT_TREE = 30,
        ID_LAYOUT = 40,
        ID_INPUT = 50,
        ID_DRAW = 60,
        ID_STATE = 70,
        ID_DEBUG = 80,
        ID_CANVAS = 90,
        ID_HIT_RECT = 100,
        ID_HIT_CIRCLE = 101,
        ID_HIT_PASS = 102
    };

    struct CoreSheetState
    {
        Float2U sheet_position = Float2U(36.0f, 36.0f);
        GUICore::CanvasLayoutItem screen_item;
        GUICore::CanvasLayoutDesc screen_canvas;
        Vector<GUICore::CanvasLayoutItem> sheet_items;
        GUICore::CanvasLayoutDesc sheet_canvas;
    };

    GUICore::LayoutConfig fixed_layout(f32 width, f32 height);
    void set_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
        GUICore::CanvasLayoutDesc* desc);
    void add_canvas_item(Vector<GUICore::CanvasLayoutItem>& items, GUICore::id_t id, f32 x, f32 y);

    void draw_rect(GUICore::IContext* context, const RectF& rect, const Float4U& color, f32 radius = 0.0f);
    void draw_gradient_rect(GUICore::IContext* context, const RectF& rect, const Float4U& top_left,
        const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left);
    void draw_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
        const Float4U& color, f32 width = 1.0f);
    void draw_outline(GUICore::IContext* context, const RectF& rect, const Float4U& color, f32 width = 1.0f);
    void draw_text(GUICore::IContext* context, const RectF& rect, const c8* text, f32 size,
        const Float4U& color, VG::TextAlignment alignment = VG::TextAlignment::begin);
    void bullet(GUICore::IContext* context, f32 x, f32 y, const c8* text);

    GUICore::ElementHandle begin_panel(GUICore::IContext* context, GUICore::id_t id, const c8* title,
        f32 width, f32 height);
    void end_panel(GUICore::IContext* context);
    void panel_label_value(GUICore::IContext* context, f32 y, const c8* label, const c8* value);
    bool circle_hit_test(const GUICore::IContext*, const GUICore::ElementHitTestRequest& request, void*);
    void set_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
        GUICore::PointerHitBehavior hit_behavior, GUICore::InteractableFlag flags);

    void build_header(GUICore::IContext* context);
    void build_frame_panel(GUICore::IContext* context, const CoreSheetState& state);
    void build_element_tree_panel(GUICore::IContext* context);
    void build_layout_panel(GUICore::IContext* context);
    void build_input_panel(GUICore::IContext* context);
    void build_hit_sample(GUICore::IContext* context, GUICore::id_t id, const c8* label,
        const Float4U& base, bool circle = false,
        GUICore::PointerHitBehavior behavior = GUICore::PointerHitBehavior::target);
    void build_draw_panel(GUICore::IContext* context);
    void build_state_panel(GUICore::IContext* context);
    void build_debug_panel(GUICore::IContext* context);
    void build_canvas_panel(GUICore::IContext* context);
}
