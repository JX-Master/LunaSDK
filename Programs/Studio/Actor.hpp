/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Actor.hpp
* @author JXMaster
* @date 2025/11/9
*/
#pragma once

#include <Luna/ECS/World.hpp>
#include "Transform.hpp"
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
    struct Actor;

    struct ActorInfo
    {
        lustruct("ActorInfo", "bfeab38f-5057-4d52-8c9c-dfe776228e7b");

        ActorInfo()
        {
            log_info("Studio", "ActorInfo construct");
        }

        ~ActorInfo()
        {
            log_info("Studio", "ActorInfo destruct");
        }

        Guid get_guid() const
        {
            return guid;
        }

        const Actor* get_actor() const
        {
            return actor;
        }

        Actor* get_actor()
        {
            return actor;
        }

        Actor* get_parent() const
        {
            return parent;
        }

        void get_children(Vector<Actor*> out_children) const
        {
            for(Actor* child : children)
            {
                out_children.push_back(child);
            }
        }

    private:
        friend Actor;
        Guid guid;
    public:
        Name name;
    private:
        Actor* actor = nullptr;
        Actor* parent = nullptr;
        Vector<Actor*> children;
    };

    //! The base actor in the scene.
    struct Actor
    {

        Actor(const Guid& guid, ECS::IWorld* world, Span<const typeinfo_t> components, Span<const ECS::tag_t> tags);
        virtual ~Actor();

        ECS::IWorld* get_world() const
        {
            return m_world;
        }

        ECS::entity_id_t get_entity() const
        {
            return m_entity;
        }

        void add_child(Actor* child, usize index = USIZE_MAX);
        void remove_child(usize index);
        void remove_child(Actor* child);

        const ActorInfo* get_actor_info() const
        {
            return get_component<ActorInfo>();
        }
        ActorInfo* get_actor_info()
        {
            return get_component<ActorInfo>();
        }
        const Transform* get_transform() const
        {
            return get_component<Transform>();
        }
        Transform* get_transform()
        {
            return get_component<Transform>();
        }
        Float4x4 get_local_to_world_matrix() const;
        Float4x4 get_world_to_local_matrix() const;

        Float3 get_world_position() const;
        Quaternion get_world_rotation() const;
        void set_world_position(const Float3& position);
        void set_local_to_world_matrix(const Float4x4& mat);
        void set_world_to_local_matrix(const Float4x4& mat);

        const void* get_component(typeinfo_t type) const;
        void* get_component(typeinfo_t type);
        template <typename _Ty>
        _Ty* get_component()
        {
            return (_Ty*)get_component(typeof<_Ty>());
        }
        template <typename _Ty>
        const _Ty* get_component() const
        {
            return (const _Ty*)get_component(typeof<_Ty>());
        }
        void* add_component(typeinfo_t type);
        template <typename _Ty>
        _Ty* add_component()
        {
            return (_Ty*)add_component(typeof<_Ty>());
        }
        void remove_component(typeinfo_t type);
        template <typename _Ty>
        void remove_component()
        {
            remove_component(typeof<_Ty>());
        }

    private:
        // This is owned by game world, the game world also manages 
        // all actors' lifecycle, so we can use raw pointer
        // here.
        ECS::IWorld* m_world;
        ECS::entity_id_t m_entity;
    };

    struct ActorRef
    {
        lustruct("ActorRef", "aa67dbc3-b319-412e-a951-67f2d818f742");

        Guid guid = Guid(0, 0);
    };
}