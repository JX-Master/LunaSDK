/*!
* This file is a portion of Luna SDK.
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
		ModuleDesc* m_prior = nullptr;
		ModuleDesc* m_next = nullptr;
		bool m_initialized = false;
	};
	static_assert(sizeof(ModuleEntry) <= sizeof(c8) * 32, "ModuleEntry size too big!");

	ModuleDesc* g_module_header;

	LUNA_RUNTIME_API void add_module(ModuleDesc* module_desc)
	{
		ModuleEntry* entry = (ModuleEntry*)(module_desc->reserved);
		new (entry) ModuleEntry();
		entry->m_next = g_module_header;
		ModuleEntry* last_entry = (ModuleEntry*)(g_module_header->reserved);
		if (last_entry)
		{
			last_entry->m_prior = module_desc;
		}
		g_module_header = module_desc;
	}

	LUNA_RUNTIME_API void remove_module(ModuleDesc* module_desc)
	{
		ModuleEntry* entry = (ModuleEntry*)(module_desc->reserved);
		ModuleDesc* prior = entry->m_prior;
		ModuleDesc* next = entry->m_next;
		if (prior)
		{
			ModuleEntry* prior_entry = (ModuleEntry*)(prior->reserved);
			prior_entry->m_next = next;
		}
		if (next)
		{
			ModuleEntry* next_extry = (ModuleEntry*)(next->reserved);
			next_extry->m_prior = prior;
		}
		entry->m_prior = nullptr;
		entry->m_next = nullptr;
		if (g_module_header == module_desc)
		{
			g_module_header = next;
		}
	}

	//! Records all initialized modules, sorted by their initialization order.
	Vector<ModuleDesc*> g_initialized_modules;

	void module_init()
	{
	}

	void module_close()
	{
		for (auto iter = g_initialized_modules.rbegin(); iter != g_initialized_modules.rend(); ++iter)
		{
			ModuleEntry* entry = (ModuleEntry*)((*iter)->reserved);
			if (entry->m_initialized)
			{
				if ((*iter)->close_func)
				{
					(*iter)->close_func();
				}
				entry->m_initialized = false;
			}
		}
		g_initialized_modules.clear();
		g_initialized_modules.shrink_to_fit();
	}

	static HashMap<Name, ModuleDesc*> get_registered_modules()
	{
		HashMap<Name, ModuleDesc*> modules;
		{
			ModuleDesc* iter = g_module_header;
			while (iter)
			{
				modules.insert(Pair<Name, ModuleDesc*>(Name(iter->name), iter));
				ModuleEntry* entry = (ModuleEntry*)(iter->reserved);
				iter = entry->m_next;
			}
		}
		return modules;
	}

	static Vector<Name> get_dependency_module_names(const c8* dependencies)
	{
		const c8* cur = dependencies;
		Vector<Name> ret;
		while (cur && *cur != '\0')
		{
			// Find next ';'
			const c8* end = strchr(cur, ';');
			Name dep;
			if (end)
			{
				dep = Name(cur, end - cur);
			}
			else
			{
				dep = Name(cur);
			}
			ret.push_back(dep);
			if (end)
			{
				cur = end + 1;
			}
			else
			{
				break;
			}
		}
		return ret;
	}

	static R<Vector<ModuleDesc*>> get_module_init_queue(const HashMap<Name, ModuleDesc*>& modules)
	{
		HashMap<Name, ModuleDesc*> unresolved_modules = modules;
		// Exclude all initialized modules.
		{
			auto iter = unresolved_modules.begin();
			while (iter != unresolved_modules.end())
			{
				ModuleEntry* entry = (ModuleEntry*)(iter->second->reserved);
				if (entry->m_initialized)
				{
					iter = unresolved_modules.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}
		Vector<ModuleDesc*> init_queue;
		// Loop until all modules are resolved and pushed into the queue.
		while (!unresolved_modules.empty())
		{
			bool any_removed = false;
			auto iter = unresolved_modules.begin();
			while (iter != unresolved_modules.end())
			{
				bool can_init = true;	// If this module can be initialized.

				// Check dependencies.
				auto dependencies = get_dependency_module_names((*iter).second->dependencies);

				for (auto& dep : dependencies)
				{
					if (!modules.contains(dep))
					{
						return set_error(BasicError::not_found(), "Module %s required by module %s is not found.", dep.c_str(), iter->second->name);
					}
					if (unresolved_modules.contains(dep))
					{
						// The dependency is not initialized.
						can_init = false;
						break;
					}
				}
				if (can_init)
				{
					// Remove this.
					init_queue.push_back(iter->second);
					ModuleEntry* entry = (ModuleEntry*)(iter->second->reserved);
					any_removed = true;
					iter = unresolved_modules.erase(iter);
				}
				else
				{
					++iter; // Skip this, leave to next roll.
				}
			}
			if (!any_removed)
			{
				// Cycle reference detected.
				return set_error(BasicError::bad_arguments(), "Cycling module dependencies detected.");
			}
		}
		return init_queue;
	}

	static void get_module_dependencies(const HashMap<Name, ModuleDesc*>& modules, HashSet<ModuleDesc*>& dependencies, ModuleDesc* desc)
	{
		auto deps = get_dependency_module_names(desc->dependencies);
		for (auto& dep : deps)
		{
			auto iter = modules.find(dep);
			luassert(iter != modules.end());
			dependencies.insert(iter->second);
			get_module_dependencies(modules, dependencies, iter->second);
		}
	}

	static RV init_module(ModuleDesc* desc)
	{
		lutry
		{
			ModuleEntry* entry = (ModuleEntry*)(desc->reserved);
			if (!entry->m_initialized)
			{
				if (desc->init_func)
				{
					luexp(desc->init_func());
				}
				entry->m_initialized = true;
				g_initialized_modules.push_back(desc);
			}
		}
		lucatchret;
		return ok;
	}

	LUNA_RUNTIME_API RV init_module_dependencies(const Name& module_name)
	{
		lutry
		{
			auto modules = get_registered_modules();
			auto iter = modules.find(module_name);
			if (iter == modules.end())
			{
				return set_error(BasicError::not_found(), "Module %s is not found.", module_name.c_str());
			}
			ModuleEntry* entry = (ModuleEntry*)(iter->second->reserved);
			if (entry->m_initialized) return ok;
			lulet(queue, get_module_init_queue(modules));
			HashSet<ModuleDesc*> dependencies;
			get_module_dependencies(modules, dependencies, iter->second);
			for (auto i : queue)
			{
				auto iter = dependencies.find(i);
				if (iter != dependencies.end())
				{
					luexp(init_module(i));
				}
			}
		}
		lucatchret;
		return ok;
	}

	LUNA_RUNTIME_API RV init_module(const Name& module_name)
	{
		lutry
		{
			auto modules = get_registered_modules();
			auto iter = modules.find(module_name);
			if (iter == modules.end())
			{
				return set_error(BasicError::not_found(), "Module %s is not found.", module_name.c_str());
			}
			ModuleEntry* entry = (ModuleEntry*)(iter->second->reserved);
			if (entry->m_initialized) return ok;
			lulet(queue, get_module_init_queue(modules));
			HashSet<ModuleDesc*> dependencies;
			get_module_dependencies(modules, dependencies, iter->second);
			for (auto i : queue)
			{
				auto iter = dependencies.find(i);
				if (iter != dependencies.end())
				{
					luexp(init_module(i));
				}
			}
			luexp(init_module(iter->second));
		}
		lucatchret;
		return ok;
	}

	LUNA_RUNTIME_API RV init_modules()
	{
		lutry
		{
			auto modules = get_registered_modules();
			lulet(queue, get_module_init_queue(modules));
			// Initialize all modules by order.
			for (auto i : queue)
			{
				luexp(init_module(i));
			}
		}
		lucatchret;
		return ok;
	}
}
