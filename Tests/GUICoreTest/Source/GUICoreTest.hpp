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
    inline constexpr f32 SHEET_WIDTH = 1366.0f;
    inline constexpr f32 SHEET_HEIGHT = 768.0f;
    inline constexpr u32 NUM_INPUT_SLICES = 3;
    inline constexpr u32 NUM_LAYOUT_SLICES = 14;
    inline constexpr u32 NUM_SLICES = NUM_INPUT_SLICES + NUM_LAYOUT_SLICES;

    enum : GUICore::id_t
    {
        ID_SCREEN_ROOT = 1,
        ID_SHEET = 2,
        ID_HEADER = 10,
        ID_INPUT = 20,
        ID_LAYOUT = 30,
        ID_POINTER_BASE_LAYER = 40,
        ID_POINTER_TOP_LAYER = 41,
        ID_POINTER_A = 42,
        ID_POINTER_B = 43,
        ID_POINTER_C = 44,
        ID_POINTER_D = 45,
        ID_POINTER_E = 46,
        ID_KEYBOARD = 50,
        ID_NAVIGATION = 51,
        ID_NAV_DEMO = 52,
        ID_IME_INPUT = 53,
        ID_NAV_GRID_BASE = 100,
        ID_NAV_SUBMENU_BASE = 120,
        ID_LAYOUT_FLEX_ROW = 60,
        ID_LAYOUT_FLEX_COLUMN = 61,
        ID_LAYOUT_FIXED_CHILD = 62,
        ID_LAYOUT_PERCENT_CHILD = 63,
        ID_LAYOUT_FIT_CHILD = 64,
        ID_LAYOUT_CANVAS = 65,
        ID_LAYOUT_SCROLL = 66,
        ID_LAYOUT_DEMO_BASE = 1000
    };

    struct CoreSheetState
    {
        u32 slice_index = 0;
        bool z_down = false;
        bool x_down = false;
        Float2U sheet_position = Float2U(36.0f, 36.0f);
        GUICore::CanvasLayoutItem screen_item;
        GUICore::CanvasLayoutDesc screen_canvas;
        Vector<GUICore::CanvasLayoutItem> sheet_items;
        GUICore::CanvasLayoutDesc sheet_canvas;
        Vector<GUICore::CanvasLayoutItem> pointer_items;
        GUICore::CanvasLayoutDesc pointer_canvas;
        Vector<GUICore::CanvasLayoutItem> pointer_base_items;
        GUICore::CanvasLayoutDesc pointer_base_canvas;
        String ime_text;
        Vector<GUICore::CanvasLayoutItem> keyboard_items;
        GUICore::CanvasLayoutDesc keyboard_canvas;
        bool navigation_submenu_open = false;
        GUICore::id_t navigation_pending_focus = 0;
        Vector<GUICore::CanvasLayoutItem> navigation_items;
        GUICore::CanvasLayoutDesc navigation_canvas;
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

    void build_slide_header(GUICore::IContext* context, const CoreSheetState& state);
    void build_pointer_input_slice(GUICore::IContext* context, CoreSheetState& state);
    void build_keyboard_input_slice(GUICore::IContext* context, CoreSheetState& state);
    void build_navigation_input_slice(GUICore::IContext* context, CoreSheetState& state);
    const c8* layout_slice_title(u32 layout_slice);
    const c8* layout_slice_subtitle(u32 layout_slice);
    void add_layout_slice_items(CoreSheetState& state, u32 layout_slice);
    void build_layout_slice(GUICore::IContext* context, u32 layout_slice);
}
