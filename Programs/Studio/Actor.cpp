/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Actor.cpp
* @author JXMaster
* @date 2025/11/9
*/
#include "Actor.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include "Scene.hpp"

namespace Luna
{
    Actor::Actor(const Guid& guid, ECS::IWorld* world, Span<const typeinfo_t> components, Span<const ECS::tag_t> tags) :
        m_world(world)
    {
        Vector<typeinfo_t> modified_components;
        modified_components.reserve(components.size() + 2);
        modified_components.push_back(typeof<Transform>());
        modified_components.push_back(typeof<ActorInfo>());
        for(typeinfo_t type : components)
        {
            modified_components.push_back(type);
        }
        ECS::Cluster* cluster = world->get_cluster(modified_components.cspan(), tags, true);
        m_entity = world->new_entity(cluster);
        ActorInfo* info = get_component<ActorInfo>();
        info->guid = guid;
        info->actor = this;
    }

    Actor::~Actor()
    {
        m_world->delete_entity(m_entity);
    }

    void Actor::add_child(Actor* child, usize index)
    {
        ActorInfo* child_info = child->get_component<ActorInfo>();
        if(child_info->parent)
        {
            child_info->parent->remove_child(child);
        }
        child_info->parent = this;
        ActorInfo* info = get_component<ActorInfo>();
        if(index >= info->children.size())
        {
            info->children.push_back(child);
        }
        else
        {
            info->children.insert(info->children.begin() + index, child);
        }
    }

    void Actor::remove_child(usize index)
    {
        ActorInfo* info = get_actor_info();
        if(index < info->children.size())
        {
            info->children[index]->get_actor_info()->parent = nullptr;
            info->children.erase(info->children.begin() + index);
        }
    }

    void Actor::remove_child(Actor* child)
    {
        ActorInfo* info = get_actor_info();
        for(auto iter = info->children.begin(); iter != info->children.end(); ++iter)
        {
            if(*iter == child)
            {
                child->get_actor_info()->parent = nullptr;
                info->children.erase(iter);
                break;
            }
        }
    }

    Float4x4 Actor::get_local_to_world_matrix() const
    {
        const ActorInfo* info = get_component<ActorInfo>();
        const Transform* transform = get_component<Transform>();
        if(info->parent)
        {
            return mul(transform->get_this_to_parent_matrix(), info->parent->get_local_to_world_matrix());
        }
        else
        {
            return transform->get_this_to_parent_matrix();
        }
    }

    Float4x4 Actor::get_world_to_local_matrix() const
    {
        const ActorInfo* info = get_component<ActorInfo>();
        const Transform* transform = get_component<Transform>();
        if(info->parent)
        {
            return mul(info->parent->get_world_to_local_matrix(), transform->get_parent_to_this_matrix());
        }
        else
        {
            return transform->get_parent_to_this_matrix();
        }
    }

    Float3 Actor::get_world_position() const
    {
        const ActorInfo* info = get_component<ActorInfo>();
        const Transform* transform = get_component<Transform>();
        if(info->parent)
        {
            Float4x4 mat = info->parent->get_local_to_world_matrix();
            Float4 pos = mul(Float4(transform->position.x, transform->position.y, transform->position.z, 1.0f), mat);
            return Float3(pos.x, pos.y, pos.z);
        }
        return transform->position;
    }

    Quaternion Actor::get_world_rotation() const
    {
        const ActorInfo* info = get_component<ActorInfo>();
        const Transform* transform = get_component<Transform>();
        if(info->parent)
        {   
            return mul(transform->rotation, info->parent->get_world_rotation());
        }
        return transform->rotation;
    }

    void Actor::set_world_position(const Float3& position)
    {
        const ActorInfo* info = get_component<ActorInfo>();
        Transform* transform = get_component<Transform>();
        Float4 pos(position.x, position.y, position.z, 1.0f);
        pos = mul(pos, get_world_to_local_matrix());
        transform->position = Float3(pos.x, pos.y, pos.z);
    }

    void Actor::set_local_to_world_matrix(const Float4x4& mat)
    {
        const ActorInfo* info = get_component<ActorInfo>();
        Transform* transform = get_component<Transform>();
        if (info->parent)
        {
            // We need to decompose the local-to-world matrix to this-to-parent matrix.
            transform->set_this_to_parent_matrix(mul(mat, info->parent->get_world_to_local_matrix()));
        }
        else
        {
            transform->set_this_to_parent_matrix(mat);
        }
    }

    void Actor::set_world_to_local_matrix(const Float4x4& mat)
    {
        const ActorInfo* info = get_component<ActorInfo>();
        Transform* transform = get_component<Transform>();
        if (info->parent)
        {
            transform->set_parent_to_this_matrix(mul(info->parent->get_local_to_world_matrix(), mat));
        }
        else
        {
            transform->set_parent_to_this_matrix(mat);
        }
    }

    const void* Actor::get_component(typeinfo_t type) const
    {
        auto r = m_world->get_entity_address(m_entity);
        luassert(succeeded(r));
        ECS::EntityAddress addr = r.get();
        void* component_data_array = ECS::get_cluster_components_data(addr.cluster, addr.index / ECS::CLUSTER_CHUNK_CAPACITY, type);
        if(!component_data_array) return nullptr;
        return (const void*)(((usize)component_data_array) + get_type_size(type) * (addr.index % ECS::CLUSTER_CHUNK_CAPACITY));
    }

    void* Actor::get_component(typeinfo_t type)
    {
        auto r = m_world->get_entity_address(m_entity);
        luassert(succeeded(r));
        ECS::EntityAddress addr = r.get();
        void* component_data_array = ECS::get_cluster_components_data(addr.cluster, addr.index / ECS::CLUSTER_CHUNK_CAPACITY, type);
        if(!component_data_array) return nullptr;
        return (void*)(((usize)component_data_array) + get_type_size(type) * (addr.index % ECS::CLUSTER_CHUNK_CAPACITY));
    }

    void* Actor::add_component(typeinfo_t type)
    {
        ECS::EntityAddress addr = m_world->get_entity_address(m_entity).get();
        void* component_data_array = ECS::get_cluster_components_data(addr.cluster, addr.index / ECS::CLUSTER_CHUNK_CAPACITY, type);
        if(component_data_array)
        {
            return (void*)(((usize)component_data_array) + get_type_size(type) * (addr.index % ECS::CLUSTER_CHUNK_CAPACITY));
        }
        Vector<typeinfo_t> types;
        auto old_types = ECS::get_cluster_components(addr.cluster);
        types.reserve(old_types.size() + 1);
        for(typeinfo_t t : old_types)
        {
            types.push_back(t);
        }
        types.push_back(type);
        auto new_cluster = m_world->get_cluster(types.cspan(), ECS::get_cluster_tags(addr.cluster), true);
        addr = m_world->set_entity_cluster(m_entity, new_cluster).get();
        component_data_array = ECS::get_cluster_components_data(addr.cluster, addr.index / ECS::CLUSTER_CHUNK_CAPACITY, type);
        return (void*)(((usize)component_data_array) + get_type_size(type) * (addr.index % ECS::CLUSTER_CHUNK_CAPACITY));
    }

    void Actor::remove_component(typeinfo_t type)
    {
        ECS::EntityAddress addr = m_world->get_entity_address(m_entity).get();
        auto types = ECS::get_cluster_components(addr.cluster);
        Vector<typeinfo_t> new_types;
        new_types.assign(types);
        bool any_removed = false;
        for(auto iter = new_types.begin(); iter != new_types.end(); ++iter)
        {
            if(*iter == type)
            {
                new_types.erase(iter);
                any_removed = true;
                break;
            }
        }
        if(!any_removed) return;
        auto new_cluster = m_world->get_cluster(new_types.cspan(), ECS::get_cluster_tags(addr.cluster), true);
        addr = m_world->set_entity_cluster(m_entity, new_cluster).get();
    }
}
