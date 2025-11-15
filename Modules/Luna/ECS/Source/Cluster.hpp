/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Cluster.hpp
* @author JXMaster
* @date 2022/9/6
*/
#pragma once
#include "../Cluster.hpp"
#include <Luna/Runtime/Array.hpp>
#include <Luna/Runtime/Vector.hpp>
namespace Luna
{
    namespace ECS
    {
        struct World;

        inline constexpr u32 get_entity_index(entity_id_t id)
        {
            return u32(id);
        }

        inline constexpr u32 get_entity_generation(entity_id_t id)
        {
            return u32(id >> 32);
        }

        inline constexpr entity_id_t make_entity_id(u32 index, u32 generation)
        {
            return (u64)index | (((u64)generation) << 32);
        }

        struct Chunk
        {
            entity_id_t m_entities[CLUSTER_CHUNK_CAPACITY];
            void** m_components = nullptr;
        };

        // corresponding to `cluster_t`, managed by World.
        struct Cluster
        {
            Array<typeinfo_t> m_component_types;
            Array<tag_t> m_tags;
            
            Vector<Chunk> m_chunks;
            usize m_size;

            Cluster() :
                m_size(0) {}

            ~Cluster();

            usize allocate_entry();
            void free_entry(World* world, usize index);
            void relocate_entity(usize dst, usize src);
        };
    }
}