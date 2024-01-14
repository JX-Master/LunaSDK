/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.hpp
* @author JXMaster
* @date 2020/8/26
* @brief Runtime module management APIs.
*/
#pragma once
#include "Base.hpp"
#include "Result.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

//! @addtogroup Runtime
//! @{
//! @defgroup RuntimeModule Module system
//! @}

namespace Luna
{
	//! @addtogroup RuntimeModule
	//! @{

	//! The module interface that should be implemented by the user.
	struct Module
	{
		//! Gets one module name. Every module must have one unique name, and the name cannot changed after the module
		//! has been registered to the module system.
		//! @return Returns one string that represents the module name. The lifetime of the string should be equal to the lifetime of the module.
		virtual const c8* get_name() = 0;

		//! Called when the module is registered to the system for the first time.
		virtual RV on_register() { return ok; }

		//! Called when the module is initialized.
		virtual RV on_init() { return ok; }
		
		//! Called when the module is closed.
		virtual void on_close() {}
		
		virtual ~Module() {}
	};

	//! @brief Adds one module to the module system. If this module is already added, this function does nothing.
	//! @param[in] handle The module description structure pointer. This pointer shall be unique for every added module, the 
	//! module system uses this pointer to identify the module.
	//! @remark Adding one module to the module system does not initialize the module, the added module must be explicitly initialized 
	//! using @ref init_module or @ref init_modules before it can be used. 
	//! @par Valid Usage
	//! * `handle` must point to one valid module instance.
	LUNA_RUNTIME_API RV add_module(Module* handle);

	//! @brief Adds modules to the module system.
	//! @param[in] handles The module description structure pointers. These pointers shall be unique for every added module, the 
	//! module system uses these pointers to identify modules.
	//! @remark See remarks of @ref add_module for details.
	//! @par Valid Usage
	//! * All module pointers in `handles` must point to valid module instances.
	inline RV add_modules(Span<Module*> handles)
	{
		for(Module* h : handles)
		{
			auto r = add_module(h);
			if(failed(r))
			{
				return r;
			}
		}
		return ok;
	}

	//! @brief Removes one module from the module system. This function cannot be called when the module is currently initialized.
	//! @param[in] handle The module description structure pointer of the module to remove.
	//! @par Valid Usage
	//! * `handle` must point to one module that is already registered by @ref add_module.
	LUNA_RUNTIME_API void remove_module(Module* handle);

	//! @brief Adds one module as the dependency module of one module. This is usually called in module registration callback.
	//! @param[in] current The current module that depends on `dependency`.
	//! @param[in] dependency The dependency module. If this module is not added, it will be added to the module system firstly.
	//! @par Valid Usage
	//! * `current` must point to one module that is already registered by @ref add_module.
	//! * `dependency` must point to one valid module instance.
	LUNA_RUNTIME_API RV add_dependency_module(Module* current, Module* dependency);

	//! @brief Adds one span of modules as the dependency modules of one module.
	//! @param[in] current The current module that depends on `dependencies`.
	//! @param[in] dependencies The dependency modules. If these modules are not added, they will be added to the module system firstly.
	//! @par Valid Usage
	//! * `current` must point to one module that is already registered by @ref add_module.
	//! * All module pointers in `dependencies` must point to valid module instances.
	inline RV add_dependency_modules(Module* current, Span<Module*> dependencies)
	{
		for(Module* dep : dependencies)
		{
			auto r = add_dependency_module(current, dep);
			if(failed(r))
			{
				return r;
			}
		}
		return ok;
	}

	//! @brief Gets the module pointer by its name. The module must be registered firstly.
	//! @param[in] name The name of the module.
	//! @return Returns the registered module pointer, or `nullptr` if the module is not found.
	LUNA_RUNTIME_API Module* get_module_by_name(const Name& name);

	//! @brief Initializes all dependency modules for the specified module, but leaves the specified module as uninitialized.
	//! You may use this API to perform some pre-init configurations for the module initialize process.
	//! @param[in] handle The module to initialize dependencies.
	//! @remark If the specified module is already initialized, this function does nothing and succeeds.
	//! @par Valid Usage
	//! * `handle` must point to one module that is already registered by @ref add_module.
	LUNA_RUNTIME_API RV init_module_dependencies(Module* handle);

	//! @brief Initializes the specified module and all dependency modules of the specified module.
	//! @param[in] handle The module to initialize.
	//! @remark If the specified module is already initialized, this function does nothing and succeeds.
	//! @par Valid Usage
	//! * `handle` must point to one module that is already registered by @ref add_module.
	LUNA_RUNTIME_API RV init_module(Module* handle);

	//! @brief Initializes all uninitialized modules.
	LUNA_RUNTIME_API RV init_modules();
	
	//! @}
}