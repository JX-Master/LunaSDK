/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.hpp
* @author JXMaster
* @date 2026/7/1
*/
#pragma once
#include <Luna/GUI/GUI.hpp>
#include <Luna/VG/TextArranger.hpp>

namespace Luna::GUITest
{
    inline constexpr f32 SHEET_WIDTH = 1366.0f;
    inline constexpr f32 SHEET_HEIGHT = 768.0f;
    inline constexpr u32 NUM_INPUT_SLICES = 3;
    inline constexpr u32 NUM_LAYOUT_SLICES = 15;
    inline constexpr u32 NUM_SDF_SLICES = 1;
    inline constexpr u32 NUM_BLUR_SLICES = 1;
    inline constexpr u32 NUM_WORLD_SLICES = 1;
    inline constexpr u32 NUM_SLICES = NUM_INPUT_SLICES + NUM_LAYOUT_SLICES + NUM_SDF_SLICES +
        NUM_BLUR_SLICES + NUM_WORLD_SLICES;
    inline constexpr u32 BLUR_MATERIALS_SLICE =
        NUM_INPUT_SLICES + NUM_LAYOUT_SLICES + NUM_SDF_SLICES;
    inline constexpr u32 WORLD_SURFACE_SLICE = NUM_SLICES - 1;

    enum : GUI::id_t
    {
        ID_SCREEN_ROOT = 1,
        ID_SHEET = 2,
        ID_HEADER = 10,
        ID_INPUT = 20,
        ID_LAYOUT = 30,
        ID_SDF = 31,
        ID_BLUR = 32,
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
        ID_SDF_SAMPLE_BASE = 200,
        ID_BLUR_BACKGROUND = 300,
        ID_BLUR_CLEAR = 301,
        ID_BLUR_ACRYLIC = 302,
        ID_BLUR_MICA = 303,
        ID_BLUR_LAMINATED = 304,
        ID_BLUR_GRAIN_FROST = 305,
        ID_BLUR_SMOKED = 306,
        ID_BLUR_ICE_FROST = 307,
        ID_BLUR_CHAMPAGNE = 308,
        ID_LAYOUT_DEMO_BASE = 1000
    };

    struct SheetState
    {
        u32 slice_index = 0;
        bool z_down = false;
        bool x_down = false;
        f32 animation_time = 0.0f;
        Float2U sheet_position = Float2U(36.0f, 36.0f);
        GUI::CanvasLayoutItem screen_item;
        GUI::CanvasLayoutDesc screen_canvas;
        Vector<GUI::CanvasLayoutItem> sheet_items;
        GUI::CanvasLayoutDesc sheet_canvas;
        Vector<GUI::CanvasLayoutItem> pointer_items;
        GUI::CanvasLayoutDesc pointer_canvas;
        Vector<GUI::CanvasLayoutItem> pointer_base_items;
        GUI::CanvasLayoutDesc pointer_base_canvas;
        String ime_text;
        Vector<GUI::CanvasLayoutItem> keyboard_items;
        GUI::CanvasLayoutDesc keyboard_canvas;
        bool navigation_submenu_open = false;
        GUI::id_t navigation_pending_focus = 0;
        Vector<GUI::CanvasLayoutItem> navigation_items;
        GUI::CanvasLayoutDesc navigation_canvas;
        GUI::ScrollViewportLayoutDesc scroll_viewport_layout;
        Vector<GUI::CanvasLayoutItem> sdf_items;
        GUI::CanvasLayoutDesc sdf_canvas;
        Vector<GUI::CanvasLayoutItem> blur_items;
        GUI::CanvasLayoutDesc blur_canvas;
    };

    GUI::LayoutConfig fixed_layout(f32 width, f32 height);
    void set_canvas_layout(GUI::IContext* context, const GUI::ElementHandle& element,
        GUI::CanvasLayoutDesc* desc);
    void add_canvas_item(Vector<GUI::CanvasLayoutItem>& items, GUI::id_t id, f32 x, f32 y);

    void draw_rect(GUI::IContext* context, const RectF& rect, const Float4U& color, f32 radius = 0.0f);
    void draw_shadow(GUI::IContext* context, const RectF& rect, const Float4U& color,
        f32 radius, const GUI::ShadowDesc& desc);
    void draw_gradient_rect(GUI::IContext* context, const RectF& rect, const Float4U& top_left,
        const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left);
    void draw_line(GUI::IContext* context, const Float2U& begin, const Float2U& end,
        const Float4U& color, f32 width = 1.0f);
    void draw_outline(GUI::IContext* context, const RectF& rect, const Float4U& color, f32 width = 1.0f);
    void draw_text(GUI::IContext* context, const RectF& rect, const c8* text, f32 size,
        const Float4U& color, VG::TextAlignment alignment = VG::TextAlignment::begin);
    void bullet(GUI::IContext* context, f32 x, f32 y, const c8* text);

    GUI::ElementHandle begin_panel(GUI::IContext* context, GUI::id_t id, const c8* title,
        f32 width, f32 height);
    void end_panel(GUI::IContext* context);
    void panel_label_value(GUI::IContext* context, f32 y, const c8* label, const c8* value);
    bool circle_hit_test(const GUI::IContext*, const GUI::ElementHitTestRequest& request, void*);
    void set_interactable(GUI::IContext* context, const GUI::ElementHandle& element,
        GUI::PointerHitBehavior hit_behavior, GUI::InteractableFlag flags);
    RV draw_sheet_callback(GUI::IContext* context, const GUI::ElementHandle& element,
        GUI::DrawPhase phase, void* userdata);

    void build_slide_header(GUI::IContext* context, const SheetState& state);
    void build_pointer_input_slice(GUI::IContext* context, SheetState& state);
    void build_keyboard_input_slice(GUI::IContext* context, SheetState& state);
    void build_navigation_input_slice(GUI::IContext* context, SheetState& state);
    const c8* layout_slice_title(u32 layout_slice);
    const c8* layout_slice_subtitle(u32 layout_slice);
    void add_layout_slice_items(SheetState& state, u32 layout_slice);
    void build_layout_slice(GUI::IContext* context, SheetState& state, u32 layout_slice);
    bool process_layout_slice_input(GUI::IContext* context, SheetState& state, u32 layout_slice);
    void add_sdf_slice_items(SheetState& state);
    void build_sdf_slice(GUI::IContext* context, SheetState& state);
    void add_blur_slice_items(SheetState& state);
    void build_blur_slice(GUI::IContext* context, SheetState& state);
}
