/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Name.cpp
* @author JXMaster
* @date 2020/8/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Module.hpp"
#include "../Module.hpp"
#include "../Assert.hpp"
#include "../Vector.hpp"
#include "../HashSet.hpp"
#include "../HashMap.hpp"
#include "../Name.hpp"
#include "../Result.hpp"
#include "../RingDeque.hpp"
namespace Luna
{
    struct ModuleEntry
    {
        Name m_name;
        Vector<Module*> m_dependencies;
        bool m_initialized = false;
    };

    // All registered modules.
    HashMap<Module*, ModuleEntry> g_modules;

    // Records all initialized modules, sorted by their initialization order.
    Vector<Module*> g_initialized_modules;

    void module_init()
    {
    }
    void module_close()
    {
        for (auto iter = g_initialized_modules.rbegin(); iter != g_initialized_modules.rend(); ++iter)
        {
            Module* entry = *iter;
            entry->on_close();
        }
        g_initialized_modules.clear();
        g_initialized_modules.shrink_to_fit();
        g_modules.clear();
        g_modules.shrink_to_fit();    
    }
    LUNA_RUNTIME_API RV add_module(Module* handle)
    {
        auto iter = g_modules.find(handle);
        if(iter != g_modules.end()) return ok;
        ModuleEntry entry;
        entry.m_name = handle->get_name();
        g_modules.insert(make_pair(handle, move(entry)));
        auto r = handle->on_register();
        if(failed(r))
        {
            g_modules.erase(handle);
            return r;
        }
        return ok;
    }
    LUNA_RUNTIME_API void remove_module(Module* handle)
    {
        auto iter = g_modules.find(handle);
        lucheck_msg(iter != g_modules.end(), "remove_module failed: module not registered.");
        lucheck_msg(!iter->second.m_initialized, "remove_module failed: try to unregister one initialized module.");
        g_modules.erase(iter);
    }
    LUNA_RUNTIME_API RV add_dependency_module(Module* current, Module* dependency)
    {
        auto iter = g_modules.find(current);
        lucheck_msg(iter != g_modules.end(), "add_dependency_module failed: current module must be registered firstly!");
        auto r = add_module(dependency);
        if(failed(r)) return r;
        for(Module* dep : iter->second.m_dependencies)
        {
            if(dep == dependency) return ok;
        }
        iter->second.m_dependencies.push_back(dependency);
        return ok;
    }
    LUNA_RUNTIME_API Module* get_module_by_name(const Name& name)
    {
        for(auto iter = g_modules.begin(); iter != g_modules.end(); ++iter)
        {
            if(iter->second.m_name == name)
            {
                return iter->first;
            }
        }
        return nullptr;
    }
    static RV visit_module(
        Module* m,
        HashSet<Module*>& visited,
        HashSet<Module*>& visiting,
        Vector<Module*>& out_init_queue
    )
    {
        lutry
        {
            if(visiting.contains(m))
            {
                // Cycle reference detected.
                return set_error(BasicError::bad_arguments(), "Cycling module dependencies detected.");
            }
            if(!visited.insert(m).second)
            {
                // This node is already visited.
                return ok;
            }
            auto iter = g_modules.find(m);
            luassert(iter != g_modules.end());
            if(iter->second.m_initialized)
            {
                // Skip modules that are already initialized.
                return ok;
            }
            const Vector<Module*>& dependencies = iter->second.m_dependencies;
            visiting.insert(m);
            for(Module* dep : dependencies)
            {
                luexp(visit_module(dep, visited, visiting, out_init_queue));
            }
            visiting.erase(m);
            out_init_queue.push_back(m);
        }
        lucatchret;
        return ok;
    }
    static R<Vector<Module*>> get_module_init_queue()
    {
        Vector<Module*> init_queue;
        HashSet<Module*> visited;
        HashSet<Module*> visiting;
        for(auto& m : g_modules)
        {
            auto r = visit_module(m.first, visited, visiting, init_queue);
            if(failed(r))
            {
                return r.errcode();
            }
        }
        return init_queue;
    }
    static RV init_single_module(Module* handle)
    {
        lutry
        {
            auto entry = g_modules.find(handle);
            if (!entry->second.m_initialized)
            {
                luexp(entry->first->on_init());
                entry->second.m_initialized = true;
                g_initialized_modules.push_back(entry->first);
            }
        }
        lucatch
        {
            return set_error(luerr, "Failed to initialize module %s: %s", handle->get_name(), explain(luerr));
        }
        return ok;
    }
    LUNA_RUNTIME_API RV init_module_dependencies(Module* handle)
    {
        lucheck_msg(handle, "init_module_dependencies failed: handle is nullptr");
        lutry
        {
            auto iter = g_modules.find(handle);
            if (iter == g_modules.end())
            {
                return set_error(BasicError::not_found(), "Module %s is not registered.", handle->get_name());
            }
            if (iter->second.m_initialized) return ok;
            lulet(queue, get_module_init_queue());
            HashSet<Module*> dependencies;
            for(Module* m : iter->second.m_dependencies)
            {
                dependencies.insert(m);
            }
            for (auto i : queue)
            {
                auto iter = dependencies.find(i);
                if (iter != dependencies.end())
                {
                    luexp(init_single_module(i));
                }
            }
        }
        lucatchret;
        return ok;
    }
    LUNA_RUNTIME_API RV init_module(Module* handle)
    {
        lucheck_msg(handle, "init_module failed: handle is nullptr");
        lutry
        {
            auto iter = g_modules.find(handle);
            if (iter == g_modules.end())
            {
                return set_error(BasicError::not_found(), "Module %s is not found.", handle->get_name());
            }
            if (iter->second.m_initialized) return ok;
            lulet(queue, get_module_init_queue());
            HashSet<Module*> dependencies;
            for(Module* m : iter->second.m_dependencies)
            {
                dependencies.insert(m);
            }
            for (auto i : queue)
            {
                auto iter = dependencies.find(i);
                if (iter != dependencies.end())
                {
                    luexp(init_single_module(i));
                }
            }
            luexp(init_single_module(iter->first));
        }
        lucatchret;
        return ok;
    }
    LUNA_RUNTIME_API bool is_module_initialized(Module* module)
    {
        auto iter = g_modules.find(module);
        if(iter == g_modules.end()) return false;
        return iter->second.m_initialized;
    }
    LUNA_RUNTIME_API RV init_modules()
    {
        lutry
        {
            lulet(queue, get_module_init_queue());
            // Initialize all modules by order.
            for (auto i : queue)
            {
                luexp(init_single_module(i));
            }
        }
        lucatchret;
        return ok;
    }
}
