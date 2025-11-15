/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.hpp
* @author JXMaster
* @date 2022/8/11
*/
#pragma once
#include "../World.hpp"
#include "Cluster.hpp"
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/SelfIndexedHashMap.hpp>

namespace Luna
{
    namespace ECS
    {
        struct EntityRecord
        {
            Cluster* m_cluster = nullptr;
            usize m_index = 0;
            u32 m_generation = 0;
        };

        struct EntityIdAllocator
        {
            RingDeque<entity_id_t> m_free_ids;
            u32 m_next_free_slot;
            SpinLock m_lock;

            EntityIdAllocator() :
                m_next_free_slot(0) {}

            entity_id_t allocate_id()
            {
                LockGuard guard(m_lock);
                if (!m_free_ids.empty())
                {
                    entity_id_t id = m_free_ids.front();
                    m_free_ids.pop_front();
                    return make_entity_id(get_entity_index(id), get_entity_generation(id) + 1);
                }
                entity_id_t ret = make_entity_id(m_next_free_slot, 1);
                ++m_next_free_slot;
                return ret;
            }

            void free_id(entity_id_t id)
            {
                LockGuard guard(m_lock);
                m_free_ids.push_back(id);
            }
        };

        struct ClusterType
        {
            //! The sorted span that refers to all components in the archetype.
            Span<const typeinfo_t> components;
            //! The sorted span that refers to all tags in the archetype.
            Span<const tag_t> tags;

            bool operator==(const ClusterType& rhs) const
            {
                return (components.size() == rhs.components.size()) &&
                    (tags.size() == rhs.tags.size()) &&
                    (!memcmp(components.data(), rhs.components.data(), components.size_bytes())) &&
                    (!memcmp(tags.data(), rhs.tags.data(), tags.size_bytes()));
            }

        };

        struct ClusterExtractKey
        {
            ClusterType operator()(const UniquePtr<Cluster>& value)
            {
                ClusterType r;
                r.components = { value->m_component_types.data(), value->m_component_types.size() };
                r.tags = { value->m_tags.data(), value->m_tags.size() };
                return r;
            }
        };

        struct World : public IWorld
        {
            lustruct("ECS::World", "{945066F9-0292-46DC-8659-41D1C5874EA6}");
            luiimpl();

            //! Entity allocation and management.
            EntityIdAllocator m_entity_id_allocator;
            Vector<EntityRecord> m_entities;

            //! Clusters managed by this world.
            SelfIndexedHashMap<ClusterType, UniquePtr<Cluster>, ClusterExtractKey> m_clusters;

            EntityRecord* get_entity_record(entity_id_t entity);

            virtual Cluster* get_cluster(Span<const typeinfo_t> components, Span<const tag_t> tags, 
                bool create_if_not_exist) override;

            virtual void delete_cluster(Cluster* cluster) override;
            
            virtual void find_clusters(Span<const typeinfo_t> components, Span<const tag_t> tags, Vector<Cluster*>& out_clusters) override;

            virtual void find_clusters(const Function<bool(Cluster* cluster)>& filter, Vector<Cluster*>& out_clusters) override;

            virtual entity_id_t new_entity(Cluster* target_cluster, EntityAddress* out_address) override;

            virtual void delete_entity(entity_id_t entity) override;

            virtual void delete_all_entities() override;
            
            virtual R<EntityAddress> get_entity_address(entity_id_t entity) override;

            virtual R<EntityAddress> set_entity_cluster(entity_id_t entity, Cluster* new_cluster) override;
        };
    }

    template<>
    struct hash<ECS::ClusterType>
    {
        usize operator()(const ECS::ClusterType& val) const
        {
            usize h = 0;
            for (auto i : val.components)
            {
                h ^= hash<typeinfo_t>()(i);
            }
            for (auto i : val.tags)
            {
                h ^= hash<void*>()(i);
            }
            return h;
        }
    };
}