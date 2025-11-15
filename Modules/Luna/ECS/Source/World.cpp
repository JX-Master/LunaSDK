/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.cpp
* @author JXMaster
* @date 2022/8/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "World.hpp"
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Log.hpp>
namespace Luna
{
    namespace ECS
    {
        EntityRecord* World::get_entity_record(entity_id_t entity)
        {
            usize index = get_entity_index(entity);
            u32 generation = get_entity_generation(entity);
            if (index >= m_entities.size()) return nullptr;
            auto& ent = m_entities[index];
            if (ent.m_generation != generation || !ent.m_cluster) return nullptr;
            return &ent;
        }

        Cluster* World::get_cluster(Span<const typeinfo_t> components, Span<const tag_t> tags,
            bool create_if_not_exist)
        {
            Array<typeinfo_t> components_arr(components.data(), components.size());
            Array<tag_t> tags_arr(tags.data(), tags.size());
            sort(components_arr.begin(), components_arr.end());
            sort(tags_arr.begin(), tags_arr.end());

            ClusterType cluster_type;
            cluster_type.components = Span<const typeinfo_t>(components_arr.data(), components_arr.size());
            cluster_type.tags = Span<const tag_t>(tags_arr.data(), tags_arr.size());
            auto iter = m_clusters.find(cluster_type);
            if (iter != m_clusters.end())
            {
                return (*iter).get();
            }
            if (create_if_not_exist)
            {
                // Create new cluster.
                UniquePtr<Cluster> new_cluster{ memnew<Cluster>() };
                Cluster* ret = new_cluster.get();
                new_cluster->m_component_types = move(components_arr);
                new_cluster->m_tags = move(tags_arr);
                m_clusters.insert(move(new_cluster));
                return ret;
            }
            return nullptr;
        }
        void World::delete_cluster(Cluster* cluster)
        {
            ClusterType type;
            type.components = { cluster->m_component_types.data(), cluster->m_component_types.size() };
            type.tags = { cluster->m_tags.data(), cluster->m_tags.size() };
            auto iter = m_clusters.find(type);
            if(iter != m_clusters.end())
            {
                // Remove entities.
                for(usize i = 0; i < get_cluster_num_chunks(cluster); ++i)
                {
                    auto entities = get_cluster_entities(cluster, i);
                    for(entity_id_t id : entities)
                    {
                        auto& record = m_entities[get_entity_index(id)];
                        record.m_cluster = nullptr;
                        m_entity_id_allocator.free_id(id);
                    }
                }
                // Remove cluster directly.
                m_clusters.erase(iter);
            }
        }
        void World::find_clusters(Span<const typeinfo_t> components, Span<const tag_t> tags, Vector<Cluster*>& out_clusters)
        {
            Array<typeinfo_t> components_arr(components.data(), components.size());
            Array<tag_t> tags_arr(tags.data(), tags.size());
            sort(components_arr.begin(), components_arr.end());
            sort(tags_arr.begin(), tags_arr.end());
            auto components_span = components_arr.cspan();
            auto tags_span = tags_arr.cspan();
            find_clusters([components_span, tags_span](Cluster* cluster) -> bool {
                auto cluster_components = get_cluster_components(cluster);
                auto cluster_tags = get_cluster_tags(cluster);
                return includes(cluster_components.begin(), cluster_components.end(), components_span.begin(), components_span.end()) &&
                    includes(cluster_tags.begin(), cluster_tags.end(), tags_span.begin(), tags_span.end());
            }, out_clusters);
        }
        void World::find_clusters(const Function<bool(Cluster* cluster)>& filter, Vector<Cluster*>& out_clusters)
        {
            for(auto& cluster : m_clusters)
            {
                if(filter(cluster.get()))
                {
                    out_clusters.push_back(cluster.get());
                }
            }
        }
        entity_id_t World::new_entity(Cluster* target_cluster, EntityAddress* out_address)
        {
            entity_id_t id = m_entity_id_allocator.allocate_id();
            // add entity record.
            usize cluster_index = target_cluster->allocate_entry();
            usize entity_index = get_entity_index(id);
            while(entity_index >= m_entities.size())
            {
                m_entities.resize(m_entities.size() + 1024);
            }
            EntityRecord& ent = m_entities[entity_index];
            ent.m_generation = get_entity_generation(id);
            ent.m_cluster = target_cluster;
            ent.m_index = cluster_index;
            target_cluster->m_chunks[cluster_index / CLUSTER_CHUNK_CAPACITY].m_entities[cluster_index % CLUSTER_CHUNK_CAPACITY] = id;
            if(out_address)
            {
                out_address->cluster = target_cluster;
                out_address->index = cluster_index;
            }
            return id;
        }
        void World::delete_entity(entity_id_t entity)
        {
            auto record = get_entity_record(entity);
            if (!record)
            {
                log_warning("ECS", "World::remove_entity - Invalid entity ID: %llu, this function takes no effect.", entity);
            }
            else
            {
                // Free entity data.
                record->m_cluster->free_entry(this, record->m_index);
                // Remove record.
                record->m_cluster = nullptr;
                m_entity_id_allocator.free_id(entity);
            }
        }
        void World::delete_all_entities()
        {
            for(auto& cluster : m_clusters)
            {
                for(usize chunk_id = 0; chunk_id < cluster->m_chunks.size(); ++chunk_id)
                {
                    auto& chunk = cluster->m_chunks[chunk_id];
                    usize num_entities = min(CLUSTER_CHUNK_CAPACITY, cluster->m_size - chunk_id * CLUSTER_CHUNK_CAPACITY);
                    for(usize i = 0; i < num_entities; ++i)
                    {
                        entity_id_t entity = chunk.m_entities[i];
                        u32 entity_index = get_entity_index(entity);
                        m_entity_id_allocator.free_id(entity);
                        m_entities[entity_index].m_cluster = nullptr;
                    }
                }
            }
            m_clusters.clear();
        }
        R<EntityAddress> World::get_entity_address(entity_id_t entity)
        {
            auto record = get_entity_record(entity);
            if (!record)
            {
                return ECSError::entity_not_found();
            }
            EntityAddress r;
            r.cluster = record->m_cluster;
            r.index = record->m_index;
            return r;
        }
        usize relocate_entity(World* world, Cluster* src_cluster, usize src_index, Cluster* dst_cluster)
        {
            // allocate entity.
            usize dst_index = dst_cluster->allocate_entry();
            // move entity ID.
            Chunk& src_chunk = src_cluster->m_chunks[src_index / CLUSTER_CHUNK_CAPACITY];
            usize src_index_in_chunk = src_index % CLUSTER_CHUNK_CAPACITY;
            Chunk& dst_chunk = dst_cluster->m_chunks[dst_index / CLUSTER_CHUNK_CAPACITY];
            usize dst_index_in_chunk = dst_index % CLUSTER_CHUNK_CAPACITY;
            dst_chunk.m_entities[dst_index_in_chunk] = src_chunk.m_entities[src_index_in_chunk];

            // Initialize component data.
            usize src_component_index = 0;
            usize dst_component_index = 0;
            auto src_components = src_cluster->m_component_types.span();
            auto dst_components = dst_cluster->m_component_types.span();

            while (src_component_index < src_components.size() && dst_component_index < dst_components.size())
            {
                typeinfo_t src_component_type = src_components[src_component_index];
                typeinfo_t dst_component_type = dst_components[dst_component_index];
                if (src_component_type == dst_component_type)
                {
                    // relocate components.
                    usize stride = get_type_size(dst_component_type);
                    void* dst_data = (void*)((usize)(dst_chunk.m_components[dst_component_index]) + stride * dst_index_in_chunk);
                    void* src_data = (void*)((usize)(src_chunk.m_components[src_component_index]) + stride * src_index_in_chunk);
                    move_construct_type(dst_component_type, dst_data, src_data);
                    ++src_component_index;
                    ++dst_component_index;
                }
                else if (src_component_type < dst_component_type)
                {
                    // exist in src but not in dst, remove.
                    ++src_component_index;
                }
                else
                {
                    // exist in dst but not in src, add.
                    usize stride = get_type_size(dst_component_type);
                    void* dst_data = (void*)((usize)dst_chunk.m_components[dst_component_index] + stride * dst_index_in_chunk);
                    construct_type(dst_component_type, dst_data);
                    ++dst_component_index;
                }
            }
            while (src_component_index < src_components.size())
            {
                // exist in src but not in dst, remove.
                ++src_component_index;
            }
            while (dst_component_index < dst_components.size())
            {
                // exist in dst but not in src, add.
                typeinfo_t dst_component_type = dst_components[dst_component_index];
                usize stride = get_type_size(dst_component_type);
                void* dst_data = (void*)((usize)dst_chunk.m_components[dst_component_index] + stride * dst_index_in_chunk);
                construct_type(dst_component_type, dst_data);
                ++dst_component_index;
            }
            
            // remove old entity data.
            src_cluster->free_entry(world, src_index);
            return dst_index;
        }
        R<EntityAddress> World::set_entity_cluster(entity_id_t entity, Cluster* new_cluster)
        {
            auto record = get_entity_record(entity);
            if (!record)
            {
                return ECSError::entity_not_found();
            }
            usize new_index = relocate_entity(this, record->m_cluster, record->m_index, new_cluster);
            record->m_cluster = new_cluster;
            record->m_index = new_index;
            EntityAddress ret;
            ret.cluster = new_cluster;
            ret.index = new_index;
            return ret;
        }
        LUNA_ECS_API Ref<IWorld> new_world()
        {
            return new_object<World>();
        }
    }
}
