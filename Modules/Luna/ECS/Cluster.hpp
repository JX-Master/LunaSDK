/*!
* This file is a portion of LunaSDK.
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

        using entity_id_t = u64;

        //! Tags are user-provided pointers.
        using tag_t = opaque_t;

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
        LUNA_ECS_API Span<const tag_t> get_cluster_tags(Cluster* cluster);

        //! Gets the number of eneities in the cluster.
        LUNA_ECS_API usize get_cluster_num_entities(Cluster* cluster);

        //! Gets the number of chunks in the cluster.
        LUNA_ECS_API usize get_cluster_num_chunks(Cluster* cluster);

        //! The number of entities one chunk can hold. This value is consistent for all clusters.
        inline constexpr usize CLUSTER_CHUNK_CAPACITY = 256;

        LUNA_ECS_API Span<const entity_id_t> get_cluster_entities(Cluster* cluster, usize chunk);

        LUNA_ECS_API void* get_cluster_components_data(Cluster* cluster, usize chunk, typeinfo_t component_type);
        
        template <typename _Ty>
        inline _Ty* get_cluster_components_data(Cluster* cluster, usize chunk)
        {
            return static_cast<_Ty*>(get_cluster_components_data(cluster, chunk, typeof<_Ty>()));
        }
    }
}