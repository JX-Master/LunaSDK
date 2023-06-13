/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Cluster.hpp
* @author JXMaster
* @date 2023/1/4
*/
#pragma once
#include <Luna/Runtime/Functional.hpp>
#include <Luna/Runtime/Span.hpp>

#ifndef LUNA_ECS_API
#define LUNA_ECS_API
#endif

namespace Luna
{
	namespace ECS
	{
		//! The entity ID is represented by a 64-bit integer that can be trivially constructed and copied.
		struct entity_id_t
		{
			union
			{
				struct
				{
					u32 index;
					u32 generation;
				};
				u64 value;
			};
			constexpr entity_id_t(u64 value = 0) : value(value) {}
			constexpr bool operator==(const entity_id_t& rhs) const { return value == rhs.value; }
			constexpr bool operator!=(const entity_id_t& rhs) const { return value != rhs.value; }
			constexpr bool operator>(const entity_id_t& rhs) const { return value > rhs.value; }
			constexpr bool operator<(const entity_id_t& rhs) const { return value < rhs.value; }
			constexpr bool operator>=(const entity_id_t& rhs) const { return value >= rhs.value; }
			constexpr bool operator<=(const entity_id_t& rhs) const { return value <= rhs.value; }
			constexpr operator bool() { return value != 0; }
		};

		//! The entity id zero always represents one invalid entity (null reference).
		constexpr entity_id_t NULL_ENTITY(0);

		//! Represents one cluster that stores entities of one particular combination of components and tags.
		//! Every world is composited by multiple clusters, every entity will only belong to one cluster.
		struct Cluster;

		//! Describes the entity address. The address of the entity will change when structural changes 
		//! are performed to the world.
		struct EntityAddress
		{
			//! The cluster that the entity belongs to. One cluster records one array of entities with the 
			//! same components and tags.
			Cluster* cluster;
			//! The index of the entity in the archetype array.
			usize index;
		};

		//! Gets the component types of the entity cluster.
		//! The returned span is valid so long as the cluster is valid.
		LUNA_ECS_API Span<const typeinfo_t> get_cluster_components(Cluster* cluster);
		//! Gets the tags of the entity cluster.
		//! The returned span is valid so long as the cluster is valid.
		LUNA_ECS_API Span<const entity_id_t> get_cluster_tags(Cluster* cluster);

		LUNA_ECS_API void* get_cluster_components_data(Cluster* cluster, typeinfo_t component_type);
		template <typename _Ty>
		inline _Ty* get_cluster_components_data(Cluster* cluster)
		{
			return static_cast<_Ty*>(get_cluster_components_data(cluster, typeof<_Ty>()));
		}
		//! Gets the cluster data array.
		LUNA_ECS_API void** get_cluster_components_data_array(Cluster* cluster);
		//!	Gets the entities ID array of the cluster.
		LUNA_ECS_API Span<const entity_id_t> get_cluster_entities(Cluster* cluster);
	}

	template<>
	struct hash<ECS::entity_id_t>
	{
		usize operator()(ECS::entity_id_t val)
		{
			return (usize)val.value;
		}
	};
}