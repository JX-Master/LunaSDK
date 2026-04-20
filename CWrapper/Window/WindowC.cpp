#include "Window.h"

#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/PlatformDefines.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Window/Application.hpp>
#include <Luna/Window/Clipboard.hpp>
#include <Luna/Window/Display.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Window/Window.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

Luna::Window::IWindow* as_window(void* self)
{
    return reinterpret_cast<Luna::Window::IWindow*>(self);
}

luna_type_t from_type(Luna::typeinfo_t type)
{
    return type.handle;
}

template <typename T>
luna_type_t event_type()
{
    return from_type(Luna::typeof<T>());
}

template <typename T>
T* as_event(luna_handle_t event_object)
{
    return event_object && Luna::object_is_type(event_object, Luna::typeof<T>()) ? reinterpret_cast<T*>(event_object) : nullptr;
}

void set_u32_pair(uint32_t* out_x, uint32_t* out_y, Luna::u32 x, Luna::u32 y)
{
    if (out_x)
    {
        *out_x = x;
    }
    if (out_y)
    {
        *out_y = y;
    }
}

void set_i32_pair(int32_t* out_x, int32_t* out_y, Luna::i32 x, Luna::i32 y)
{
    if (out_x)
    {
        *out_x = x;
    }
    if (out_y)
    {
        *out_y = y;
    }
}

void set_f32_pair(float* out_x, float* out_y, Luna::f32 x, Luna::f32 y)
{
    if (out_x)
    {
        *out_x = x;
    }
    if (out_y)
    {
        *out_y = y;
    }
}

#if defined(LUNA_PLATFORM_DESKTOP)
    LunaWindowVideoMode from_video_mode(const Luna::Window::VideoMode& mode)
    {
        return LunaWindowVideoMode
        {
            mode.width,
            mode.height,
            mode.bits_per_pixel,
            mode.refresh_rate
        };
    }
#endif

    LunaWindowPointI from_point_i(const Luna::Int2U& point)
    {
        return LunaWindowPointI{point.x, point.y};
    }

    LunaWindowRectI from_rect_i(const Luna::RectI& rect)
    {
        return LunaWindowRectI{rect.offset_x, rect.offset_y, rect.width, rect.height};
    }

    Luna::RectI to_rect_i(const LunaWindowRectI& rect)
    {
        return Luna::RectI(rect.offset_x, rect.offset_y, rect.width, rect.height);
    }

    luna_errcode_t copy_string_to_c(const char* text, const char** out_text)
    {
        if (!out_text)
        {
            return from_errcode(Luna::BasicError::bad_arguments());
        }
        text = text ? text : "";
        Luna::usize size = std::strlen(text);
        char* buffer = reinterpret_cast<char*>(Luna::memalloc(size + 1));
        if (!buffer)
        {
            return from_errcode(Luna::BasicError::out_of_memory());
        }
        std::memcpy(buffer, text, size + 1);
        *out_text = buffer;
        return 0;
    }

    void free_string_list(const char** texts, uint64_t count)
    {
        if (!texts)
        {
            return;
        }
        for (uint64_t i = 0; i < count; ++i)
        {
            Luna::memfree(const_cast<char*>(texts[i]));
        }
        Luna::memfree(const_cast<char**>(texts));
    }

    luna_errcode_t copy_paths_to_c(const Luna::Vector<Luna::Path>& paths, LunaWindowStringList* out_paths)
    {
        if (!out_paths)
        {
            return from_errcode(Luna::BasicError::bad_arguments());
        }
        out_paths->items = nullptr;
        out_paths->count = 0;
        if (paths.empty())
        {
            return 0;
        }
        const char** items = reinterpret_cast<const char**>(Luna::memalloc(sizeof(char*) * paths.size()));
        if (!items)
        {
            return from_errcode(Luna::BasicError::out_of_memory());
        }
        for (Luna::usize i = 0; i < paths.size(); ++i)
        {
            auto encoded_path = paths[i].encode();
            luna_errcode_t err = copy_string_to_c(encoded_path.c_str(), &items[i]);
            if (err)
            {
                free_string_list(items, static_cast<uint64_t>(i));
                return err;
            }
        }
        out_paths->items = items;
        out_paths->count = static_cast<uint64_t>(paths.size());
        return 0;
    }

    luna_errcode_t build_dialog_filters(
        const LunaWindowFileDialogFilter* filters,
        uint64_t filter_count,
        Luna::Vector<Luna::Vector<const Luna::c8*>>& extension_sets,
        Luna::Vector<Luna::Window::FileDialogFilter>& out_filters)
    {
        if (filter_count && !filters)
        {
            return from_errcode(Luna::BasicError::bad_arguments());
        }
        extension_sets.resize(static_cast<Luna::usize>(filter_count));
        out_filters.reserve(static_cast<Luna::usize>(filter_count));
        for (uint64_t i = 0; i < filter_count; ++i)
        {
            if (filters[i].extension_count && !filters[i].extensions)
            {
                return from_errcode(Luna::BasicError::bad_arguments());
            }
            auto& extensions = extension_sets[static_cast<Luna::usize>(i)];
            extensions.reserve(static_cast<Luna::usize>(filters[i].extension_count));
            for (uint64_t j = 0; j < filters[i].extension_count; ++j)
            {
                const Luna::c8* extension = filters[i].extensions[j];
                if (extension && extension[0])
                {
                    extensions.push_back(extension);
                }
            }
            Luna::Window::FileDialogFilter filter;
            filter.name = filters[i].name ? filters[i].name : "";
            filter.extensions = Luna::Span<const Luna::c8*>(extensions.data(), extensions.size());
            out_filters.push_back(filter);
        }
        return 0;
    }
}

extern "C"
{
LUNA_WINDOW_C_API luna_errcode_t luna_window_init_module(const char* app_name)
{
    Luna::Window::StartupParams params;
    params.name = app_name;
    Luna::Window::set_startup_params(params);

    Luna::Module* window_module = Luna::module_window();
    Luna::RV result = Luna::add_module(window_module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(window_module);
    return from_result(result);
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_new(
    const char* title,
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t style_flags,
    uint32_t creation_flags,
    LunaWindowHandle* out_window)
{
    if (!out_window)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_window->object = nullptr;
    out_window->iwindow = nullptr;

    auto result = Luna::Window::new_window(
        title ? title : "",
        x,
        y,
        width,
        height,
        static_cast<Luna::Window::WindowStyleFlag>(style_flags),
        static_cast<Luna::Window::WindowCreationFlag>(creation_flags));

    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::Window::IWindow> window = Luna::move(result.get());
    Luna::object_t object = window.detach();
    void* iwindow = Luna::query_interface<Luna::Window::IWindow>(object);
    if (!iwindow)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_window->object = object;
    out_window->iwindow = iwindow;
    return 0;
}

LUNA_WINDOW_C_API void luna_window_poll_events(int32_t wait_events)
{
    Luna::Window::poll_events(wait_events != 0);
}

LUNA_WINDOW_C_API void luna_window_set_event_handler(LunaWindowEventHandler event_handler, void* userdata)
{
    Luna::Window::set_event_handler(event_handler, userdata);
}

LUNA_WINDOW_C_API void luna_window_get_event_handler(LunaWindowEventHandler* out_event_handler, void** out_userdata)
{
    Luna::Window::get_event_handler(out_event_handler, out_userdata);
}

LUNA_WINDOW_C_API luna_display_t luna_window_display_get_primary(void)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    auto display = Luna::Window::get_primary_display();
    if (display)
    {
        return display;
    }
    Luna::Vector<Luna::Window::display_t> displays;
    Luna::Window::get_displays(displays);
    return displays.empty() ? nullptr : displays[0];
#else
    return nullptr;
#endif
}

LUNA_WINDOW_C_API void luna_window_display_get_all(luna_display_t* out_displays, uint64_t capacity, uint64_t* out_count)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    Luna::Vector<Luna::Window::display_t> displays;
    Luna::Window::get_displays(displays);
    if (out_count)
    {
        *out_count = static_cast<uint64_t>(displays.size());
    }
    if (!out_displays)
    {
        return;
    }
    Luna::usize num_to_write = Luna::min<Luna::usize>(static_cast<Luna::usize>(capacity), displays.size());
    for (Luna::usize i = 0; i < num_to_write; ++i)
    {
        out_displays[i] = displays[i];
    }
#else
    if (out_count)
    {
        *out_count = 0;
    }
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_supported_video_modes(
    luna_display_t display,
    LunaWindowVideoMode* out_modes,
    uint64_t capacity,
    uint64_t* out_count)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    Luna::Vector<Luna::Window::VideoMode> modes;
    auto result = Luna::Window::get_display_supported_video_modes(display, modes);
    if (!result.valid())
    {
        return from_result(result);
    }
    if (out_count)
    {
        *out_count = static_cast<uint64_t>(modes.size());
    }
    if (!out_modes)
    {
        return 0;
    }
    Luna::usize num_to_write = Luna::min<Luna::usize>(static_cast<Luna::usize>(capacity), modes.size());
    for (Luna::usize i = 0; i < num_to_write; ++i)
    {
        out_modes[i] = from_video_mode(modes[i]);
    }
    return 0;
#else
    if (out_count)
    {
        *out_count = 0;
    }
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_video_mode(luna_display_t display, LunaWindowVideoMode* out_mode)
{
    if (!out_mode)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    auto result = Luna::Window::get_display_video_mode(display);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_mode = from_video_mode(result.get());
    return 0;
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_position(luna_display_t display, LunaWindowPointI* out_position)
{
    if (!out_position)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    auto result = Luna::Window::get_display_position(display);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_position = from_point_i(result.get());
    return 0;
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_working_area(luna_display_t display, LunaWindowRectI* out_rect)
{
    if (!out_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    auto result = Luna::Window::get_display_working_area(display);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_rect = from_rect_i(result.get());
    return 0;
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_display_get_name(luna_display_t display, const char** out_name)
{
    if (!out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = nullptr;
#if defined(LUNA_PLATFORM_DESKTOP)
    auto result = Luna::Window::get_display_name(display);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    Luna::String display_name = static_cast<Luna::String>(result.get());
    return copy_string_to_c(display_name.c_str(), out_name);
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API void luna_window_free_string(const char* text)
{
    Luna::memfree(const_cast<char*>(text));
}

LUNA_WINDOW_C_API void luna_window_free_string_list(const char** texts, uint64_t count)
{
    free_string_list(texts, count);
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_clipboard_get_text(const char** out_text)
{
    if (!out_text)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_text = "";
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
    Luna::String clipboard_text;
    auto result = Luna::Window::get_clipboard_text(clipboard_text);
    if (!result.valid())
    {
        return from_result(result);
    }
    return copy_string_to_c(clipboard_text.c_str(), out_text);
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_clipboard_set_text(const char* text)
{
    if (!text)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
    return from_result(Luna::Window::set_clipboard_text(text));
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_message_box(
    const char* text,
    const char* caption,
    uint32_t type,
    uint32_t icon,
    uint32_t* out_button)
{
    if (!text || !caption || !out_button)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_button = 0;
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS)
    auto result = Luna::Window::message_box(
        text,
        caption,
        static_cast<Luna::Window::MessageBoxType>(type),
        static_cast<Luna::Window::MessageBoxIcon>(icon));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_button = static_cast<uint32_t>(result.get());
    return 0;
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_open_file_dialog(
    const char* title,
    const LunaWindowFileDialogFilter* filters,
    uint64_t filter_count,
    const char* initial_dir,
    uint32_t flags,
    LunaWindowStringList* out_paths)
{
    if (!out_paths)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_paths->items = nullptr;
    out_paths->count = 0;
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS)
    Luna::Vector<Luna::Vector<const Luna::c8*>> extension_sets;
    Luna::Vector<Luna::Window::FileDialogFilter> native_filters;
    luna_errcode_t err = build_dialog_filters(filters, filter_count, extension_sets, native_filters);
    if (err)
    {
        return err;
    }
    auto result = Luna::Window::open_file_dialog(
        title,
        Luna::Span<const Luna::Window::FileDialogFilter>(native_filters.data(), native_filters.size()),
        initial_dir && initial_dir[0] ? Luna::Path(initial_dir) : Luna::Path(),
        static_cast<Luna::Window::FileDialogFlag>(flags));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    return copy_paths_to_c(result.get(), out_paths);
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_save_file_dialog(
    const char* title,
    const LunaWindowFileDialogFilter* filters,
    uint64_t filter_count,
    const char* initial_file_path,
    uint32_t flags,
    const char** out_path)
{
    if (!out_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_path = nullptr;
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS)
    Luna::Vector<Luna::Vector<const Luna::c8*>> extension_sets;
    Luna::Vector<Luna::Window::FileDialogFilter> native_filters;
    luna_errcode_t err = build_dialog_filters(filters, filter_count, extension_sets, native_filters);
    if (err)
    {
        return err;
    }
    auto result = Luna::Window::save_file_dialog(
        title,
        Luna::Span<const Luna::Window::FileDialogFilter>(native_filters.data(), native_filters.size()),
        initial_file_path && initial_file_path[0] ? Luna::Path(initial_file_path) : Luna::Path(),
        static_cast<Luna::Window::FileDialogFlag>(flags));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    auto encoded_path = result.get().encode();
    return copy_string_to_c(encoded_path.c_str(), out_path);
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_open_dir_dialog(
    const char* title,
    const char* initial_dir,
    const char** out_path)
{
    if (!out_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_path = nullptr;
#if defined(LUNA_PLATFORM_WINDOWS) || defined(LUNA_PLATFORM_MACOS)
    auto result = Luna::Window::open_dir_dialog(
        title,
        initial_dir && initial_dir[0] ? Luna::Path(initial_dir) : Luna::Path());
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    auto encoded_path = result.get().encode();
    return copy_string_to_c(encoded_path.c_str(), out_path);
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

#define LUNA_WINDOW_EVENT_TYPE_API(api_name, type_name) \
    LUNA_WINDOW_C_API luna_type_t api_name(void) \
    { \
        return event_type<Luna::Window::type_name>(); \
    }

LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_event_type, WindowEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_request_close_event_type, WindowRequestCloseEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_closed_event_type, WindowClosedEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_input_focus_event_type, WindowInputFocusEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_lose_input_focus_event_type, WindowLoseInputFocusEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_show_event_type, WindowShowEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_hide_event_type, WindowHideEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_resize_event_type, WindowResizeEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_framebuffer_resize_event_type, WindowFramebufferResizeEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_move_event_type, WindowMoveEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_dpi_scale_changed_event_type, WindowDPIScaleChangedEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_key_down_event_type, WindowKeyDownEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_key_up_event_type, WindowKeyUpEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_input_text_event_type, WindowInputTextEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_mouse_enter_event_type, WindowMouseEnterEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_mouse_leave_event_type, WindowMouseLeaveEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_mouse_move_event_type, WindowMouseMoveEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_mouse_down_event_type, WindowMouseDownEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_mouse_up_event_type, WindowMouseUpEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_scroll_event_type, WindowScrollEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_touch_down_event_type, WindowTouchDownEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_touch_move_event_type, WindowTouchMoveEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_touch_up_event_type, WindowTouchUpEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_window_drop_files_event_type, WindowDropFilesEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_event_type, ApplicationEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_did_enter_foreground_event_type, ApplicationDidEnterForegroundEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_will_enter_foreground_event_type, ApplicationWillEnterForegroundEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_did_enter_background_event_type, ApplicationDidEnterBackgroundEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_will_enter_background_event_type, ApplicationWillEnterBackgroundEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_will_terminate_event_type, ApplicationWillTerminateEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_application_did_receive_memory_warning_event_type, ApplicationDidReceiveMemoryWarningEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_screen_keyboard_shown_event_type, ScreenKeyboardShownEvent)
LUNA_WINDOW_EVENT_TYPE_API(luna_window_get_screen_keyboard_hidden_event_type, ScreenKeyboardHiddenEvent)

#undef LUNA_WINDOW_EVENT_TYPE_API

LUNA_WINDOW_C_API luna_errcode_t luna_window_event_get_window(luna_handle_t event_object, LunaWindowHandle* out_window)
{
    if (!out_window)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_window->object = nullptr;
    out_window->iwindow = nullptr;

    auto event = as_event<Luna::Window::WindowEvent>(event_object);
    if (!event)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    Luna::object_t object = event->window.object();
    if (!object)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    void* iwindow = Luna::query_interface<Luna::Window::IWindow>(object);
    if (!iwindow)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    out_window->object = object;
    out_window->iwindow = iwindow;
    return 0;
}

LUNA_WINDOW_C_API int32_t luna_window_request_close_event_get_do_close(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowRequestCloseEvent>(event_object);
    return event && event->do_close ? 1 : 0;
}

LUNA_WINDOW_C_API void luna_window_request_close_event_set_do_close(luna_handle_t event_object, int32_t do_close)
{
    auto event = as_event<Luna::Window::WindowRequestCloseEvent>(event_object);
    if (event)
    {
        event->do_close = do_close != 0;
    }
}

LUNA_WINDOW_C_API void luna_window_resize_event_get_size(luna_handle_t event_object, uint32_t* out_width, uint32_t* out_height)
{
    auto event = as_event<Luna::Window::WindowResizeEvent>(event_object);
    set_u32_pair(out_width, out_height, event ? event->width : 0, event ? event->height : 0);
}

LUNA_WINDOW_C_API void luna_window_framebuffer_resize_event_get_size(luna_handle_t event_object, uint32_t* out_width, uint32_t* out_height)
{
    auto event = as_event<Luna::Window::WindowFramebufferResizeEvent>(event_object);
    set_u32_pair(out_width, out_height, event ? event->width : 0, event ? event->height : 0);
}

LUNA_WINDOW_C_API void luna_window_move_event_get_position(luna_handle_t event_object, int32_t* out_x, int32_t* out_y)
{
    auto event = as_event<Luna::Window::WindowMoveEvent>(event_object);
    set_i32_pair(out_x, out_y, event ? event->x : 0, event ? event->y : 0);
}

LUNA_WINDOW_C_API uint32_t luna_window_key_down_event_get_key(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowKeyDownEvent>(event_object);
    return event ? static_cast<uint32_t>(event->key) : 0;
}

LUNA_WINDOW_C_API uint32_t luna_window_key_up_event_get_key(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowKeyUpEvent>(event_object);
    return event ? static_cast<uint32_t>(event->key) : 0;
}

LUNA_WINDOW_C_API const char* luna_window_input_text_event_get_text(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowInputTextEvent>(event_object);
    return event ? event->text.c_str() : "";
}

LUNA_WINDOW_C_API void luna_window_mouse_move_event_get_position(luna_handle_t event_object, int32_t* out_x, int32_t* out_y)
{
    auto event = as_event<Luna::Window::WindowMouseMoveEvent>(event_object);
    set_i32_pair(out_x, out_y, event ? event->x : 0, event ? event->y : 0);
}

LUNA_WINDOW_C_API uint32_t luna_window_mouse_down_event_get_button(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowMouseDownEvent>(event_object);
    return event ? static_cast<uint32_t>(event->button) : 0;
}

LUNA_WINDOW_C_API uint32_t luna_window_mouse_up_event_get_button(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowMouseUpEvent>(event_object);
    return event ? static_cast<uint32_t>(event->button) : 0;
}

LUNA_WINDOW_C_API void luna_window_scroll_event_get_delta(luna_handle_t event_object, float* out_x, float* out_y)
{
    auto event = as_event<Luna::Window::WindowScrollEvent>(event_object);
    set_f32_pair(out_x, out_y, event ? event->scroll_x : 0.0f, event ? event->scroll_y : 0.0f);
}

LUNA_WINDOW_C_API void luna_window_touch_down_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y)
{
    auto event = as_event<Luna::Window::WindowTouchDownEvent>(event_object);
    if (out_id)
    {
        *out_id = event ? event->id : 0;
    }
    set_f32_pair(out_x, out_y, event ? event->x : 0.0f, event ? event->y : 0.0f);
}

LUNA_WINDOW_C_API void luna_window_touch_move_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y)
{
    auto event = as_event<Luna::Window::WindowTouchMoveEvent>(event_object);
    if (out_id)
    {
        *out_id = event ? event->id : 0;
    }
    set_f32_pair(out_x, out_y, event ? event->x : 0.0f, event ? event->y : 0.0f);
}

LUNA_WINDOW_C_API void luna_window_touch_up_event_get_point(luna_handle_t event_object, uint64_t* out_id, float* out_x, float* out_y)
{
    auto event = as_event<Luna::Window::WindowTouchUpEvent>(event_object);
    if (out_id)
    {
        *out_id = event ? event->id : 0;
    }
    set_f32_pair(out_x, out_y, event ? event->x : 0.0f, event ? event->y : 0.0f);
}

LUNA_WINDOW_C_API uint64_t luna_window_drop_files_event_get_file_count(luna_handle_t event_object)
{
    auto event = as_event<Luna::Window::WindowDropFilesEvent>(event_object);
    return event ? event->files.size() : 0;
}

LUNA_WINDOW_C_API const char* luna_window_drop_files_event_get_file(luna_handle_t event_object, uint64_t index)
{
    auto event = as_event<Luna::Window::WindowDropFilesEvent>(event_object);
    if (!event || index >= event->files.size())
    {
        return "";
    }
    return event->files[static_cast<Luna::usize>(index)].c_str();
}

LUNA_WINDOW_C_API void luna_window_drop_files_event_get_position(luna_handle_t event_object, float* out_x, float* out_y)
{
    auto event = as_event<Luna::Window::WindowDropFilesEvent>(event_object);
    set_f32_pair(out_x, out_y, event ? event->x : 0.0f, event ? event->y : 0.0f);
}

LUNA_WINDOW_C_API void luna_window_iwindow_close(void* self)
{
    if (self)
    {
        as_window(self)->close();
    }
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_closed(void* self)
{
    return !self || as_window(self)->is_closed() ? 1 : 0;
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_has_input_focus(void* self)
{
    return self && as_window(self)->has_input_focus() ? 1 : 0;
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_has_mouse_focus(void* self)
{
    return self && as_window(self)->has_mouse_focus() ? 1 : 0;
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_minimized(void* self)
{
    return self && as_window(self)->is_minimized() ? 1 : 0;
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_foreground(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_foreground());
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_maximized(void* self)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    return self && as_window(self)->is_maximized() ? 1 : 0;
#else
    return 0;
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_minimized(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_minimized());
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_maximized(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_maximized());
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_restored(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_restored());
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_hovered(void* self)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    return self && as_window(self)->is_hovered() ? 1 : 0;
#else
    return 0;
#endif
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_visible(void* self)
{
    return self && as_window(self)->is_visible() ? 1 : 0;
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_visible(void* self, int32_t visible)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_window(self)->set_visible(visible != 0));
}

LUNA_WINDOW_C_API uint32_t luna_window_iwindow_get_style(void* self)
{
#if defined(LUNA_PLATFORM_DESKTOP)
    return self ? static_cast<uint32_t>(as_window(self)->get_style()) : 0;
#else
    return 0;
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_style(void* self, uint32_t style)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_style(static_cast<Luna::Window::WindowStyleFlag>(style)));
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_title(void* self, const char* title)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_window(self)->set_title(title ? title : ""));
}

LUNA_WINDOW_C_API void luna_window_iwindow_get_position(void* self, int32_t* out_x, int32_t* out_y)
{
    Luna::Int2U position(0, 0);
    if (self)
    {
        position = as_window(self)->get_position();
    }
    set_i32_pair(out_x, out_y, position.x, position.y);
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_position(void* self, int32_t x, int32_t y)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_position(x, y));
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API void luna_window_iwindow_get_size(void* self, uint32_t* out_width, uint32_t* out_height)
{
    Luna::UInt2U size(0, 0);
    if (self)
    {
        size = as_window(self)->get_size();
    }
    if (out_width)
    {
        *out_width = size.x;
    }
    if (out_height)
    {
        *out_height = size.y;
    }
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_size(void* self, uint32_t width, uint32_t height)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
#if defined(LUNA_PLATFORM_DESKTOP)
    return from_result(as_window(self)->set_size(width, height));
#else
    return from_errcode(Luna::BasicError::not_supported());
#endif
}

LUNA_WINDOW_C_API void luna_window_iwindow_get_framebuffer_size(void* self, uint32_t* out_width, uint32_t* out_height)
{
    Luna::UInt2U size(0, 0);
    if (self)
    {
        size = as_window(self)->get_framebuffer_size();
    }
    if (out_width)
    {
        *out_width = size.x;
    }
    if (out_height)
    {
        *out_height = size.y;
    }
}

LUNA_WINDOW_C_API float luna_window_iwindow_get_dpi_scale_factor(void* self)
{
    return self ? as_window(self)->get_dpi_scale_factor() : 1.0f;
}

LUNA_WINDOW_C_API void luna_window_iwindow_screen_to_client(void* self, int32_t x, int32_t y, int32_t* out_x, int32_t* out_y)
{
    Luna::Int2U point(x, y);
    if (self)
    {
        point = as_window(self)->screen_to_client(point);
    }
    set_i32_pair(out_x, out_y, point.x, point.y);
}

LUNA_WINDOW_C_API void luna_window_iwindow_client_to_screen(void* self, int32_t x, int32_t y, int32_t* out_x, int32_t* out_y)
{
    Luna::Int2U point(x, y);
    if (self)
    {
        point = as_window(self)->client_to_screen(point);
    }
    set_i32_pair(out_x, out_y, point.x, point.y);
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_begin_text_input(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_window(self)->begin_text_input());
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_set_text_input_area(void* self, const LunaWindowRectI* input_rect, int32_t cursor)
{
    if (!self || !input_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_window(self)->set_text_input_area(to_rect_i(*input_rect), cursor));
}

LUNA_WINDOW_C_API luna_errcode_t luna_window_iwindow_end_text_input(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_window(self)->end_text_input());
}

LUNA_WINDOW_C_API int32_t luna_window_iwindow_is_text_input_active(void* self)
{
    return self && as_window(self)->is_text_input_active() ? 1 : 0;
}
}
