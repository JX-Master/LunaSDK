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

	//! @brief The function to be called when the module is initialized.
	using module_init_func_t = RV();

	//! @brief The function to be called when the module is closed.
	using module_close_func_t = void(void);

	//! @brief Module description structure. 
	//! @details This shall be allocated on static memory and being kept valid during the application 
	//! lifetime.
	struct ModuleDesc
	{
		//! @brief Used by the Runtime. The user should not change these memory.
		c8 reserved[32];

		// Filled by the user.
		
		//! @brief The name of the module. The lifetime of the string should be equal to the lifetime of the module.
		const c8* name;
		
		//! @brief A string that records modules this module depends on.
		//！@details The lifetime of the string should be no shorter than
		//! the lifetime of the module.
		//! 
		//! In case that multiple modules are required, use semicolons(;) to separate them (for example: "Core;Input;Gfx").
		//! There should not be any semicolon after the last item in the string.
		//! 
		//! Dependencies to `Runtime` should not be listed here. The Runtime is always initialized before any other
		//! module gets initialized.
		const c8* dependencies;

		//! @brief The initialize function of the module. 
		//! @details This can be `nullptr`, which behaves the same as returning `RV()` directly.
		module_init_func_t* init_func;

		//! @brief The close function of the module.
		//! @details This can be `nullptr`, which behaves like an empty close function.
		module_close_func_t* close_func;
	};

	//! @brief Adds one module to the Runtime. This function can be called before `Luna::init` is called.
	//! 
	//! @details The added module will not be initialized immediately. If the module is added before the Runtime is initialized,
	//! they will be initialized when `Luna::init` is called; if the module is added after the Runtime is initialized,
	//! they must be initialized by one explicit call to `init_modules`. When `Luna::init` or `init_modules` are called,
	//! all dependent modules of the registered module must also be registered.
	//! 
	//! All modules will be closed by their dependency order when `Luna::close` is called. There is no way to close and remove 
	//! modules on the fly, since modules are "unmanaged" and it makes the module implementation very complex to do so.
	//! You should restart the SDK if you make changes to modules.
	//! 
	//! @param[in] module_desc The description structure of the module. This structure should be available during the module lifetime. 
	LUNA_RUNTIME_API void add_module(ModuleDesc* module_desc);

	//! @brief Removes one module from the Runtime. This function cannot be called when the module is currently initialized.
	LUNA_RUNTIME_API void remove_module(ModuleDesc* module_desc);

	//! @brief Initializes all dependency modules for the specified module, but leaves the specified module as uninitialized.
	//! You may use this API to perform some pre-init configurations for the module initialize process.
	//! @param[in] module_name The name of the specified module.
	//! @remark If the specified module is already initialized, this function does nothing and succeeds.
	LUNA_RUNTIME_API RV init_module_dependencies(const Name& module_name);

	//! @brief Initializes the specified module and all dependency modules of the specified module.
	//! @param[in] module_name The name of the specified module.
	//! @remark If the specified module is already initialized, this function does nothing and succeeds.
	LUNA_RUNTIME_API RV init_module(const Name& module_name);

	//! @brief Initializes all uninitialized modules.
	//! @return Returns error code if at least one module is failed to initialize.
	LUNA_RUNTIME_API RV init_modules();

	//! @brief One static module registration helper class that registers the specified module when being constructed.
	//! @details Do not use this structure directly, instead, use @ref LUNA_STATIC_REGISTER_MODULE to register one module, which will define this
	//! structure internally.
	struct StaticRegisterModule
	{
		ModuleDesc module_desc;

		StaticRegisterModule(const c8* name, const c8* dependencies, module_init_func_t* init_func, module_close_func_t* close_func)
		{
			module_desc.name = name;
			module_desc.dependencies = dependencies;
			module_desc.init_func = init_func;
			module_desc.close_func = close_func;
			add_module(&module_desc);
		}
	};

	//! @}
}

//! @brief Registers one module statically. Define this in one of CPP files of your module.
//! @ingroup RuntimeModule
//! @param[in] _name The name of the module. Used by the system to index this module.
//! @param[in] _dependencies The dependency modules of this module, written in one string separated by semicolons (";").
//! @param[in] _init_func The module initialization function.
//! @param[in] _close_func The module close function.
//! @remark See docs of @ref Luna::ModuleDesc for details about every parameter of this macro.
#define LUNA_STATIC_REGISTER_MODULE(_name, _dependencies, _init_func, _close_func) Luna::StaticRegisterModule luna_module_register_##_name(#_name, _dependencies, _init_func, _close_func); \
	extern "C" LUNA_EXPORT void luna_static_register_module_##_name() {}