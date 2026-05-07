#include "VG.h"

#include <Luna/VG/FontAtlas.hpp>
#include <Luna/VG/ShapeBuffer.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/RHI/Device.hpp>

#include <memory>

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

template <typename T>
T* checked_interface(void* self)
{
    return reinterpret_cast<T*>(self);
}

template <typename T>
void fill_object_handle(luna_handle_t object, T* interface_ptr, luna_handle_t* out_object, void** out_interface)
{
    *out_object = object;
    *out_interface = interface_ptr;
}

LunaVGFloat2 to_c(const Luna::Float2U& value)
{
    return LunaVGFloat2{value.x, value.y};
}

Luna::Float2U from_c(const LunaVGFloat2& value)
{
    return Luna::Float2U(value.x, value.y);
}

LunaVGFloat4 to_c(const Luna::Float4U& value)
{
    return LunaVGFloat4{value.x, value.y, value.z, value.w};
}

Luna::Float4U from_c(const LunaVGFloat4& value)
{
    return Luna::Float4U(value.x, value.y, value.z, value.w);
}

LunaVGRectF to_c(const Luna::RectF& value)
{
    return LunaVGRectF{value.offset_x, value.offset_y, value.width, value.height};
}

Luna::RectF from_c(const LunaVGRectF& value)
{
    return Luna::RectF(value.offset_x, value.offset_y, value.width, value.height);
}

LunaVGMatrix4x4 to_c(const Luna::Float4x4U& value)
{
    LunaVGMatrix4x4 result{};
    for (int i = 0; i < 4; ++i)
    {
        result.rows[i] = to_c(value.r[i]);
    }
    return result;
}

Luna::Float4x4U from_c(const LunaVGMatrix4x4& value)
{
    Luna::Float4x4U result;
    for (int i = 0; i < 4; ++i)
    {
        result.r[i] = from_c(value.rows[i]);
    }
    return result;
}

LunaVGVertex to_c(const Luna::VG::Vertex& value)
{
    LunaVGVertex result{};
    result.position = to_c(value.position);
    result.shapecoord = to_c(value.shapecoord);
    result.texcoord = to_c(value.texcoord);
    result.begin_command = value.begin_command;
    result.num_commands = value.num_commands;
    result.color = to_c(value.color);
    return result;
}

Luna::VG::Vertex from_c(const LunaVGVertex& value)
{
    Luna::VG::Vertex result{};
    result.position = from_c(value.position);
    result.shapecoord = from_c(value.shapecoord);
    result.texcoord = from_c(value.texcoord);
    result.begin_command = value.begin_command;
    result.num_commands = value.num_commands;
    result.color = from_c(value.color);
    return result;
}

Luna::RHI::SamplerDesc from_c(const LunaRhiSamplerDesc& value)
{
    Luna::RHI::SamplerDesc result{};
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

LunaRhiSamplerDesc to_c(const Luna::RHI::SamplerDesc& value)
{
    LunaRhiSamplerDesc result{};
    result.min_filter = static_cast<uint32_t>(value.min_filter);
    result.mag_filter = static_cast<uint32_t>(value.mag_filter);
    result.mip_filter = static_cast<uint32_t>(value.mip_filter);
    result.address_u = static_cast<uint32_t>(value.address_u);
    result.address_v = static_cast<uint32_t>(value.address_v);
    result.address_w = static_cast<uint32_t>(value.address_w);
    result.anisotropy_enable = value.anisotropy_enable ? 1 : 0;
    result.compare_enable = value.compare_enable ? 1 : 0;
    result.compare_function = static_cast<uint32_t>(value.compare_function);
    result.border_color = static_cast<uint32_t>(value.border_color);
    result.max_anisotropy = value.max_anisotropy;
    result.min_lod = value.min_lod;
    result.max_lod = value.max_lod;
    return result;
}

Luna::VG::TextAlignment from_c(uint8_t alignment)
{
    switch (alignment)
    {
    case LUNA_VG_TEXT_ALIGNMENT_BEGIN: return Luna::VG::TextAlignment::begin;
    case LUNA_VG_TEXT_ALIGNMENT_CENTER: return Luna::VG::TextAlignment::center;
    case LUNA_VG_TEXT_ALIGNMENT_END: return Luna::VG::TextAlignment::end;
    default: return Luna::VG::TextAlignment::begin;
    }
}

Luna::VG::TextArrangeSection from_c(const LunaVGTextArrangeSection& value)
{
    Luna::VG::TextArrangeSection result{};
    result.font_file = checked_interface<Luna::Font::IFontFile>(value.font_file);
    result.num_chars = static_cast<Luna::usize>(value.num_chars);
    result.font_index = value.font_index;
    result.color = from_c(value.color);
    result.font_size = value.font_size;
    result.char_span = value.char_span;
    result.line_span = value.line_span;
    return result;
}

LunaVGTextLineArrangeResult to_c(const Luna::VG::TextLineArrangeResult& value)
{
    LunaVGTextLineArrangeResult result{};
    result.bounding_rect = to_c(value.bounding_rect);
    result.baseline_offset = value.baseline_offset;
    result.ascent = value.ascent;
    result.decent = value.decent;
    result.line_gap = value.line_gap;
    return result;
}

LunaVGTextGlyphArrangeResult to_c(const Luna::VG::TextGlyphArrangeResult& value)
{
    LunaVGTextGlyphArrangeResult result{};
    result.bounding_rect = to_c(value.bounding_rect);
    result.origin_offset = value.origin_offset;
    result.advance_length = value.advance_length;
    result.character = value.character;
    result.index = value.index;
    return result;
}

LunaRhiBufferHandle make_buffer_handle(Luna::RHI::IBuffer* buffer)
{
    LunaRhiBufferHandle handle{};
    if (buffer)
    {
        Luna::object_t object = buffer->get_object();
        Luna::object_retain(object);
        handle.object = object;
        handle.ibuffer = buffer;
    }
    return handle;
}

LunaRhiTextureHandle make_texture_handle(Luna::RHI::ITexture* texture)
{
    LunaRhiTextureHandle handle{};
    if (texture)
    {
        Luna::object_t object = texture->get_object();
        Luna::object_retain(object);
        handle.object = object;
        handle.itexture = texture;
    }
    return handle;
}

LunaRhiDeviceHandle make_device_handle(Luna::RHI::IDevice* device)
{
    LunaRhiDeviceHandle handle{};
    if (device)
    {
        Luna::object_t object = device->get_object();
        Luna::object_retain(object);
        handle.object = object;
        handle.idevice = device;
    }
    return handle;
}

LunaVGShapeBufferHandle make_shape_buffer_handle(Luna::VG::IShapeBuffer* shape_buffer)
{
    LunaVGShapeBufferHandle handle{};
    if (shape_buffer)
    {
        Luna::object_t object = shape_buffer->get_object();
        Luna::object_retain(object);
        handle.object = object;
        handle.ishape_buffer = shape_buffer;
    }
    return handle;
}

LunaVGShapeRendererHandle make_shape_renderer_handle(Luna::VG::IShapeRenderer* shape_renderer)
{
    LunaVGShapeRendererHandle handle{};
    if (shape_renderer)
    {
        Luna::object_t object = shape_renderer->get_object();
        Luna::object_retain(object);
        handle.object = object;
        handle.ishape_renderer = shape_renderer;
    }
    return handle;
}

Luna::VG::ShapeDrawCall from_c(const LunaVGShapeDrawCall& value)
{
    Luna::VG::ShapeDrawCall result{};
    if (value.shape_buffer.object)
    {
        Luna::object_retain(value.shape_buffer.object);
        result.shape_buffer.attach(value.shape_buffer.object);
    }
    if (value.texture.object)
    {
        Luna::object_retain(value.texture.object);
        result.texture.attach(value.texture.object);
    }
    result.sampler = from_c(value.sampler);
    result.clip_rect = from_c(value.clip_rect);
    result.base_index = value.base_index;
    result.num_indices = value.num_indices;
    result.transform = from_c(value.transform);
    return result;
}

struct TextArrangeResultHolder
{
    Luna::VG::TextArrangeResult result;
};
}

extern "C"
{
LUNA_VG_C_API luna_errcode_t luna_vg_init_module(void)
{
    Luna::Module* module = Luna::module_vg();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    return from_result(Luna::init_module(module));
}

LUNA_VG_C_API luna_errcode_t luna_vg_arrange_text(
    const char* text,
    uint64_t text_len,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    const LunaVGRectF* bounding_rect,
    uint8_t vertical_alignment,
    uint8_t horizontal_alignment,
    luna_vg_text_arrange_result_t* out_result)
{
    if (!text || !sections || !bounding_rect || !out_result)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::TextArrangeSection> native_sections;
    native_sections.reserve(num_sections);
    for (uint64_t i = 0; i < num_sections; ++i)
    {
        native_sections.push_back(from_c(sections[i]));
    }
    auto holder = std::make_unique<TextArrangeResultHolder>();
    holder->result = Luna::VG::arrange_text(
        text,
        text_len == UINT64_MAX ? Luna::USIZE_MAX : static_cast<Luna::usize>(text_len),
        {native_sections.data(), native_sections.size()},
        from_c(*bounding_rect),
        from_c(vertical_alignment),
        from_c(horizontal_alignment));
    *out_result = holder.release();
    return 0;
}

LUNA_VG_C_API void luna_vg_text_arrange_result_free(luna_vg_text_arrange_result_t result)
{
    delete static_cast<TextArrangeResultHolder*>(result);
}

LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_bounding_rect(
    luna_vg_text_arrange_result_t result,
    LunaVGRectF* out_bounding_rect)
{
    if (!result || !out_bounding_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_bounding_rect = to_c(static_cast<TextArrangeResultHolder*>(result)->result.bounding_rect);
    return 0;
}

LUNA_VG_C_API int32_t luna_vg_text_arrange_result_get_overflow(luna_vg_text_arrange_result_t result)
{
    return result && static_cast<TextArrangeResultHolder*>(result)->result.overflow ? 1 : 0;
}

LUNA_VG_C_API uint64_t luna_vg_text_arrange_result_get_num_lines(luna_vg_text_arrange_result_t result)
{
    return result ? static_cast<uint64_t>(static_cast<TextArrangeResultHolder*>(result)->result.lines.size()) : 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_line(
    luna_vg_text_arrange_result_t result,
    uint64_t index,
    LunaVGTextLineArrangeResult* out_line)
{
    if (!result || !out_line)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto& lines = static_cast<TextArrangeResultHolder*>(result)->result.lines;
    if (index >= lines.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }
    *out_line = to_c(lines[static_cast<Luna::usize>(index)]);
    return 0;
}

LUNA_VG_C_API uint64_t luna_vg_text_arrange_result_get_num_glyphs(
    luna_vg_text_arrange_result_t result,
    uint64_t line_index)
{
    if (!result)
    {
        return 0;
    }
    auto& lines = static_cast<TextArrangeResultHolder*>(result)->result.lines;
    if (line_index >= lines.size())
    {
        return 0;
    }
    return static_cast<uint64_t>(lines[static_cast<Luna::usize>(line_index)].glyphs.size());
}

LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_glyph(
    luna_vg_text_arrange_result_t result,
    uint64_t line_index,
    uint64_t glyph_index,
    LunaVGTextGlyphArrangeResult* out_glyph)
{
    if (!result || !out_glyph)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto& lines = static_cast<TextArrangeResultHolder*>(result)->result.lines;
    if (line_index >= lines.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }
    auto& glyphs = lines[static_cast<Luna::usize>(line_index)].glyphs;
    if (glyph_index >= glyphs.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }
    *out_glyph = to_c(glyphs[static_cast<Luna::usize>(glyph_index)]);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_generate_text_arrange_result_draw_vertices(
    luna_vg_text_arrange_result_t result,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    void* font_atlas,
    LunaVGVertex* out_vertices,
    uint64_t vertex_capacity,
    uint64_t* out_vertex_count,
    uint32_t* out_indices,
    uint64_t index_capacity,
    uint64_t* out_index_count)
{
    if (!result || !sections || !font_atlas || !out_vertex_count || !out_index_count || (!out_vertices && vertex_capacity) || (!out_indices && index_capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::TextArrangeSection> native_sections;
    native_sections.reserve(num_sections);
    for (uint64_t i = 0; i < num_sections; ++i)
    {
        native_sections.push_back(from_c(sections[i]));
    }
    Luna::Vector<Luna::VG::Vertex> vertices;
    Luna::Vector<Luna::u32> indices;
    Luna::VG::generate_text_arrange_result_draw_vertices(
        static_cast<TextArrangeResultHolder*>(result)->result,
        {native_sections.data(), native_sections.size()},
        checked_interface<Luna::VG::IFontAtlas>(font_atlas),
        vertices,
        indices);
    *out_vertex_count = static_cast<uint64_t>(vertices.size());
    *out_index_count = static_cast<uint64_t>(indices.size());
    if (vertex_capacity < *out_vertex_count || index_capacity < *out_index_count)
    {
        return from_errcode(Luna::BasicError::insufficient_user_buffer());
    }
    for (uint64_t i = 0; i < *out_vertex_count; ++i)
    {
        out_vertices[i] = to_c(vertices[static_cast<Luna::usize>(i)]);
    }
    if (!indices.empty())
    {
        memcpy(out_indices, indices.data(), indices.size() * sizeof(uint32_t));
    }
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_commit_text_arrange_result(
    luna_vg_text_arrange_result_t result,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    void* font_atlas,
    void* draw_list)
{
    if (!result || !sections || !font_atlas || !draw_list)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::TextArrangeSection> native_sections;
    native_sections.reserve(num_sections);
    for (uint64_t i = 0; i < num_sections; ++i)
    {
        native_sections.push_back(from_c(sections[i]));
    }
    Luna::VG::commit_text_arrange_result(
        static_cast<TextArrangeResultHolder*>(result)->result,
        {native_sections.data(), native_sections.size()},
        checked_interface<Luna::VG::IFontAtlas>(font_atlas),
        checked_interface<Luna::VG::IShapeDrawList>(draw_list));
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_new_shape_buffer(LunaVGShapeBufferHandle* out_shape_buffer)
{
    if (!out_shape_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_shape_buffer->object = nullptr;
    out_shape_buffer->ishape_buffer = nullptr;
    Luna::Ref<Luna::VG::IShapeBuffer> shape_buffer = Luna::VG::new_shape_buffer();
    Luna::object_t object = shape_buffer.detach();
    out_shape_buffer->object = object;
    out_shape_buffer->ishape_buffer = object_as<Luna::VG::IShapeBuffer>(object);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_get_points(
    void* self,
    float* out_points,
    uint64_t capacity,
    uint64_t* out_count)
{
    if (!self || !out_count || (!out_points && capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(self)->get_shape_points(false);
    *out_count = static_cast<uint64_t>(points.size());
    if (capacity < *out_count)
    {
        return from_errcode(Luna::BasicError::insufficient_user_buffer());
    }
    if (!points.empty())
    {
        memcpy(out_points, points.data(), points.size() * sizeof(float));
    }
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_set_points(
    void* self,
    const float* points,
    uint64_t count)
{
    if (!self || (!points && count))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto& shape_points = checked_interface<Luna::VG::IShapeBuffer>(self)->get_shape_points(true);
    shape_points.assign(points, points + count);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_build(
    void* self,
    void* device,
    LunaRhiBufferHandle* out_buffer)
{
    if (!self || !device || !out_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_buffer->object = nullptr;
    out_buffer->ibuffer = nullptr;
    auto result = checked_interface<Luna::VG::IShapeBuffer>(self)->build(checked_interface<Luna::RHI::IDevice>(device));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    auto* buffer = result.get();
    if (!buffer)
    {
        return 0;
    }
    Luna::object_t object = buffer->get_object();
    Luna::object_retain(object);
    out_buffer->object = object;
    out_buffer->ibuffer = buffer;
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_new_font_atlas(LunaVGFontAtlasHandle* out_font_atlas)
{
    if (!out_font_atlas)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_font_atlas->object = nullptr;
    out_font_atlas->ifont_atlas = nullptr;
    Luna::Ref<Luna::VG::IFontAtlas> font_atlas = Luna::VG::new_font_atlas();
    Luna::object_t object = font_atlas.detach();
    out_font_atlas->object = object;
    out_font_atlas->ifont_atlas = object_as<Luna::VG::IFontAtlas>(object);
    return 0;
}

LUNA_VG_C_API void luna_vg_font_atlas_clear(void* self)
{
    if (self)
    {
        checked_interface<Luna::VG::IFontAtlas>(self)->clear();
    }
}

LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_font(void* self, LunaFontHandle* out_font, uint32_t* out_index)
{
    if (!self || !out_font)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_font->object = nullptr;
    out_font->ifont_file = nullptr;
    uint32_t index = 0;
    auto* font = checked_interface<Luna::VG::IFontAtlas>(self)->get_font(out_index ? &index : nullptr);
    if (!font)
    {
        return 0;
    }
    auto object = font->get_object();
    Luna::object_retain(object);
    out_font->object = object;
    out_font->ifont_file = font;
    if (out_index)
    {
        *out_index = index;
    }
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_set_font(void* self, void* font_file, uint32_t index)
{
    if (!self || !font_file)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    checked_interface<Luna::VG::IFontAtlas>(self)->set_font(checked_interface<Luna::Font::IFontFile>(font_file), index);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_shape_buffer(void* self, LunaVGShapeBufferHandle* out_shape_buffer)
{
    if (!self || !out_shape_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto* shape_buffer = checked_interface<Luna::VG::IFontAtlas>(self)->get_shape_buffer();
    *out_shape_buffer = make_shape_buffer_handle(shape_buffer);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_glyph(
    void* self,
    uint32_t codepoint,
    uint64_t* out_first_shape_point,
    uint64_t* out_num_shape_points,
    LunaVGRectF* out_bounding_rect)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::usize first = 0;
    Luna::usize count = 0;
    Luna::RectF rect{};
    checked_interface<Luna::VG::IFontAtlas>(self)->get_glyph(
        codepoint,
        out_first_shape_point ? &first : nullptr,
        out_num_shape_points ? &count : nullptr,
        out_bounding_rect ? &rect : nullptr);
    if (out_first_shape_point) *out_first_shape_point = static_cast<uint64_t>(first);
    if (out_num_shape_points) *out_num_shape_points = static_cast<uint64_t>(count);
    if (out_bounding_rect) *out_bounding_rect = to_c(rect);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_get_font_glyph_shape(
    void* font_file,
    uint32_t font_index,
    uint32_t codepoint,
    float* out_shape_points,
    uint64_t capacity,
    uint64_t* out_count,
    LunaVGRectF* out_bounding_rect)
{
    if (!font_file || !out_count || (!out_shape_points && capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<float> shape_points;
    Luna::RectF rect{};
    auto result = Luna::VG::get_font_glyph_shape(
        checked_interface<Luna::Font::IFontFile>(font_file),
        font_index,
        codepoint,
        &shape_points,
        out_bounding_rect ? &rect : nullptr);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_count = static_cast<uint64_t>(shape_points.size());
    if (capacity < *out_count)
    {
        return from_errcode(Luna::BasicError::insufficient_user_buffer());
    }
    if (!shape_points.empty())
    {
        memcpy(out_shape_points, shape_points.data(), shape_points.size() * sizeof(float));
    }
    if (out_bounding_rect)
    {
        *out_bounding_rect = to_c(rect);
    }
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_new_shape_draw_list(
    void* device,
    LunaVGShapeDrawListHandle* out_draw_list)
{
    if (!out_draw_list)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_draw_list->object = nullptr;
    out_draw_list->ishape_draw_list = nullptr;
    auto native = Luna::VG::new_shape_draw_list(device ? checked_interface<Luna::RHI::IDevice>(device) : nullptr);
    Luna::object_t object = native.detach();
    out_draw_list->object = object;
    out_draw_list->ishape_draw_list = object_as<Luna::VG::IShapeDrawList>(object);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_device(void* self, LunaRhiDeviceHandle* out_device)
{
    if (!self || !out_device)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_device = make_device_handle(checked_interface<Luna::VG::IShapeDrawList>(self)->get_device());
    return 0;
}

LUNA_VG_C_API void luna_vg_shape_draw_list_reset(void* self)
{
    if (self)
    {
        checked_interface<Luna::VG::IShapeDrawList>(self)->reset();
    }
}

LUNA_VG_C_API void luna_vg_shape_draw_list_set_shape_buffer(void* self, void* shape_buffer)
{
    if (self)
    {
        checked_interface<Luna::VG::IShapeDrawList>(self)->set_shape_buffer(checked_interface<Luna::VG::IShapeBuffer>(shape_buffer));
    }
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_shape_buffer(void* self, LunaVGShapeBufferHandle* out_shape_buffer)
{
    if (!self || !out_shape_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_shape_buffer = make_shape_buffer_handle(checked_interface<Luna::VG::IShapeDrawList>(self)->get_shape_buffer());
    return 0;
}

LUNA_VG_C_API void luna_vg_shape_draw_list_set_texture(void* self, void* texture)
{
    if (self)
    {
        checked_interface<Luna::VG::IShapeDrawList>(self)->set_texture(checked_interface<Luna::RHI::ITexture>(texture));
    }
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_texture(void* self, LunaRhiTextureHandle* out_texture)
{
    if (!self || !out_texture)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_texture = make_texture_handle(checked_interface<Luna::VG::IShapeDrawList>(self)->get_texture());
    return 0;
}

LUNA_VG_C_API void luna_vg_shape_draw_list_set_sampler(void* self, const LunaRhiSamplerDesc* desc)
{
    if (self)
    {
        if (desc)
        {
            auto sampler = from_c(*desc);
            checked_interface<Luna::VG::IShapeDrawList>(self)->set_sampler(&sampler);
        }
    }
}

LUNA_VG_C_API void luna_vg_shape_draw_list_reset_sampler(void* self)
{
    if (self)
    {
        checked_interface<Luna::VG::IShapeDrawList>(self)->set_sampler(nullptr);
    }
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_sampler(void* self, LunaRhiSamplerDesc* out_sampler)
{
    if (!self || !out_sampler)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_sampler = to_c(checked_interface<Luna::VG::IShapeDrawList>(self)->get_sampler());
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_set_transform(void* self, const LunaVGMatrix4x4* transform)
{
    if (!self || !transform)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    checked_interface<Luna::VG::IShapeDrawList>(self)->set_transform(from_c(*transform));
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_transform(void* self, LunaVGMatrix4x4* out_transform)
{
    if (!self || !out_transform)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_transform = to_c(checked_interface<Luna::VG::IShapeDrawList>(self)->get_transform());
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_set_clip_rect(void* self, const LunaVGRectF* clip_rect)
{
    if (!self || !clip_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    checked_interface<Luna::VG::IShapeDrawList>(self)->set_clip_rect(from_c(*clip_rect));
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_clip_rect(void* self, LunaVGRectF* out_clip_rect)
{
    if (!self || !out_clip_rect)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_clip_rect = to_c(checked_interface<Luna::VG::IShapeDrawList>(self)->get_clip_rect());
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_draw_shape_raw(
    void* self,
    const LunaVGVertex* vertices,
    uint64_t num_vertices,
    const uint32_t* indices,
    uint64_t num_indices)
{
    if (!self || (!vertices && num_vertices) || (!indices && num_indices))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::Vertex> native_vertices;
    native_vertices.reserve(num_vertices);
    for (uint64_t i = 0; i < num_vertices; ++i)
    {
        native_vertices.push_back(from_c(vertices[i]));
    }
    checked_interface<Luna::VG::IShapeDrawList>(self)->draw_shape_raw(
        {native_vertices.data(), native_vertices.size()},
        {indices, static_cast<Luna::usize>(num_indices)});
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_draw_shape(
    void* self,
    uint32_t begin_command,
    uint32_t num_commands,
    const LunaVGFloat2* min_position,
    const LunaVGFloat2* max_position,
    const LunaVGFloat2* min_shapecoord,
    const LunaVGFloat2* max_shapecoord,
    const LunaVGFloat4* color,
    const LunaVGFloat2* min_texcoord,
    const LunaVGFloat2* max_texcoord)
{
    if (!self || !min_position || !max_position || !min_shapecoord || !max_shapecoord)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    checked_interface<Luna::VG::IShapeDrawList>(self)->draw_shape(
        begin_command,
        num_commands,
        from_c(*min_position),
        from_c(*max_position),
        from_c(*min_shapecoord),
        from_c(*max_shapecoord),
        color ? from_c(*color) : Luna::Float4U(Luna::Color::white()),
        min_texcoord ? from_c(*min_texcoord) : Luna::Float2U(0.0f),
        max_texcoord ? from_c(*max_texcoord) : Luna::Float2U(0.0f));
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_compile(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(checked_interface<Luna::VG::IShapeDrawList>(self)->compile());
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_vertex_buffer(void* self, LunaRhiBufferHandle* out_buffer)
{
    if (!self || !out_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_buffer = make_buffer_handle(checked_interface<Luna::VG::IShapeDrawList>(self)->get_vertex_buffer());
    return 0;
}

LUNA_VG_C_API uint32_t luna_vg_shape_draw_list_get_vertex_buffer_size(void* self)
{
    return self ? checked_interface<Luna::VG::IShapeDrawList>(self)->get_vertex_buffer_size() : 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_index_buffer(void* self, LunaRhiBufferHandle* out_buffer)
{
    if (!self || !out_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_buffer = make_buffer_handle(checked_interface<Luna::VG::IShapeDrawList>(self)->get_index_buffer());
    return 0;
}

LUNA_VG_C_API uint32_t luna_vg_shape_draw_list_get_index_buffer_size(void* self)
{
    return self ? checked_interface<Luna::VG::IShapeDrawList>(self)->get_index_buffer_size() : 0;
}

LUNA_VG_C_API uint64_t luna_vg_shape_draw_list_get_draw_call_count(void* self)
{
    return self ? static_cast<uint64_t>(checked_interface<Luna::VG::IShapeDrawList>(self)->get_draw_calls().size()) : 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_draw_call(
    void* self,
    uint64_t index,
    LunaVGShapeDrawCall* out_draw_call)
{
    if (!self || !out_draw_call)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto draw_calls = checked_interface<Luna::VG::IShapeDrawList>(self)->get_draw_calls();
    if (index >= draw_calls.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }
    auto& draw_call = draw_calls[static_cast<Luna::usize>(index)];
    out_draw_call->shape_buffer = make_buffer_handle(draw_call.shape_buffer ? draw_call.shape_buffer.get() : nullptr);
    out_draw_call->texture = make_texture_handle(draw_call.texture ? draw_call.texture.get() : nullptr);
    out_draw_call->sampler = to_c(draw_call.sampler);
    out_draw_call->clip_rect = to_c(draw_call.clip_rect);
    out_draw_call->base_index = draw_call.base_index;
    out_draw_call->num_indices = draw_call.num_indices;
    out_draw_call->transform = to_c(draw_call.transform);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_new_fill_shape_renderer(LunaVGShapeRendererHandle* out_renderer)
{
    if (!out_renderer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_renderer = {};
    auto native = Luna::VG::new_fill_shape_renderer();
    Luna::object_t object = native.detach();
    out_renderer->object = object;
    out_renderer->ishape_renderer = object_as<Luna::VG::IShapeRenderer>(object);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_begin(void* self, void* render_target)
{
    if (!self || !render_target)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(checked_interface<Luna::VG::IShapeRenderer>(self)->begin(checked_interface<Luna::RHI::ITexture>(render_target)));
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_draw(
    void* self,
    void* vertex_buffer,
    void* index_buffer,
    const LunaVGShapeDrawCall* draw_calls,
    uint64_t num_draw_calls)
{
    if (!self || !vertex_buffer || !index_buffer || (!draw_calls && num_draw_calls))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::ShapeDrawCall> native_draw_calls;
    native_draw_calls.reserve(num_draw_calls);
    for (uint64_t i = 0; i < num_draw_calls; ++i)
    {
        native_draw_calls.push_back(from_c(draw_calls[i]));
    }
    checked_interface<Luna::VG::IShapeRenderer>(self)->draw(
        checked_interface<Luna::RHI::IBuffer>(vertex_buffer),
        checked_interface<Luna::RHI::IBuffer>(index_buffer),
        {native_draw_calls.data(), native_draw_calls.size()},
        nullptr);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_draw_with_transform(
    void* self,
    void* vertex_buffer,
    void* index_buffer,
    const LunaVGShapeDrawCall* draw_calls,
    uint64_t num_draw_calls,
    const LunaVGMatrix4x4* transform_matrix)
{
    if (!self || !vertex_buffer || !index_buffer || !transform_matrix || (!draw_calls && num_draw_calls))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::VG::ShapeDrawCall> native_draw_calls;
    native_draw_calls.reserve(num_draw_calls);
    for (uint64_t i = 0; i < num_draw_calls; ++i)
    {
        native_draw_calls.push_back(from_c(draw_calls[i]));
    }
    auto native_transform = from_c(*transform_matrix);
    checked_interface<Luna::VG::IShapeRenderer>(self)->draw(
        checked_interface<Luna::RHI::IBuffer>(vertex_buffer),
        checked_interface<Luna::RHI::IBuffer>(index_buffer),
        {native_draw_calls.data(), native_draw_calls.size()},
        &native_transform);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_end(void* self)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(checked_interface<Luna::VG::IShapeRenderer>(self)->end());
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_submit(void* self, void* command_buffer)
{
    if (!self || !command_buffer)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    checked_interface<Luna::VG::IShapeRenderer>(self)->submit(checked_interface<Luna::RHI::ICommandBuffer>(command_buffer));
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rectangle_filled(void* shape_buffer, float min_x, float min_y, float max_x, float max_y)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_rectangle_filled(points, min_x, min_y, max_x, max_y);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rectangle_bordered(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float border_width, float border_offset)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_rectangle_bordered(points, min_x, min_y, max_x, max_y, border_width, border_offset);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rounded_rectangle_filled(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float radius)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_rounded_rectangle_filled(points, min_x, min_y, max_x, max_y, radius);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rounded_rectangle_bordered(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float radius, float border_width, float border_offset)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_rounded_rectangle_bordered(points, min_x, min_y, max_x, max_y, radius, border_width, border_offset);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_triangle_filled(void* shape_buffer, float x1, float y1, float x2, float y2, float x3, float y3)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_triangle_filled(points, x1, y1, x2, y2, x3, y3);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_triangle_bordered(void* shape_buffer, float x1, float y1, float x2, float y2, float x3, float y3, float border_width, float border_offset)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_triangle_bordered(points, x1, y1, x2, y2, x3, y3, border_width, border_offset);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_circle_filled(void* shape_buffer, float center_x, float center_y, float radius)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_circle_filled(points, center_x, center_y, radius);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_circle_bordered(void* shape_buffer, float center_x, float center_y, float radius, float border_width, float border_offset)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_circle_bordered(points, center_x, center_y, radius, border_width, border_offset);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_axis_aligned_ellipse_filled(void* shape_buffer, float center_x, float center_y, float radius_x, float radius_y)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_axis_aligned_ellipse_filled(points, center_x, center_y, radius_x, radius_y);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_axis_aligned_ellipse_bordered(void* shape_buffer, float center_x, float center_y, float radius_x, float radius_y, float border_width, float border_offset)
{
    if (!shape_buffer) return from_errcode(Luna::BasicError::bad_arguments());
    auto& points = checked_interface<Luna::VG::IShapeBuffer>(shape_buffer)->get_shape_points(true);
    Luna::VG::ShapeBuilder::add_axis_aligned_ellipse_bordered(points, center_x, center_y, radius_x, radius_y, border_width, border_offset);
    return 0;
}

LUNA_VG_C_API luna_errcode_t luna_vg_get_rect_shape_draw_vertices(
    LunaVGVertex out_vertices[4],
    uint32_t out_indices[6],
    uint32_t begin_command,
    uint32_t num_commands,
    const LunaVGFloat2* min_position,
    const LunaVGFloat2* max_position,
    const LunaVGFloat2* min_shapecoord,
    const LunaVGFloat2* max_shapecoord,
    const LunaVGFloat4* color,
    const LunaVGFloat2* min_texcoord,
    const LunaVGFloat2* max_texcoord)
{
    if (!out_vertices || !out_indices || !min_position || !max_position || !min_shapecoord || !max_shapecoord)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VG::Vertex vertices[4];
    Luna::u32 indices[6];
    Luna::VG::get_rect_shape_draw_vertices(
        vertices,
        indices,
        begin_command,
        num_commands,
        from_c(*min_position),
        from_c(*max_position),
        from_c(*min_shapecoord),
        from_c(*max_shapecoord),
        color ? from_c(*color) : Luna::Float4U(Luna::Color::white()),
        min_texcoord ? from_c(*min_texcoord) : Luna::Float2U(0.0f),
        max_texcoord ? from_c(*max_texcoord) : Luna::Float2U(0.0f));
    for (int i = 0; i < 4; ++i)
    {
        out_vertices[i] = to_c(vertices[i]);
    }
    memcpy(out_indices, indices, sizeof(indices));
    return 0;
}
}
