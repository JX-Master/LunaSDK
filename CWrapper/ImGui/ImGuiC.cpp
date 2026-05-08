#include "ImGui.h"

#include <Luna/ImGui/ImGui.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/RHI/Texture.hpp>

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

template <typename T>
T* object_as(luna_handle_t object)
{
    return object ? Luna::query_interface<T>(object) : nullptr;
}

const char* duplicate_string(const char* source)
{
    if (!source)
    {
        return nullptr;
    }
    auto size = std::strlen(source);
    auto* buffer = static_cast<char*>(Luna::memalloc(size + 1));
    if (!buffer)
    {
        return nullptr;
    }
    std::memcpy(buffer, source, size + 1);
    return buffer;
}

Luna::Float4 to_float4(const LunaImGuiFloat4& value)
{
    return Luna::Float4(value.x, value.y, value.z, value.w);
}

ImVec2 to_imvec2(const LunaImGuiFloat2& value)
{
    return ImVec2(value.x, value.y);
}

ImVec4 to_imvec4(const LunaImGuiFloat4& value)
{
    return ImVec4(value.x, value.y, value.z, value.w);
}

Luna::RectF to_rectf(const LunaImGuiRectF& value)
{
    Luna::RectF result;
    result.offset_x = value.offset_x;
    result.offset_y = value.offset_y;
    result.width = value.width;
    result.height = value.height;
    return result;
}

Luna::Float4x4 to_matrix(const LunaImGuiMatrix4x4& value)
{
    Luna::Float4x4 result;
    result.r[0] = to_float4(value.row0);
    result.r[1] = to_float4(value.row1);
    result.r[2] = to_float4(value.row2);
    result.r[3] = to_float4(value.row3);
    return result;
}

LunaImGuiMatrix4x4 from_matrix(const Luna::Float4x4& value)
{
    return LunaImGuiMatrix4x4{
        LunaImGuiFloat4{value.r[0].x, value.r[0].y, value.r[0].z, value.r[0].w},
        LunaImGuiFloat4{value.r[1].x, value.r[1].y, value.r[1].z, value.r[1].w},
        LunaImGuiFloat4{value.r[2].x, value.r[2].y, value.r[2].z, value.r[2].w},
        LunaImGuiFloat4{value.r[3].x, value.r[3].y, value.r[3].z, value.r[3].w}};
}

Luna::RHI::SamplerDesc to_sampler_desc(const LunaRhiSamplerDesc& value)
{
    Luna::RHI::SamplerDesc result;
    result.min_filter = static_cast<Luna::RHI::Filter>(value.min_filter);
    result.mag_filter = static_cast<Luna::RHI::Filter>(value.mag_filter);
    result.mip_filter = static_cast<Luna::RHI::Filter>(value.mip_filter);
    result.address_u = static_cast<Luna::RHI::TextureAddressMode>(value.address_u);
    result.address_v = static_cast<Luna::RHI::TextureAddressMode>(value.address_v);
    result.address_w = static_cast<Luna::RHI::TextureAddressMode>(value.address_w);
    result.anisotropy_enable = value.anisotropy_enable != 0;
    result.compare_enable = value.compare_enable != 0;
    result.compare_function = static_cast<Luna::RHI::CompareFunction>(value.compare_function);
    result.border_color = static_cast<Luna::RHI::BorderColor>(value.border_color);
    result.max_anisotropy = value.max_anisotropy;
    result.min_lod = value.min_lod;
    result.max_lod = value.max_lod;
    return result;
}

LunaRhiSamplerDesc from_sampler_desc(const Luna::RHI::SamplerDesc& value)
{
    return LunaRhiSamplerDesc{
        static_cast<uint32_t>(value.min_filter),
        static_cast<uint32_t>(value.mag_filter),
        static_cast<uint32_t>(value.mip_filter),
        static_cast<uint32_t>(value.address_u),
        static_cast<uint32_t>(value.address_v),
        static_cast<uint32_t>(value.address_w),
        value.anisotropy_enable ? 1 : 0,
        value.compare_enable ? 1 : 0,
        static_cast<uint32_t>(value.compare_function),
        static_cast<uint32_t>(value.border_color),
        value.max_anisotropy,
        value.min_lod,
        value.max_lod};
}

LunaRhiTextureHandle from_texture(Luna::RHI::ITexture* texture)
{
    if (!texture)
    {
        return LunaRhiTextureHandle{nullptr, nullptr};
    }
    auto object = texture->get_object();
    Luna::object_retain(object);
    return LunaRhiTextureHandle{object, texture};
}

LunaImGuiSampledImageHandle from_sampled_image(Luna::ImGuiUtils::ISampledImage* image)
{
    if (!image)
    {
        return LunaImGuiSampledImageHandle{nullptr, nullptr};
    }
    auto object = image->get_object();
    Luna::object_retain(object);
    return LunaImGuiSampledImageHandle{object, image};
}

struct ManagedInputTextCallbackData
{
    LunaImGuiInputTextCallback callback = nullptr;
    void* userdata = nullptr;
};

int input_text_callback_trampoline(ImGuiInputTextCallbackData* data)
{
    auto* callback_data = static_cast<ManagedInputTextCallbackData*>(data->UserData);
    if (!callback_data || !callback_data->callback)
    {
        return 0;
    }
    return callback_data->callback(data, callback_data->userdata);
}
}

extern "C"
{
LUNA_IMGUI_C_API luna_errcode_t luna_imgui_init_module(void)
{
    Luna::Module* module = Luna::module_imgui();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_IMGUI_C_API void luna_imgui_set_active_window(luna_handle_t window)
{
    Luna::ImGuiUtils::set_active_window(object_as<Luna::Window::IWindow>(window));
}

LUNA_IMGUI_C_API int32_t luna_imgui_handle_window_event(luna_handle_t event_object)
{
    return Luna::ImGuiUtils::handle_window_event(event_object) ? 1 : 0;
}

LUNA_IMGUI_C_API void luna_imgui_update_io(void)
{
    Luna::ImGuiUtils::update_io();
}

LUNA_IMGUI_C_API void luna_imgui_add_default_font(float font_size)
{
    Luna::ImGuiUtils::add_default_font(font_size);
}

LUNA_IMGUI_C_API void luna_imgui_get_glyph_ranges_default(LunaImGuiGlyphRange* out_ranges, uint64_t capacity, uint64_t* out_count)
{
    auto ranges = Luna::ImGuiUtils::get_glyph_ranges_default();
    if (out_count)
    {
        *out_count = static_cast<uint64_t>(ranges.size());
    }
    if (!out_ranges || capacity < static_cast<uint64_t>(ranges.size()))
    {
        return;
    }
    for (uint64_t i = 0; i < static_cast<uint64_t>(ranges.size()); ++i)
    {
        out_ranges[i].start = ranges[static_cast<Luna::usize>(i)].first;
        out_ranges[i].end = ranges[static_cast<Luna::usize>(i)].second;
    }
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_new_sampled_image(luna_handle_t texture, const LunaRhiSamplerDesc* sampler_desc, LunaImGuiSampledImageHandle* out_image)
{
    if (!out_image || !sampler_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto* native_texture = object_as<Luna::RHI::ITexture>(texture);
    if (!native_texture)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    auto image = Luna::ImGuiUtils::new_sampled_image(native_texture, to_sampler_desc(*sampler_desc));
    *out_image = from_sampled_image(image.get());
    return 0;
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_sampled_image_get_texture(void* self, LunaRhiTextureHandle* out_texture)
{
    if (!self || !out_texture)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_texture = from_texture(static_cast<Luna::ImGuiUtils::ISampledImage*>(self)->get_texture());
    return 0;
}

LUNA_IMGUI_C_API void luna_imgui_sampled_image_set_texture(void* self, luna_handle_t texture)
{
    if (!self)
    {
        return;
    }
    static_cast<Luna::ImGuiUtils::ISampledImage*>(self)->set_texture(object_as<Luna::RHI::ITexture>(texture));
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_sampled_image_get_sampler(void* self, LunaRhiSamplerDesc* out_sampler)
{
    if (!self || !out_sampler)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_sampler = from_sampler_desc(static_cast<Luna::ImGuiUtils::ISampledImage*>(self)->get_sampler());
    return 0;
}

LUNA_IMGUI_C_API void luna_imgui_sampled_image_set_sampler(void* self, const LunaRhiSamplerDesc* sampler_desc)
{
    if (!self || !sampler_desc)
    {
        return;
    }
    static_cast<Luna::ImGuiUtils::ISampledImage*>(self)->set_sampler(to_sampler_desc(*sampler_desc));
}

LUNA_IMGUI_C_API void luna_imgui_new_frame(void)
{
    ImGui::NewFrame();
}

LUNA_IMGUI_C_API void luna_imgui_show_demo_window(void)
{
    ImGui::ShowDemoWindow();
}

LUNA_IMGUI_C_API int32_t luna_imgui_begin(const char* name)
{
    if (!name)
    {
        return 0;
    }
    return ImGui::Begin(name) ? 1 : 0;
}

LUNA_IMGUI_C_API void luna_imgui_end(void)
{
    ImGui::End();
}

LUNA_IMGUI_C_API void luna_imgui_text(const char* text)
{
    ImGui::TextUnformatted(text ? text : "");
}

LUNA_IMGUI_C_API void luna_imgui_image_texture(luna_handle_t texture, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1)
{
    auto* native_texture = object_as<Luna::RHI::ITexture>(texture);
    if (!native_texture || !image_size)
    {
        return;
    }
    ImGui::Image(
        native_texture,
        to_imvec2(*image_size),
        uv0 ? to_imvec2(*uv0) : ImVec2(0.0f, 0.0f),
        uv1 ? to_imvec2(*uv1) : ImVec2(1.0f, 1.0f));
}

LUNA_IMGUI_C_API void luna_imgui_image_sampled_image(void* sampled_image, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1)
{
    auto* image = static_cast<Luna::ImGuiUtils::ISampledImage*>(sampled_image);
    if (!image || !image_size)
    {
        return;
    }
    ImGui::Image(
        image,
        to_imvec2(*image_size),
        uv0 ? to_imvec2(*uv0) : ImVec2(0.0f, 0.0f),
        uv1 ? to_imvec2(*uv1) : ImVec2(1.0f, 1.0f));
}

LUNA_IMGUI_C_API int32_t luna_imgui_image_button_texture(const char* str_id, luna_handle_t texture, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1, const LunaImGuiFloat4* bg_col, const LunaImGuiFloat4* tint_col)
{
    auto* native_texture = object_as<Luna::RHI::ITexture>(texture);
    if (!str_id || !native_texture || !image_size)
    {
        return 0;
    }
    return ImGui::ImageButton(
        str_id,
        native_texture,
        to_imvec2(*image_size),
        uv0 ? to_imvec2(*uv0) : ImVec2(0.0f, 0.0f),
        uv1 ? to_imvec2(*uv1) : ImVec2(1.0f, 1.0f),
        bg_col ? to_imvec4(*bg_col) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
        tint_col ? to_imvec4(*tint_col) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
        ? 1
        : 0;
}

LUNA_IMGUI_C_API int32_t luna_imgui_image_button_sampled_image(const char* str_id, void* sampled_image, const LunaImGuiFloat2* image_size, const LunaImGuiFloat2* uv0, const LunaImGuiFloat2* uv1, const LunaImGuiFloat4* bg_col, const LunaImGuiFloat4* tint_col)
{
    auto* image = static_cast<Luna::ImGuiUtils::ISampledImage*>(sampled_image);
    if (!str_id || !image || !image_size)
    {
        return 0;
    }
    return ImGui::ImageButton(
        str_id,
        image,
        to_imvec2(*image_size),
        uv0 ? to_imvec2(*uv0) : ImVec2(0.0f, 0.0f),
        uv1 ? to_imvec2(*uv1) : ImVec2(1.0f, 1.0f),
        bg_col ? to_imvec4(*bg_col) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
        tint_col ? to_imvec4(*tint_col) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
        ? 1
        : 0;
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text(const char* label, const char* value, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value)
{
    if (!label || !out_changed || !out_value)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::String text = value ? value : "";
    ManagedInputTextCallbackData callback_data{callback, userdata};
    *out_changed = ImGui::InputText(label, text, static_cast<ImGuiInputTextFlags>(flags), callback ? input_text_callback_trampoline : nullptr, callback ? &callback_data : nullptr) ? 1 : 0;
    *out_value = duplicate_string(text.c_str());
    return 0;
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text_multiline(const char* label, const char* value, const LunaImGuiFloat2* size, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value)
{
    if (!label || !size || !out_changed || !out_value)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::String text = value ? value : "";
    ManagedInputTextCallbackData callback_data{callback, userdata};
    *out_changed = ImGui::InputTextMultiline(label, text, to_imvec2(*size), static_cast<ImGuiInputTextFlags>(flags), callback ? input_text_callback_trampoline : nullptr, callback ? &callback_data : nullptr) ? 1 : 0;
    *out_value = duplicate_string(text.c_str());
    return 0;
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_input_text_with_hint(const char* label, const char* hint, const char* value, uint32_t flags, LunaImGuiInputTextCallback callback, void* userdata, int32_t* out_changed, const char** out_value)
{
    if (!label || !hint || !out_changed || !out_value)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::String text = value ? value : "";
    ManagedInputTextCallbackData callback_data{callback, userdata};
    *out_changed = ImGui::InputTextWithHint(label, hint, text, static_cast<ImGuiInputTextFlags>(flags), callback ? input_text_callback_trampoline : nullptr, callback ? &callback_data : nullptr) ? 1 : 0;
    *out_value = duplicate_string(text.c_str());
    return 0;
}

LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_flag(void* data)
{
    return data ? static_cast<uint32_t>(static_cast<ImGuiInputTextCallbackData*>(data)->EventFlag) : 0;
}

LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_flags(void* data)
{
    return data ? static_cast<uint32_t>(static_cast<ImGuiInputTextCallbackData*>(data)->Flags) : 0;
}

LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_char(void* data)
{
    return data ? static_cast<uint32_t>(static_cast<ImGuiInputTextCallbackData*>(data)->EventChar) : 0;
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_event_char(void* data, uint32_t ch)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->EventChar = static_cast<ImWchar>(ch);
}

LUNA_IMGUI_C_API uint32_t luna_imgui_input_text_callback_data_get_event_key(void* data)
{
    return data ? static_cast<uint32_t>(static_cast<ImGuiInputTextCallbackData*>(data)->EventKey) : 0;
}

LUNA_IMGUI_C_API const char* luna_imgui_input_text_callback_data_get_text(void* data)
{
    if (!data)
    {
        return nullptr;
    }
    return duplicate_string(static_cast<ImGuiInputTextCallbackData*>(data)->Buf);
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_text(void* data, const char* text)
{
    if (!data)
    {
        return;
    }
    auto* callback_data = static_cast<ImGuiInputTextCallbackData*>(data);
    callback_data->DeleteChars(0, callback_data->BufTextLen);
    callback_data->InsertChars(0, text ? text : "");
}

LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_buffer_size(void* data)
{
    return data ? static_cast<ImGuiInputTextCallbackData*>(data)->BufSize : 0;
}

LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_cursor_pos(void* data)
{
    return data ? static_cast<ImGuiInputTextCallbackData*>(data)->CursorPos : 0;
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_cursor_pos(void* data, int32_t pos)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->CursorPos = pos;
}

LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_selection_start(void* data)
{
    return data ? static_cast<ImGuiInputTextCallbackData*>(data)->SelectionStart : 0;
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_selection_start(void* data, int32_t pos)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->SelectionStart = pos;
}

LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_get_selection_end(void* data)
{
    return data ? static_cast<ImGuiInputTextCallbackData*>(data)->SelectionEnd : 0;
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_set_selection_end(void* data, int32_t pos)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->SelectionEnd = pos;
}

LUNA_IMGUI_C_API int32_t luna_imgui_input_text_callback_data_has_selection(void* data)
{
    return data && static_cast<ImGuiInputTextCallbackData*>(data)->HasSelection() ? 1 : 0;
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_delete_chars(void* data, int32_t pos, int32_t bytes_count)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->DeleteChars(pos, bytes_count);
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_insert_chars(void* data, int32_t pos, const char* text)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->InsertChars(pos, text ? text : "");
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_select_all(void* data)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->SelectAll();
}

LUNA_IMGUI_C_API void luna_imgui_input_text_callback_data_clear_selection(void* data)
{
    if (!data)
    {
        return;
    }
    static_cast<ImGuiInputTextCallbackData*>(data)->ClearSelection();
}

LUNA_IMGUI_C_API void luna_imgui_gizmo(LunaImGuiMatrix4x4* world_matrix, const LunaImGuiMatrix4x4* view, const LunaImGuiMatrix4x4* projection, const LunaImGuiRectF* viewport_rect, uint32_t operation, uint32_t mode, float snap, int32_t enabled, int32_t orthographic, LunaImGuiMatrix4x4* delta_matrix, int32_t* out_is_mouse_hover, int32_t* out_is_mouse_moving)
{
    if (!world_matrix || !view || !projection || !viewport_rect)
    {
        return;
    }
    auto native_world = to_matrix(*world_matrix);
    auto native_view = to_matrix(*view);
    auto native_projection = to_matrix(*projection);
    Luna::Float4x4 native_delta{};
    bool is_mouse_hover = false;
    bool is_mouse_moving = false;
    ImGui::Gizmo(
        native_world,
        native_view,
        native_projection,
        to_rectf(*viewport_rect),
        static_cast<ImGui::GizmoOperation>(operation),
        static_cast<ImGui::GizmoMode>(mode),
        snap,
        enabled != 0,
        orthographic != 0,
        delta_matrix ? &native_delta : nullptr,
        out_is_mouse_hover ? &is_mouse_hover : nullptr,
        out_is_mouse_moving ? &is_mouse_moving : nullptr);
    *world_matrix = from_matrix(native_world);
    if (delta_matrix)
    {
        *delta_matrix = from_matrix(native_delta);
    }
    if (out_is_mouse_hover)
    {
        *out_is_mouse_hover = is_mouse_hover ? 1 : 0;
    }
    if (out_is_mouse_moving)
    {
        *out_is_mouse_moving = is_mouse_moving ? 1 : 0;
    }
}

LUNA_IMGUI_C_API void luna_imgui_render(void)
{
    ImGui::Render();
}

LUNA_IMGUI_C_API luna_errcode_t luna_imgui_render_draw_data(luna_handle_t command_buffer, luna_handle_t render_target)
{
    auto* cmd = object_as<Luna::RHI::ICommandBuffer>(command_buffer);
    auto* target = object_as<Luna::RHI::ITexture>(render_target);
    if (!cmd || !target)
    {
        return from_errcode(Luna::BasicError::bad_cast());
    }
    return from_result(Luna::ImGuiUtils::render_draw_data(ImGui::GetDrawData(), cmd, target));
}
}
