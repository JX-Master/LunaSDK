#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_WINDOW_C_API __declspec(dllexport)
#else
#define LUNA_WINDOW_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    LUNA_WINDOW_DEFAULT_POS = INT32_MAX
};

typedef struct LunaWindowHandle
{
    luna_handle_t object;
    void* iwindow;
} LunaWindowHandle;

typedef void* luna_display_t;

typedef struct LunaWindowVideoMode
{
    uint32_t width;
    uint32_t height;
    uint32_t bits_per_pixel;
    uint32_t refresh_rate;
} LunaWindowVideoMode;

typedef struct LunaWindowPointI
{
    int32_t x;
    int32_t y;
} LunaWindowPointI;

typedef struct LunaWindowRectI
{
    int32_t offset_x;
    int32_t offset_y;
    int32_t width;
    int32_t height;
} LunaWindowRectI;

typedef struct LunaWindowFileDialogFilter
{
    const char* name;
    const char** extensions;
    uint64_t extension_count;
} LunaWindowFileDialogFilter;

typedef struct LunaWindowStringList
{
    const char** items;
    uint64_t count;
} LunaWindowStringList;

typedef void (*LunaWindowEventHandler)(luna_handle_t event_object, void* userdata);

LUNA_WINDOW_C_API luna_errcode_t luna_window_init_module(const char* app_name);
LUNA_WINDOW_C_API luna_errcode_t luna_window_new(
    const char* title,
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t style_flags,
    uint32_t creation_flags,
    LunaWindowHandle* out_window);
LUNA_WINDOW_C_API void luna_window_poll_events(int32_t wait_events);
LUNA_WINDOW_C_API void luna_window_set_event_handler(LunaWindowEventHandler event_handler, void* userdata);
LUNA_WINDOW_C_API void luna_window_get_event_handler(LunaWindowEventHandler* out_event_handler, void** out_userdata);

LUNA_WINDOW_C_API luna_display_t luna_window_display_get_primary(void);
LUNA_WINDOW_C_API void luna_window_display_get_all(luna_display_t* out_displays, uint64_t capacity, uint64_t* out_count);
LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_supported_video_modes(luna_display_t display, LunaWindowVideoMode* out_modes, uint64_t capacity, uint64_t* out_count);
LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_video_mode(luna_display_t display, LunaWindowVideoMode* out_mode);
LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_position(luna_display_t display, LunaWindowPointI* out_position);
LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_working_area(luna_display_t display, LunaWindowRectI* out_rect);
LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_name(luna_display_t display, const char** out_name);

LUNA_WINDOW_C_API void luna_window_free_string(const char* text);
LUNA_WINDOW_C_API void luna_window_free_string_list(const char** texts, uint64_t count);
LUNA_WINDOW_C_API luna_errcode_t luna_window_clipboard_get_text(const char** out_text);
LUNA_WINDOW_C_API luna_errcode_t luna_window_clipboard_set_text(const char* text);

LUNA_WINDOW_C_API luna_errcode_t luna_window_message_box(
    const char* text,
    const char* caption,
    uint32_t type,
    uint32_t icon,
    uint32_t* out_button);

LUNA_WINDOW_C_API luna_errcode_t luna_window_open_file_dialog(
    const char* title,
    const LunaWindowFileDialogFilter* filters,
    uint64_t filter_count,
    const char* initial_dir,
    uint32_t flags,
    LunaWindowStringList* out_paths);
LUNA_WINDOW_C_API luna_errcode_t luna_window_save_file_dialog(
    const char* title,
    const LunaWindowFileDialogFilter* filters,
    uint64_t filter_count,
    const char* initial_file_path,
    uint32_t flags,
    const char** out_path);
LUNA_WINDOW_C_API luna_errcode_t luna_window_open_dir_dialog(
    const char* title,
    const char* initial_dir,
    const char** out_path);

LUNA_WINDOW_C_API luna_type_t luna_window_get_window_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_request_close_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_closed_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_input_focus_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_lose_input_focus_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_show_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_hide_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_resize_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_framebuffer_resize_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_move_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_dpi_scale_changed_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_key_down_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_key_up_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_input_text_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_mouse_enter_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_mouse_leave_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_mouse_move_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_mouse_down_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_mouse_up_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_scroll_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_touch_down_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_touch_move_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_touch_up_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_window_drop_files_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_did_enter_foreground_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_will_enter_foreground_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_did_enter_background_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_will_enter_background_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_will_terminate_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_application_did_receive_memory_warning_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_screen_keyboard_shown_event_type(void);
LUNA_WINDOW_C_API luna_type_t luna_window_get_screen_keyboard_hidden_event_type(void);

LUNA_WINDOW_C_API luna_errcode_t luna_window_event_get_window(luna_handle_t event_object, LunaWindowHandle* out_window);
LUNA_WINDOW_C_API int32_t luna_window_request_close_event_get_do_close(luna_handle_t event_object);
LUNA_WINDOW_C_API void luna_window_request_close_event_set_do_close(luna_handle_t event_object, int32_t do_close);
LUNA_WINDOW_C_API void luna_window_resize_event_get_size(luna_handle_t event_object, uint32_t* out_width, uint32_t* out_height);
LUNA_WINDOW_C_API void luna_window_framebuffer_resize_event_get_size(luna_handle_t event_object, uint32_t* out_width, uint32_t* out_height);
LUNA_WINDOW_C_API void luna_window_move_event_get_position(luna_handle_t event_object, int32_t* out_x, int32_t* out_y);
LUNA_WINDOW_C_API uint32_t luna_window_key_down_event_get_key(luna_handle_t event_object);
LUNA_WINDOW_C_API uint32_t luna_window_key_up_event_get_key(luna_handle_t event_object);
LUNA_WINDOW_C_API const char* luna_window_input_text_event_get_text(luna_handle_t event_object);
LUNA_WINDOW_C_API void luna_window_mouse_move_event_get_position(luna_handle_t event_object, int32_t* out_x, int32_t* out_y);
LUNA_WINDOW_C_API uint32_t luna_window_mouse_down_event_get_button(luna_handle_t event_object);
LUNA_WINDOW_C_API uint32_t luna_window_mouse_up_event_get_button(luna_handle_t event_object);
LUNA_WINDOW_C_API void luna_window_scroll_event_get_delta(luna_handle_t event_object, float* out_x, float* out_y);
LUNA_WINDOW_C_API void luna_window_touch_down_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y);
LUNA_WINDOW_C_API void luna_window_touch_move_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y);
LUNA_WINDOW_C_API void luna_window_touch_up_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y);
LUNA_WINDOW_C_API uint64_t luna_window_drop_files_event_get_file_count(luna_handle_t event_object);
LUNA_WINDOW_C_API const char* luna_window_drop_files_event_get_file(luna_handle_t event_object, uint64_t index);
LUNA_WINDOW_C_API void luna_window_drop_files_event_get_position(luna_handle_t event_object, float* out_x, float* out_y);

LUNA_WINDOW_C_API void luna_window_iwindow_close(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_closed(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_has_input_focus(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_has_mouse_focus(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_minimized(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_foreground(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_maximized(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_minimized(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_maximized(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_restored(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_hovered(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_visible(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_visible(void* self, int32_t visible);
LUNA_WINDOW_C_API uint32_t luna_window_iwindow_get_style(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_style(void* self, uint32_t style);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_title(void* self, const char* title);
LUNA_WINDOW_C_API void luna_window_iwindow_get_position(void* self, int32_t* out_x, int32_t* out_y);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_position(void* self, int32_t x, int32_t y);
LUNA_WINDOW_C_API void luna_window_iwindow_get_size(void* self, uint32_t* out_width, uint32_t* out_height);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_size(void* self, uint32_t width, uint32_t height);
LUNA_WINDOW_C_API void luna_window_iwindow_get_framebuffer_size(void* self, uint32_t* out_width, uint32_t* out_height);
LUNA_WINDOW_C_API float luna_window_iwindow_get_dpi_scale_factor(void* self);
LUNA_WINDOW_C_API void luna_window_iwindow_screen_to_client(void* self, int32_t x, int32_t y, int32_t* out_x, int32_t* out_y);
LUNA_WINDOW_C_API void luna_window_iwindow_client_to_screen(void* self, int32_t x, int32_t y, int32_t* out_x, int32_t* out_y);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_begin_text_input(void* self);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_text_input_area(void* self, const LunaWindowRectI* input_rect, int32_t cursor);
LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_end_text_input(void* self);
LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_text_input_active(void* self);

#ifdef __cplusplus
}
#endif
