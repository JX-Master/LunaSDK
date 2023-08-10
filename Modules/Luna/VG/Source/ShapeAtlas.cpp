/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeAtlas.cpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeAtlas.hpp"
namespace Luna
{
	namespace VG
	{
		RV ShapeAtlas::recreate_buffer()
		{
			lutry
			{
				using namespace RHI;
				if (m_buffer_resource_capacity < m_commands.size())
				{
					u64 shape_buffer_size = max<u64>(m_commands.size() * sizeof(f32), get_main_device()->get_uniform_buffer_data_alignment());
					luset(m_buffer_resource, get_main_device()->new_buffer(MemoryType::upload, BufferDesc(
						BufferUsageFlag::read_buffer, shape_buffer_size)));
					m_buffer_resource_capacity = m_commands.size();
				}
				void* shape_data = nullptr;
				luexp(m_buffer_resource->map(0, 0, &shape_data));
				memcpy(shape_data, m_commands.data(), m_commands.size() * sizeof(f32));
				m_buffer_resource->unmap(0, m_commands.size() * sizeof(f32));
				m_buffer_resource_dirty = false;
			}
			lucatchret;
			return ok;
		}
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
		usize ShapeAtlas::add_shape(Span<const f32> commands, const RectF* bounding_rect)
		{
			lutsassert();
			usize begin = m_commands.size();
			m_commands.insert_n(m_commands.end(), commands.data(), commands.size());
			ShapeDesc desc;
			desc.command_offset = begin;
			desc.num_commands = commands.size();
			if (bounding_rect)
			{
				desc.bounding_rect = *bounding_rect;
			}
			else
			{
				compute_bounding_rect(commands, &desc.bounding_rect);
			}
			usize r = m_shapes.size();
			m_shapes.push_back(desc);
			m_buffer_resource_dirty = true;
			return r;
		}
		usize ShapeAtlas::add_shapes(const f32* commands, Span<ShapeDesc> shapes)
		{
			lutsassert();
			usize r = m_shapes.size();
			if (shapes.empty()) return r;
			usize num_commands = shapes.back().command_offset + shapes.back().num_commands;
			usize begin = m_commands.size();
			m_commands.insert_n(m_commands.end(), commands, num_commands);
			lucheck(shapes[0].command_offset == 0);
			for (usize i = 0; i < shapes.size(); ++i)
			{
				if (i < shapes.size() - 1)
				{
					lucheck(shapes[i].command_offset + shapes[i].num_commands == shapes[i + 1].command_offset);
				}
				if (shapes[i].bounding_rect == RectF(0, 0, 0, 0))
				{
					compute_bounding_rect({ commands + shapes[i].command_offset, shapes[i].num_commands }, &(shapes[i].bounding_rect));
				}
				shapes[i].command_offset += begin;
			}
			m_shapes.insert_n(m_shapes.end(), shapes.data(), shapes.size());
			m_buffer_resource_dirty = true;
			return r;
		}
		usize ShapeAtlas::copy_shapes(IShapeAtlas* src, usize start_shape_index, usize num_shapes)
		{
			lutsassert();
			ShapeAtlas* s = (ShapeAtlas*)src->get_object();
			lucheck(start_shape_index + num_shapes <= s->m_shapes.size());
			usize r = m_shapes.size();
			if (!num_shapes) return r;
			usize begin = m_commands.size();
			usize copy_begin = s->m_shapes[start_shape_index].command_offset;
			usize copy_end = s->m_shapes[start_shape_index + num_shapes - 1].command_offset + s->m_shapes[start_shape_index + num_shapes - 1].num_commands;
			m_commands.insert_n(m_commands.end(), s->m_commands.data() + copy_begin, copy_end - copy_begin);
			m_shapes.reserve(m_shapes.size() + num_shapes);
			for (usize i = 0; i < num_shapes; ++i)
			{
				ShapeDesc desc = s->m_shapes[start_shape_index + i];
				desc.command_offset = desc.command_offset - copy_begin + begin;
				m_shapes.push_back(desc);
			}
			m_buffer_resource_dirty = true;
			return r;
		}
		void ShapeAtlas::remove_shapes(usize start_shape_index, usize num_shapes)
		{
			lutsassert();
			lucheck(start_shape_index + num_shapes <= m_shapes.size());
			if (!num_shapes) return;
			usize remove_begin = m_shapes[start_shape_index].command_offset;
			usize remove_end = m_shapes[start_shape_index + num_shapes - 1].command_offset + m_shapes[start_shape_index + num_shapes - 1].num_commands;
			m_commands.erase(m_commands.begin() + remove_begin, m_commands.begin() + remove_end);
			m_shapes.erase(m_shapes.begin() + start_shape_index, m_shapes.begin() + start_shape_index + num_shapes);
			usize remove_count = remove_end - remove_begin;
			for (auto iter = m_shapes.begin() + start_shape_index; iter != m_shapes.end(); ++iter)
			{
				iter->command_offset -= remove_count;
			}
			m_buffer_resource_dirty = true;
		}
		R<RHI::IBuffer*> ShapeAtlas::get_shape_resource()
		{
			lutsassert();
			lutry
			{
				if (m_buffer_resource_dirty)
				{
					luexp(recreate_buffer());
				}
			}
			lucatchret;
			return m_buffer_resource.get();
		}
		LUNA_VG_API Ref<IShapeAtlas> new_shape_atlas()
		{
			return new_object<ShapeAtlas>();
		}
	}
}
