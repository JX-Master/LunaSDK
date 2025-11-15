/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Cluster.cpp
* @author JXMaster
* @date 2022/9/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "Cluster.hpp"
#include "World.hpp"

namespace Luna
{
    namespace ECS
    {
        Cluster::~Cluster()
        {
            for (usize cluster_i = 0; cluster_i < m_chunks.size(); ++cluster_i)
            {
                Chunk& chunk = m_chunks[cluster_i];
                usize num_entities = min(CLUSTER_CHUNK_CAPACITY, m_size - cluster_i * CLUSTER_CHUNK_CAPACITY);
                if(chunk.m_components)
                {
                    for (usize i = 0; i < m_component_types.size(); ++i)
                    {
                        typeinfo_t type = m_component_types[i];
                        if (!is_type_trivially_destructable(type))
                        {
                            usize strip = get_type_size(type);
                            for (usize i = 0; i < num_entities; ++i)
                            {
                                if (chunk.m_entities[i] != NULL_ENTITY)
                                {
                                    void* data = (void*)((usize)(chunk.m_components[i]) + strip * i);
                                    destruct_type(type, data);
                                }
                            }
                        }
                        memfree(chunk.m_components[i], get_type_alignment(type));
                    }
                    memfree(chunk.m_components);
                    chunk.m_components = nullptr;
                }
            }
        }
        usize Cluster::allocate_entry()
        {
            if (m_size == m_chunks.size() * CLUSTER_CHUNK_CAPACITY)
            {
                // Allocate a new chunk.
                m_chunks.emplace_back();
                Chunk& new_chunk = m_chunks.back();
                new_chunk.m_components = (void**)memalloc(sizeof(void*) * m_component_types.size());
                for(usize i = 0; i < m_component_types.size(); ++i)
                {
                    typeinfo_t type = m_component_types[i];
                    usize type_size = get_type_size(type);
                    // All memory remain uninitialized.
                    new_chunk.m_components[i] = (void*)memalloc(type_size * CLUSTER_CHUNK_CAPACITY, get_type_alignment(type));
                }
            }
            usize r = m_size;
            ++m_size;
            // Construct components.
            for(usize i = 0; i < m_component_types.size(); ++i)
            {
                typeinfo_t type = m_component_types[i];
                void* data = m_chunks[r / CLUSTER_CHUNK_CAPACITY].m_components[i];
                data = (void*)((usize)data + get_type_size(type) * (r % CLUSTER_CHUNK_CAPACITY));
                construct_type(type, data);
            }
            return r;
        }
        void Cluster::free_entry(World* world, usize index)
        {
            auto& chunk = m_chunks[index / CLUSTER_CHUNK_CAPACITY];
            usize index_in_chunk = index % CLUSTER_CHUNK_CAPACITY;
            // Destruct components.
            for (usize i = 0; i < m_component_types.size(); ++i)
            {
                typeinfo_t type = m_component_types[i];
                if (!is_type_trivially_destructable(type))
                {
                    usize strip = get_type_size(type);
                    void* data = (void*)((usize)(chunk.m_components[i]) + strip * index_in_chunk);
                    destruct_type(type, data);
                }
            }
            --m_size;
            if (index != m_size)
            {
                // Swap the back entity to fill the empty space.
                relocate_entity(index, m_size);
                // Update world record for the swapped entity.
                auto& ent = world->m_entities[get_entity_index(chunk.m_entities[index_in_chunk])];
                ent.m_index = index;
            }
            // Free unused chunks.
            if(m_size <= (m_chunks.size() - 1) * CLUSTER_CHUNK_CAPACITY)
            {
                auto& chunk = m_chunks.back();
                for(usize i = 0; i < m_component_types.size(); ++i)
                {
                    memfree(chunk.m_components[i], get_type_alignment(m_component_types[i]));
                }
                memfree(chunk.m_components);
                m_chunks.pop_back();
            }
        }
        void Cluster::relocate_entity(usize dst, usize src)
        {
            Chunk& dst_chunk = m_chunks[dst / CLUSTER_CHUNK_CAPACITY];
            Chunk& src_chunk = m_chunks[src / CLUSTER_CHUNK_CAPACITY];
            usize dst_index_in_chunk = dst % CLUSTER_CHUNK_CAPACITY;
            usize src_index_in_chunk = src % CLUSTER_CHUNK_CAPACITY;
            dst_chunk.m_entities[dst_index_in_chunk] = move(src_chunk.m_entities[src_index_in_chunk]);
            for (usize i = 0; i < m_component_types.size(); ++i)
            {
                typeinfo_t type = m_component_types[i];
                usize strip = get_type_size(type);
                void* dst_ptr = (void*)((usize)(dst_chunk.m_components[i]) + strip * dst_index_in_chunk);
                void* src_ptr = (void*)((usize)(src_chunk.m_components[i]) + strip * src_index_in_chunk);
                relocate_type(type, dst_ptr, src_ptr);
            }
        }
        LUNA_ECS_API Span<const typeinfo_t> get_cluster_components(Cluster* cluster)
        {
            return { cluster->m_component_types.data(), cluster->m_component_types.size() };
        }
        LUNA_ECS_API Span<const tag_t> get_cluster_tags(Cluster* cluster)
        {
            return { cluster->m_tags.data(), cluster->m_tags.size() };
        }
        LUNA_ECS_API usize get_cluster_num_entities(Cluster* cluster)
        {
            return cluster->m_size;
        }
        LUNA_ECS_API usize get_cluster_num_chunks(Cluster* cluster)
        {
            return cluster->m_chunks.size();
        }
        LUNA_ECS_API Span<const entity_id_t> get_cluster_entities(Cluster* cluster, usize chunk)
        {
            usize num_entities = min(CLUSTER_CHUNK_CAPACITY, cluster->m_size - chunk * CLUSTER_CHUNK_CAPACITY);
            return Span<const entity_id_t>(cluster->m_chunks[chunk].m_entities, num_entities);
        }
        LUNA_ECS_API void* get_cluster_components_data(Cluster* cluster, usize chunk, typeinfo_t component_type)
        {
            auto& component_types = cluster->m_component_types;
            auto iter = binary_search_iter(component_types.begin(), component_types.end(), component_type);
            if (iter == component_types.end()) return nullptr;
            usize index = iter - component_types.begin();
            return cluster->m_chunks[chunk].m_components[index];
        }
    }
}