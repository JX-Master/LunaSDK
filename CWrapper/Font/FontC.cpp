#include "Font.h"

#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Vector.hpp>

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

Luna::Font::IFontFile* as_font_file(void* self)
{
    return reinterpret_cast<Luna::Font::IFontFile*>(self);
}

template <typename T>
T* object_as(luna_handle_t object)
{
    return object ? Luna::query_interface<T>(object) : nullptr;
}

LunaFontRectI from_rect(const Luna::RectI& rect)
{
    return LunaFontRectI
    {
        rect.offset_x,
        rect.offset_y,
        rect.width,
        rect.height
    };
}
}

extern "C"
{
LUNA_FONT_C_API luna_errcode_t luna_font_init_module(void)
{
    Luna::Module* module = Luna::module_font();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_FONT_C_API luna_errcode_t luna_font_load_ttf_font_file(const void* data, uint64_t data_size, LunaFontHandle* out_font_file)
{
    if ((!data && data_size) || !out_font_file)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_font_file->object = nullptr;
    out_font_file->ifont_file = nullptr;

    auto result = Luna::Font::load_ttf_font_file(static_cast<const Luna::byte_t*>(data), static_cast<Luna::usize>(data_size));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::Font::IFontFile> font_file = Luna::move(result.get());
    Luna::object_t object = font_file.detach();
    auto ifont_file = object_as<Luna::Font::IFontFile>(object);
    if (!ifont_file)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_font_file->object = object;
    out_font_file->ifont_file = ifont_file;
    return 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_get_default_font(LunaFontHandle* out_font_file)
{
    if (!out_font_file)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_font_file->object = nullptr;
    out_font_file->ifont_file = nullptr;

    auto* ifont_file = Luna::Font::get_default_font();
    if (!ifont_file)
    {
        return from_errcode(Luna::BasicError::not_ready());
    }

    auto object = static_cast<Luna::object_t>(ifont_file);
    Luna::object_retain(object);
    out_font_file->object = object;
    out_font_file->ifont_file = ifont_file;
    return 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_data(void* self, void** out_data, uint64_t* out_size)
{
    if (!self || !out_data || !out_size)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    *out_data = nullptr;
    *out_size = 0;

    auto data = as_font_file(self)->get_data();
    if (!data.empty())
    {
        auto* buffer = Luna::memalloc(data.size());
        if (!buffer)
        {
            return from_errcode(Luna::BasicError::out_of_memory());
        }
        std::memcpy(buffer, data.data(), data.size());
        *out_data = buffer;
        *out_size = static_cast<uint64_t>(data.size());
    }
    return 0;
}

LUNA_FONT_C_API uint32_t luna_font_ifont_file_get_num_fonts(void* self)
{
    return self ? as_font_file(self)->get_num_fonts() : 0;
}

LUNA_FONT_C_API int32_t luna_font_ifont_file_find_glyph(void* self, uint32_t font_index, uint32_t codepoint)
{
    return self ? as_font_file(self)->find_glyph(font_index, codepoint) : LUNA_FONT_INVALID_GLYPH;
}

LUNA_FONT_C_API float luna_font_ifont_file_scale_for_pixel_height(void* self, uint32_t font_index, float pixels)
{
    return self ? as_font_file(self)->scale_for_pixel_height(font_index, pixels) : 0.0f;
}

LUNA_FONT_C_API void luna_font_ifont_file_get_vmetrics(void* self, uint32_t font_index, LunaFontVMetrics* out_metrics)
{
    if (!self || !out_metrics)
    {
        return;
    }
    as_font_file(self)->get_vmetrics(font_index, &out_metrics->ascent, &out_metrics->descent, &out_metrics->line_gap);
}

LUNA_FONT_C_API void luna_font_ifont_file_get_glyph_hmetrics(void* self, uint32_t font_index, int32_t glyph, LunaFontGlyphHMetrics* out_metrics)
{
    if (!self || !out_metrics)
    {
        return;
    }
    as_font_file(self)->get_glyph_hmetrics(font_index, glyph, &out_metrics->advance_width, &out_metrics->left_side_bearing);
}

LUNA_FONT_C_API int32_t luna_font_ifont_file_get_kern_advance(void* self, uint32_t font_index, int32_t glyph1, int32_t glyph2)
{
    return self ? as_font_file(self)->get_kern_advance(font_index, glyph1, glyph2) : 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_shape(void* self, uint32_t font_index, int32_t glyph, int16_t* out_commands, uint64_t capacity, uint64_t* out_count)
{
    if (!self || !out_count || (!out_commands && capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::i16> commands;
    as_font_file(self)->get_glyph_shape(font_index, glyph, commands);
    *out_count = static_cast<uint64_t>(commands.size());
    if (capacity < *out_count)
    {
        return from_errcode(Luna::BasicError::insufficient_user_buffer());
    }
    if (!commands.empty())
    {
        std::memcpy(out_commands, commands.data(), commands.size() * sizeof(Luna::i16));
    }
    return 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_bounding_box(void* self, uint32_t font_index, int32_t glyph, LunaFontRectI* out_rect)
{
    if (!self || !out_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_rect = from_rect(as_font_file(self)->get_glyph_bounding_box(font_index, glyph));
    return 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_bitmap_box(void* self, uint32_t font_index, int32_t glyph, float scale_x, float scale_y, float shift_x, float shift_y, LunaFontRectI* out_rect)
{
    if (!self || !out_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_rect = from_rect(as_font_file(self)->get_glyph_bitmap_box(font_index, glyph, scale_x, scale_y, shift_x, shift_y));
    return 0;
}

LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_render_glyph_bitmap(void* self, uint32_t font_index, int32_t glyph, void* output, int32_t out_w, int32_t out_h, int32_t out_row_pitch, float scale_x, float scale_y, float shift_x, float shift_y)
{
    if (!self || (!output && out_w > 0 && out_h > 0))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    as_font_file(self)->render_glyph_bitmap(font_index, glyph, output, out_w, out_h, out_row_pitch, scale_x, scale_y, shift_x, shift_y);
    return 0;
}
}
