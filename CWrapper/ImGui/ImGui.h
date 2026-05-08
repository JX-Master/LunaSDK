#pragma once

#include "../Runtime/Runtime.h"
#include "../RHI/RHI.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_IMGUI_C_API __declspec(dllexport)
#else
#define LUNA_IMGUI_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaImGuiFloat2
{
    float x;
    float y;
} LunaImGuiFloat2;

typedef struct LunaImGuiFloat4
{
    float x;
    float y;
    float z;
    float w;
} LunaImGuiFloat4;

typedef struct LunaImGuiMatrix4x4
{
    LunaImGuiFloat4 row0;
    LunaImGuiFloat4 row1;
    LunaImGuiFloat4 row2;
    LunaImGuiFloat4 row3;
} LunaImGuiMatrix4x4;

typedef struct LunaImGuiRectF
{
    float offset_x;
    float offset_y;
    float width;
    float height;
} LunaImGuiRectF;

typedef struct LunaImGuiGlyphRange
{
    uint16_t start;
    uint16_t end;
} LunaImGuiGlyphRange;

typedef struct LunaImGuiSampledImageHandle
{
    luna_handle_t object;
    void* isampled_image;
} LunaImGuiSampledImageHandle;

typedef int32_t (*LunaImGuiInputTextCallback)(void* data, void* userdata);

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_init_module(void);

LUNA_IMGUI_C_API void luna_imgui_set_active_window(luna_handle_t window);
LUNA_IMGUI_C_API int32_t luna_imgui_handle_window_event(luna_handle_t event_object);
LUNA_IMGUI_C_API void luna_imgui_update_io(void);
LUNA_IMGUI_C_API void luna_imgui_add_default_font(float font_size);
LUNA_IMGUI_C_API void luna_imgui_get_glyph_ranges_default(LunaImGuiGlyphRange* out_ranges, uint64_t capacity, uint64_t* out_count);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_new_sampled_image(luna_handle_t texture, const LunaRhiSamplerDesc* sampler_desc, LunaImGuiSampledImageHandle* out_image);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_sampled_image_get_texture(void* self, LunaRhiTextureHandle* out_texture);
LUNA_IMGUI_C_API void luna_imgui_sampled_image_set_texture(void* self, luna_handle_t texture);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_sampled_image_get_sampler(void* self, LunaRhiSamplerDesc* out_sampler);
LUNA_IMGUI_C_API void luna_imgui_sampled_image_set_sampler(void* self, const LunaRhiSamplerDesc* sampler_desc);

LUNA_IMGUI_C_API void luna_imgui_new_frame(void);
LUNA_IMGUI_C_API void luna_imgui_show_demo_window(void);
LUNA_IMGUI_C_API int32_t luna_imgui_begin(const char* name);
LUNA_IMGUI_C_API void luna_imgui_end(void);
LUNA_IMGUI_C_API void luna_imgui_text(const char* text);
LUNA_IMGUI_C_API void luna_imgui_image_texture(luna_handle_t texture, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1);
LUNA_IMGUI_C_API void luna_imgui_image_sampled_image(void* sampled_image, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1);
LUNA_IMGUI_C_API int32_t luna_imgui_image_button_texture(const char* str_id, luna_handle_t texture, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1, const LunaImGuiFloat4* bg_col, const LunaImGuiFloat4* tint_col);
LUNA_IMGUI_C_API int32_t luna_imgui_image_button_sampled_image(const char* str_id, void* sampled_image, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1, const LunaImGuiFloat4* bg_col, const LunaImGuiFloat4* tint_col);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text(const char* label, const char* value, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text_multiline(const char* label, const char* value, const LunaImGuiFloat2* size, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value);
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text_with_hint(const char* label, const char* hint, const char* value, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value);
LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_flag(void* data);
LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_flags(void* data);
LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_char(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_event_char(void* data, uint32_t ch);
LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_key(void* data);
LUNA_IMGUI_C_API const char* luna_imgui_input_text_callback_data_get_text(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_text(void* data, const char* text);
LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_buffer_size(void* data);
LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_cursor_pos(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_cursor_pos(void* data, int32_t pos);
LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_selection_start(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_selection_start(void* data, int32_t pos);
LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_selection_end(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_selection_end(void* data, int32_t pos);
LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_has_selection(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_delete_chars(void* data, int32_t pos, int32_t bytes_count);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_insert_chars(void* data, int32_t pos, const char* text);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_select_all(void* data);
LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_clear_selection(void* data);
LUNA_IMGUI_C_API void luna_imgui_gizmo(LunaImGuiMatrix4x4* world_matrix, const LunaImGuiMatrix4x4* view, const LunaImGuiMatrix4x4* projection, const LunaImGuiRectF* viewport_rect, uint32_t operation, uint32_t mode, float snap, int32_t enabled, int32_t orthographic, LunaImGuiMatrix4x4* delta_matrix, int32_t* out_is_mouse_hover, int32_t* out_is_mouse_moving);
LUNA_IMGUI_C_API void luna_imgui_render(void);

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_render_draw_data(luna_handle_t command_buffer, luna_handle_t render_target);

#ifdef __cplusplus
}
#endif
