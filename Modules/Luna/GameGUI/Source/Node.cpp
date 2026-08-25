/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Node.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            SpinLock g_node_registry_lock;
            HashMap<Guid, NodeTypeDesc> g_node_types;
        }

        LUNA_GAME_GUI_API RV register_node_type(const NodeTypeDesc& desc)
        {
            if(desc.type == Guid() || desc.name.empty() || desc.current_version == 0 || !desc.build)
            {
                return set_error(E_BAD_ARGUMENTS,
                    "GameGUI node descriptors require a non-zero type, name, current version and build callback.");
            }
            if(desc.property_schema.valid() && desc.property_schema.type() != VariantType::object)
            {
                return set_error(E_BAD_ARGUMENTS, "GameGUI property schema must be an object.");
            }
            if(desc.default_properties.valid() && desc.default_properties.type() != VariantType::object)
            {
                return set_error(E_BAD_ARGUMENTS, "GameGUI default properties must be an object.");
            }
            LockGuard guard(g_node_registry_lock);
            if(g_node_types.find(desc.type) != g_node_types.end()) return E_ALREADY_EXISTS;
            for(const auto& registered : g_node_types)
            {
                if(registered.second.name == desc.name) return E_ALREADY_EXISTS;
            }
            g_node_types.insert(make_pair(desc.type, desc));
            return ok;
        }

        LUNA_GAME_GUI_API RV unregister_node_type(const Guid& type)
        {
            LockGuard guard(g_node_registry_lock);
            auto iter = g_node_types.find(type);
            if(iter == g_node_types.end()) return E_NOT_FOUND;
            g_node_types.erase(iter);
            return ok;
        }

        LUNA_GAME_GUI_API R<NodeTypeDesc> get_node_type(const Guid& type)
        {
            LockGuard guard(g_node_registry_lock);
            auto iter = g_node_types.find(type);
            if(iter == g_node_types.end()) return E_NOT_FOUND;
            return iter->second;
        }

        LUNA_GAME_GUI_API void get_node_types(Vector<NodeTypeDesc>& descriptors)
        {
            LockGuard guard(g_node_registry_lock);
            for(const auto& desc : g_node_types) descriptors.push_back(desc.second);
        }

        void close_node_registry()
        {
            LockGuard guard(g_node_registry_lock);
            g_node_types.clear();
        }
    }
}
