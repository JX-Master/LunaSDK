/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.cpp
* @author JXMaster
* @date 2022/4/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "FontAtlas.hpp"
#include "../Shapes.hpp"
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/RHI/RHI.hpp>

namespace Luna
{
    namespace VG
    {
        void FontAtlas::load_default_glyph()
        {
            f32 points[] = {
                COMMAND_MOVE_TO, 0.0f, 0.0f,
                COMMAND_LINE_TO, 0.0f, 10.0f,
                COMMAND_LINE_TO, 5.0f, 10.0f,
                COMMAND_LINE_TO, 5.0f, 0.0f,
                COMMAND_LINE_TO, 0.0f, 0.0f,
            };
            RectF rect = RectF(0.0f, 0.0f, 5.0f, 10.0f);
            u32 first_command = (u32)m_shape_points.size();
            m_shape_points.insert(m_shape_points.end(), Span<f32>(points, 5));
            m_default_glyph.m_first_command = first_command;
            m_default_glyph.m_num_commands = 5;
            m_default_glyph.m_bounding_rect = rect;
        }
        RV FontAtlas::recreate_buffer(RHI::IDevice* device)
        {
            lutry
            {
                using namespace RHI;
                if (m_shape_buffer_capacity < m_shape_points.size())
                {
                    u64 shape_buffer_size = m_shape_points.size() * sizeof(f32);
                    luset(m_shape_buffer, get_main_device()->new_buffer(MemoryType::upload, BufferDesc(
                        BufferUsageFlag::read_buffer, shape_buffer_size)));
                    m_shape_buffer_capacity = m_shape_points.size();
                }
                void* shape_data = nullptr;
                luexp(m_shape_buffer->map(0, 0, &shape_data));
                memcpy(shape_data, m_shape_points.data(), m_shape_points.size() * sizeof(f32));
                m_shape_buffer->unmap(0, m_shape_points.size() * sizeof(f32));
                m_shape_buffer_dirty = false;
            }
            lucatchret;
            return ok;
        }
        R<RHI::IBuffer*> FontAtlas::get_shape_buffer(RHI::IDevice* device)
        {
            lutsassert();
            lutry
            {
                if (m_shape_buffer_dirty)
                {
                    luexp(recreate_buffer(device));
                }
            }
            lucatchret;
            return m_shape_buffer.get();
        }
        void FontAtlas::get_glyph(u32 codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect)
        {
            auto iter = m_current_font_glyphs->m_glyphs.find(codepoint);
            if(iter != m_current_font_glyphs->m_glyphs.end())
            {
                if (first_shape_point) *first_shape_point = iter->second.m_first_command;
                if (num_shape_points) *num_shape_points = iter->second.m_num_commands;
                if (bounding_rect) *bounding_rect = iter->second.m_bounding_rect;
                return;
            }
            // try to load glyph from font file.
            u32 first_command = (u32)m_shape_points.size();
            RectF rect;
            auto r = get_font_glyph_shape(m_current_font, m_current_font_index, codepoint, &m_shape_points, &rect);
            if(failed(r))
            {
                if (first_shape_point) *first_shape_point = m_default_glyph.m_first_command;
                if (num_shape_points) *num_shape_points = m_default_glyph.m_num_commands;
                if (bounding_rect) *bounding_rect = m_default_glyph.m_bounding_rect;
                return;
            }
            m_shape_buffer_dirty = true;
            u32 num_commands = (u32)m_shape_points.size() - first_command;
            GlyphDesc desc;
            desc.m_first_command = first_command;
            desc.m_num_commands = num_commands;
            desc.m_bounding_rect = rect;
            m_current_font_glyphs->m_glyphs.insert(make_pair(codepoint, desc));
            if (first_shape_point) *first_shape_point = first_command;
            if (num_shape_points) *num_shape_points = num_commands;
            if (bounding_rect) *bounding_rect = rect;
        }
        LUNA_VG_API Ref<IFontAtlas> new_font_atlas()
        {
            return new_object<FontAtlas>();
        }
        LUNA_VG_API RV get_font_glyph_shape(Font::IFontFile* font, u32 font_index, u32 codepoint, Vector<f32>* out_shape_points, RectF* out_bounding_rect)
        {
            auto glyph = font->find_glyph(font_index, codepoint);
            if(glyph == Font::INVALID_GLYPH)
            {
                return BasicError::not_found();
            }
            if(out_shape_points)
            {
                Vector<i16> font_shape;
                font->get_glyph_shape(font_index, glyph, font_shape);
                usize i = 0;
                while(i < font_shape.size())
                {
                    auto command = font_shape[i];
                    switch (command)
                    {
                    case Font::COMMAND_MOVE_TO:
                    {
                        f32 p0 = f32(font_shape[i + 1]);
                        f32 p1 = f32(font_shape[i + 2]);
                        out_shape_points->insert(out_shape_points->end(), { VG::COMMAND_MOVE_TO,
                            p0, p1 });
                        i += 3;
                    }
                    break;
                    case Font::COMMAND_LINE_TO:
                    {
                        f32 p0 = f32(font_shape[i + 1]);
                        f32 p1 = f32(font_shape[i + 2]);
                        out_shape_points->insert(out_shape_points->end(), { VG::COMMAND_LINE_TO,
                            p0, p1 });
                        i += 3;
                    }
                    break;
                    case Font::COMMAND_CURVE_TO:
                    {
                        f32 p0 = f32(font_shape[i + 1]);
                        f32 p1 = f32(font_shape[i + 2]);
                        f32 p2 = f32(font_shape[i + 3]);
                        f32 p3 = f32(font_shape[i + 4]);
                        out_shape_points->insert(out_shape_points->end(), { VG::COMMAND_CURVE_TO,
                            p0, p1, p2, p3 });
                        i += 5;
                    }
                    break;
                    default: lupanic();
                    }
                }
            }
            if(out_bounding_rect)
            {
                RectI rect = font->get_glyph_bounding_box(font_index, glyph);
                *out_bounding_rect = RectF((f32)rect.offset_x, (f32)rect.offset_y, (f32)rect.width, (f32)rect.height);
            }
            return ok;
        }
    }
}
