/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeAtlas.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include "../VG.hpp"
#include <Luna/Runtime/TSAssert.hpp>
namespace Luna
{
	namespace VG
	{
		struct ShapeAtlas : IShapeAtlas
		{
			lustruct("VG::ShapeAtlas", "{CDEDAC79-848D-49EC-8E32-6AFDF19BE4D8}");
			luiimpl();
			lutsassert_lock();

			Vector<f32> m_commands;
			Vector<ShapeDesc> m_shapes;

			Ref<RHI::IBuffer> m_buffer_resource;
			usize m_buffer_resource_capacity;
			bool m_buffer_resource_dirty;

			ShapeAtlas() :
				m_buffer_resource_capacity(0),
				m_buffer_resource_dirty(false) {}

			RV recreate_buffer();
			
			void clear()
			{
				lutsassert();
				m_commands.clear();
				m_shapes.clear();
				m_buffer_resource_dirty = false;
			}
			const f32* get_command_buffer_data()
			{
				lutsassert();
				return m_commands.data();
			}
			usize get_command_buffer_size()
			{
				lutsassert();
				return m_commands.size();
			}
			usize add_shape(Span<const f32> commands, const RectF* bounding_rect);
			usize add_shapes(const f32* commands, Span<ShapeDesc> shapes);
			usize copy_shapes(IShapeAtlas* src, usize start_shape_index, usize num_shapes);
			void remove_shapes(usize start_shape_index, usize num_shapes);
			usize count_shapes()
			{
				lutsassert();
				return m_shapes.size();
			}
			void get_shape(usize index, usize* data_offset, usize* data_size, RectF* bounding_rect)
			{
				lutsassert();
				lucheck(index < m_shapes.size());
				auto& desc = m_shapes[index];
				if (data_offset) *data_offset = desc.command_offset;
				if (data_size) *data_size = desc.num_commands;
				if (bounding_rect) *bounding_rect = desc.bounding_rect;
			}
			R<RHI::IBuffer*> get_shape_resource();
			usize get_shape_resource_size()
			{
				lutsassert();
				lutry
				{
					if (m_buffer_resource_dirty)
					{
						luexp(recreate_buffer());
					}
				}
				lucatch
				{
					return 0;
				}
				return m_buffer_resource_capacity;
			}
		};
	}
}