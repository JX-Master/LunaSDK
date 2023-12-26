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
		static Float2 circle_get_point(const Float2& center, f32 radius, f32 angle)
		{
			angle = angle * PI / 180.0f;
			Float2 sc;
			sc.y = sinf(angle);
			sc.x = cosf(angle);
			return center + radius * sc;
		}
		static void compute_bounding_rect(Span<const f32> commands, RectF* bounding_rect)
		{
			if (commands.size() < 3)
			{
				*bounding_rect = RectF(0, 0, 0, 0);
			}
			else
			{
				Float2 min_point = Float2(commands[1], commands[2]);
				Float2 max_point = min_point;
				Float2 last_point = Float2(0.0f);
				usize i = 0;
				while (i < commands.size())
				{
					if (commands[i] == COMMAND_MOVE_TO)
					{
						Float2 p1 = Float2(commands[i + 1], commands[i + 2]);
						min_point = min(min_point, p1);
						max_point = max(max_point, p1);
						last_point = p1;
						i += 3;
					}
					else if (commands[i] == COMMAND_LINE_TO)
					{
						Float2 p1 = Float2(commands[i + 1], commands[i + 2]);
						min_point = min(min_point, p1);
						max_point = max(max_point, p1);
						last_point = p1;
						i += 3;
					}
					else if (commands[i] == COMMAND_CURVE_TO)
					{
						Float2 p1 = Float2(commands[i + 1], commands[i + 2]);
						Float2 p2 = Float2(commands[i + 3], commands[i + 4]);
						min_point = min(min_point, p1);
						min_point = min(min_point, p2);
						max_point = max(max_point, p1);
						max_point = max(max_point, p2);
						last_point = p2;
						i += 5;
					}
					else if (commands[i] >= COMMAND_CIRCLE_Q1 && commands[i] <= COMMAND_CIRCLE_Q4)
					{
						f32 radius = commands[i + 1];
						f32 begin = commands[i + 2];
						f32 end = commands[i + 3];
						Float2 center = circle_get_point(last_point, radius, 180.0f + begin);
						Float2 p2 = circle_get_point(center, radius, end);
						min_point = min(min_point, p2);
						max_point = max(max_point, p2);
						last_point = p2;
						i += 4;
					}
				}
				*bounding_rect = RectF(min_point.x, min_point.y, max_point.x - min_point.x, max_point.y - min_point.y);
			}
		}
		usize FontAtlas::add_shape(Span<const f32> points, const RectF* bounding_rect)
		{
			lutsassert();
			usize begin = m_shape_points.size();
			m_shape_points.insert(m_shape_points.end(), points);
			ShapeDesc desc;
			desc.first_shape_point = begin;
			desc.num_shape_points = points.size();
			if (bounding_rect)
			{
				desc.bounding_rect = *bounding_rect;
			}
			else
			{
				compute_bounding_rect(points, &desc.bounding_rect);
			}
			usize r = m_shapes.size();
			m_shapes.push_back(desc);
			m_shape_buffer_dirty = true;
			return r;
		}
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
			usize index = add_shape({ points, 15 }, &rect);
			GlyphData data;
			data.m_glyph = Font::INVALID_GLYPH;
			data.m_shape_index = index;
			data.m_advance_width = 5;
			data.m_left_side_bearing = 0;
			m_shape_map.insert(make_pair(0, data));
		}
		bool FontAtlas::load_glyph(u32 codepoint)
		{
			auto glyph = m_font->find_glyph(m_font_index, codepoint);
			if (glyph == Font::INVALID_GLYPH)
			{
				return false;
			}
			auto font_shape = m_font->get_glyph_shape(m_font_index, glyph);
			usize i = 0;
			Vector<f32> font_data;
			while (i < font_shape.size())
			{
				auto command = font_shape[i];
				switch (command)
				{
				case Font::COMMAND_MOVE_TO:
				{
					f32 p0 = f32(font_shape[i + 1]);
					f32 p1 = f32(font_shape[i + 2]);
					font_data.insert(font_data.end(), { VG::COMMAND_MOVE_TO,
						p0, p1 });
					i += 3;
				}
				break;
				case Font::COMMAND_LINE_TO:
				{
					f32 p0 = f32(font_shape[i + 1]);
					f32 p1 = f32(font_shape[i + 2]);
					font_data.insert(font_data.end(), { VG::COMMAND_LINE_TO,
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
					font_data.insert(font_data.end(), { VG::COMMAND_CURVE_TO,
						p0, p1, p2, p3 });
					i += 5;
				}
				break;
				default: lupanic();
				}
			}
			RectI rect = m_font->get_glyph_bounding_box(m_font_index, glyph);
            RectF rectf = RectF((f32)rect.offset_x, (f32)rect.offset_y, (f32)rect.width, (f32)rect.height);
			usize shape_index = add_shape({ font_data.data(), font_data.size() }, &rectf);
			GlyphData data;
			data.m_shape_index = shape_index;
			data.m_glyph = glyph;
			m_font->get_glyph_hmetrics(m_font_index, glyph, &data.m_advance_width, &data.m_left_side_bearing);
			m_shape_map.insert(make_pair(codepoint, data));
			return true;
		}
		usize FontAtlas::get_glyph_shape_index(u32 codepoint)
		{
			auto iter = m_shape_map.find(codepoint);
			if (iter == m_shape_map.end())
			{
				auto loaded = load_glyph(codepoint);
				if (loaded) iter = m_shape_map.find(codepoint);
				else iter = m_shape_map.find(0);
			}
			return iter->second.m_shape_index;
		}
		RV FontAtlas::recreate_buffer()
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
		void FontAtlas::get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing)
		{
			auto iter = m_shape_map.find(codepoint);
			if (iter == m_shape_map.end())
			{
				auto loaded = load_glyph(codepoint);
				if (loaded) iter = m_shape_map.find(codepoint);
				else iter = m_shape_map.find(0);
			}
			if (advance_width) *advance_width = iter->second.m_advance_width;
			if (left_side_bearing) *left_side_bearing = iter->second.m_left_side_bearing;
		}
		i32 FontAtlas::get_kern_advance(u32 ch1, u32 ch2)
		{
			auto iter1 = m_shape_map.find(ch1);
			if (iter1 == m_shape_map.end())
			{
				auto loaded = load_glyph(ch1);
				if (loaded)
				{
					iter1 = m_shape_map.find(ch1);
				}
			}
			auto iter2 = m_shape_map.find(ch2);
			if (iter2 == m_shape_map.end())
			{
				auto loaded = load_glyph(ch2);
				if (loaded)
				{
					iter2 = m_shape_map.find(ch2);
				}
			}
			if (iter1 != m_shape_map.end() && iter2 != m_shape_map.end())
			{
				return m_font->get_kern_advance(m_font_index, iter1->second.m_glyph, iter2->second.m_glyph);
			}
			return 0;
		}
		R<RHI::IBuffer*> FontAtlas::get_shape_buffer()
		{
			lutsassert();
			lutry
			{
				if (m_shape_buffer_dirty)
				{
					luexp(recreate_buffer());
				}
			}
			lucatchret;
			return m_shape_buffer.get();
		}
		Span<const f32> FontAtlas::get_shape_points()
		{
			lutsassert();
			return { m_shape_points.data(), m_shape_points.size() };
		}
		void FontAtlas::get_glyph(usize codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect)
		{
			usize shape_index = get_glyph_shape_index(codepoint);
			auto& desc = m_shapes[shape_index];
			if (first_shape_point) *first_shape_point = desc.first_shape_point;
			if (num_shape_points) *num_shape_points = desc.num_shape_points;
			if (bounding_rect) *bounding_rect = desc.bounding_rect;
		}
		LUNA_VG_API Ref<IFontAtlas> new_font_atlas(Font::IFontFile* font, u32 index, RHI::IDevice* device)
		{
			Ref<FontAtlas> ret = new_object<FontAtlas>();
			ret->m_device = device ? device : RHI::get_main_device();
			ret->set_font(font, index);
			return ret;
		}
	}
}
