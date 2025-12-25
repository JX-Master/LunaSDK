/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.hpp
* @author JXMaster
* @date 2023/1/4
*/
#pragma once
#include "Cluster.hpp"
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/JobSystem/JobSystem.hpp>

namespace Luna
{
    namespace ECS
    {
        //! @interface IWorld
        //! Represents one ECS context that holds entities and their components. Every world is independent to each other.
        //! @remark The world object implements `IChangeList` as well. In such case, all calls to `IChangeList` behave like being committed immediately 
        //! before return.
        //! The world itself is not thread safe, the user must ensure that modifications to the world are synchronized.
        struct IWorld : virtual Interface
        {
            luiid("{14F85B5E-D509-40A8-A7F6-49778783418A}");

            //! Gets the cluster by components and tags.
            virtual Cluster* get_cluster(Span<const typeinfo_t> components, Span<const tag_t> tags, 
                bool create_if_not_exist = true) = 0;

            //! Deletes the specified cluster. This will deletes all entities in the cluster.
            //! @param[in] cluster The cluster to delete.
            virtual void delete_cluster(Cluster* cluster) = 0;

            //! Finds out all clusters with the specified components and tags.
            //! @param[in] components The components to find.
            //! @param[in] tags The tags to find.
            //! @param[out] out_clusters The vector that receives collected clusters.
            virtual void find_clusters(Span<const typeinfo_t> components, Span<const tag_t> tags, Vector<Cluster*>& out_clusters) = 0;

            //! Finds out all clusters that matches the specified filter.
            //! @param[in] filter The filter function to use. 
            //! This function function will be called for every cluster, if the function returns `true`, 
            //! the cluster will be collected to `out_clusters`.
            //! @param[out] out_clusters The vector that receives collected clusters.
            virtual void find_clusters(const Function<bool(Cluster* cluster)>& filter, Vector<Cluster*>& out_clusters) = 0;

            //! Creates a new entity.
            //! @param[in] target_cluster The cluster to place the new entity in.
            //! @param[out] out_address If not `nullptr`, returns the entity address of the created entity.
            //! @return Returns the entity ID of the created entity.
            virtual entity_id_t new_entity(Cluster* target_cluster, EntityAddress* out_address = nullptr) = 0;

            //! Deletes the specified entity.
            //! @param[in] entity The entity to delete.
            virtual void delete_entity(entity_id_t entity) = 0;

            //! Deletes all entities.
            virtual void delete_all_entities() = 0;

            //! Gets the entity address for the specified entity.
            virtual R<EntityAddress> get_entity_address(entity_id_t entity) = 0;

            //! Moves entity to a new cluster. This changes components and tags one entity has.
            //! @param[in] entity The entity to move.
            //! @param[in] new_cluster The target cluster to move entity to.
            //! @return Returns the new entity address after move.
            virtual R<EntityAddress> set_entity_cluster(entity_id_t entity, Cluster* new_cluster) = 0;
        };

        //! Creates one new world.
        LUNA_ECS_API Ref<IWorld> new_world();
    }

    namespace ECSError
    {
        LUNA_ECS_API errcat_t errtype();
        LUNA_ECS_API ErrCode entity_not_found();
        LUNA_ECS_API ErrCode component_not_found();
    }
}