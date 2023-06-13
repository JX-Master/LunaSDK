
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EntityResolver.hpp
* @author JXMaster
* @date 2023/1/10
*/
#pragma once
#include "Cluster.hpp"
namespace Luna
{
    namespace ECS
    {
        struct EntityResolver
        {
            Cluster* m_src_cluster = { nullptr };
			usize m_src_index = 0;
			Vector<typeinfo_t> m_component_types;
			Vector<entity_id_t> m_tags;
            HashMap<typeinfo_t, void*> m_data;
            void apply(World* world, entity_id_t entity);
			bool add_component(typeinfo_t component);
			bool remove_component(typeinfo_t component);
			bool add_tag(entity_id_t tag);
			bool remove_tag(entity_id_t tag);
        };
    }
}