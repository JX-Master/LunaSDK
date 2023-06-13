/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Cluster.hpp
* @author JXMaster
* @date 2022/9/6
*/
#pragma once
#include "../ECS.hpp"
#include <Luna/Runtime/RingDeque.hpp>
namespace Luna
{
	namespace ECS
	{
		struct World;

		// corresponding to `cluster_t`, managed by World.
		struct Cluster
		{
			Span<typeinfo_t> m_component_types;
			Span<entity_id_t> m_tags;
			//! Entities that is contained by this arche type.
			Vector<entity_id_t> m_entities;
			//! Array of pointers to component arrays.
			//! Using the same order of `m_components` in archetype.
			void** m_components;

			usize m_size;

			Cluster() :
				m_components(nullptr),
				m_size(0) {}

			~Cluster()
			{
				for (usize i = 0; i < m_component_types.size(); ++i)
				{
					if (m_components[i])
					{
						typeinfo_t type = m_component_types[i];
						if (!is_type_trivially_destructable(type))
						{
							usize strip = get_type_size(type);
							for (usize i = 0; i < m_entities.size(); ++i)
							{
								if (m_entities[i] != NULL_ENTITY)
								{
									void* data = (void*)((usize)(m_components[i]) + strip * i);
									destruct_type(type, data);
								}
							}
						}
						memfree(m_components[i], get_type_alignment(type));
						m_components[i] = nullptr;
					}
				}
				if (m_components)
				{
					memfree(m_components);
				}
				if (m_component_types.data())
				{
					memfree(m_component_types.data());
				}
				if (m_tags.data())
				{
					memfree(m_tags.data());
				}
			}

			void expand_buffer();
			usize allocate_entry();
			void free_entry(World* world, usize index);
			void relocate_entity(usize dst, usize src);
			void free_all_entities();
		};
	}
}