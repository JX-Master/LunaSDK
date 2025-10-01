LunaSDK is a modular framework, every function of LunaSDK is provided by one or more modules. The fundamental functions of LunaSDK are provided by `Runtime` module, which will be initialized along with LunaSDK and is can be used anywhere. Other functions are provided by dedicated modules and should be added to LunaSDK explicitly when required.

## Module files

Every LunaSDK module should have its own directory under `${ROOT_DIR}/Modules` directory, with the module name as the directory name. Under the module root directory, every module must have one `xmake.lua` script defining the building rules of the module. The user can use the following code as the starting point for new modules:

```lua
target("MyModule")
    set_luna_sdk_module()
    add_headerfiles("*.hpp", "Source/**.hpp")
    add_files("Source/**.cpp")
    add_deps("Runtime", "MuDepModule1", "MyDepModule2")
target_end()
```

`set_luna_sdk_module()` tells xmake to import all LunaSDK module global options and specifications to the current module, including one `set_kind` call to properly set the module target file kind. One module may be built into one static library (`.lib` or `.o`) or one shared library (`.dll` or `.so`) based on `build_shared_lib` xmake config of LunaSDK. `add_headerfiles` and `add_files` imports module header files (`.h`, `.hpp` and `.inl`) and module source files (`.c`, `.cpp`) to the module. `add_deps` adds dependency modules for the current module, so that they can be linked correctly.

Every module should have one `Source` directory under the module root directory that contains all private files and directories only visible to the current module. All files and directories that are not in `Source` directory will be considered as module public files and should not contain module source files (`.c`, `.cpp` files). LunaSDK sets `${ROOT_DIR}/Modules` as the global include directory for all modules and programs, so you can simply include module interface files by `#include <Luna/ModuleName/FileName.hpp>`, like `#include <Luna/RHI/RHI.hpp>`.

## Module namespace

Every module should declare all entities under its own namespace under `Luna` namespace. The namespace name for the module should be concise and is may not be equal to the name of the module. Do not declare `using namespace` under the module interface header files.

```c++
namespace Luna
{
	namespace MyModule
	{
		// Your declarations goes here...
	}
}
```

## Module API declaration

Module API functions and variables should have special linkage and codec specifications to be exported and linked correctly when compiled to shared or static libraries. Every module should use `LUNA_XXX_API` macro to decorate all APIs of the module, where `XXX` is the name of your module. `LUNA_XXX_API`  should be defined like so in the module header files:

```c++
#ifndef LUNA_XXX_API
#define LUNA_XXX_API
#endif

// Your API.
LUNA_XXX_API void do_something();
```

When you need to provide definitions for APIs on module source files, define `LUNA_XXX_API`  before including header files like so:

```c++
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_XXX_API LUNA_EXPORT
// Include your header files...
```

This will overwrite `LUNA_XXX_API`  with `LUNA_EXPORT`, which is a predefined platform-specific macro to append linkage and codec declarations for API functions and objects.

## Module registration

```c++
#include <Luna/Runtime/Module.hpp>
```

One module must be registered to LunaSDK before it can be initialized and used by your program or other modules. Modules are described by `ModuleDesc` structure, you can fill this structure and call `add_module` to add one module to LunaSDK.

```c++
ModuleDesc desc;
desc.name = "MyModule";
desc.dependencies = "MuDepModule1;MyDepModule2"; // `Runtime` is always included and should not be listed here.
desc.init_func = my_module_init;	// Can be `nullptr` if not needed.
desc.close_func = my_module_close;	// Can be `nullptr` if not needed.
add_module(&desc);
```

`add_module` is one of few functions that can be called before LunaSDK is initialized. `StaticRegisterModule` uses this behavior to register modules automatically by calling `add_module` in its constructor. We can simply declare it as a global object for our module to register our module automatically when the module library is loaded.

```c++
StaticRegisterModule my_module("MyModule", "MuDepModule1;MyDepModule2", my_module_init, my_module_close);
```

## Module initialization

```c++
#include <Luna/Runtime/Module.hpp>
```

Modules are not initialized along with LunaSDK and should be manually initialized after LunaSDK is initialized. This behavior enables the user to have a precisely control over module initialization time and can perform some extra operations before the module is initialized. Module system provides three methods to initialize modules: `init_modules`, `init_module` and `init_module_dependencies`.

`init_modules` initialize all uninitialized modules registered to LunaSDK, by their dependency order. This is the simplest way to initialize all modules in one call, but the user does not have much control during the module initialization process.

`init_module`  initializes one specific module and all its recursively dependency modules of that module by their dependency order, while `init_module_dependencies` only initializes all recursively dependency modules of the specified module by their dependency order, but not the specified module. These two functions let the user pause the module initialization process to perform some extra tasks (like choosing the default graphic device), then continue to initialize other modules, which make the module initialization process more flexible.

## Module closing

Modules are closed along with LunaSDK in the reverse order of their initialization order. There is no approach to close modules manually.
