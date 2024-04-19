/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.hpp
* @author JXMaster
* @date 2022/4/27
*/
#pragma once
#include "../FontAtlas.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/RHI/Device.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace VG
    {
        struct FontAtlas : IFontAtlas
        {
            lustruct("VG::FontAtlas", "{E25DC74A-20B6-4207-B0C1-3E4F8CDB45A2}");
            luiimpl();
            lutsassert_lock();

            struct GlyphDesc
            {
                u32 m_first_command;
                u32 m_num_commands;
                RectF m_bounding_rect;
            };

            struct FontGlyphs
            {
                HashMap<u32, GlyphDesc> m_glyphs;
            };

            Vector<f32> m_shape_points;
            HashMap<Pair<Font::IFontFile*, u32>, UniquePtr<FontGlyphs>> m_shapes;

            Font::IFontFile* m_current_font;
            u32 m_current_font_index;
            FontGlyphs* m_current_font_glyphs;

            GlyphDesc m_default_glyph;

            Ref<RHI::IBuffer> m_shape_buffer;
            usize m_shape_buffer_capacity;
            bool m_shape_buffer_dirty;

            FontAtlas() :
                m_shape_buffer_capacity(0),
                m_shape_buffer_dirty(false) 
            {
                clear();
            }

            usize add_shape(Span<const f32> points, const RectF* bounding_rect);

            void load_default_glyph();

            RV recreate_buffer(RHI::IDevice* device);

            virtual void clear() override
            {
                lutsassert();
                m_shape_points.clear();
                m_shapes.clear();
                m_shape_buffer_dirty = false;
                m_current_font = nullptr;
                m_current_font_index = 0;
                m_current_font_glyphs = nullptr;
                load_default_glyph();
            }
            virtual Font::IFontFile* get_font(u32* index) override
            {
                lutsassert();
                if (index) *index = m_current_font_index;
                return m_current_font;
            }
            virtual void set_font(Font::IFontFile* font, u32 index) override
            {
                lutsassert();
                m_current_font = font;
                m_current_font_index = index;
                auto iter = m_shapes.find(make_pair(font, index));
                if(iter == m_shapes.end())
                {
                    iter = m_shapes.insert(make_pair(make_pair(font, index), UniquePtr<FontGlyphs>(memnew<FontGlyphs>()))).first;
                }
                m_current_font_glyphs = iter->second.get();
            }
            virtual R<RHI::IBuffer*> get_shape_buffer(RHI::IDevice* device) override;
            virtual Span<const f32> get_shape_points() override
            {
                lutsassert();
                return { m_shape_points.data(), m_shape_points.size() };
            }
            virtual void get_glyph(u32 codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect) override;
        };
    }

    template<>
    struct hash<Pair<Font::IFontFile*, u32>>
    {
        usize operator()(const Pair<Font::IFontFile*, u32>& key)
        {
            return hash<Font::IFontFile*>()(key.first) ^ hash<u32>()(key.second);
        }
    };
}