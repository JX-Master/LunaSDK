LunaSDK is a modular framework. Every function of LunaSDK is provided by one or more modules. The fundamental functions of LunaSDK are provided by the `Runtime` module, which is initialized along with LunaSDK and can be used anywhere. Other functions are provided by dedicated modules and should be added to LunaSDK explicitly when required.

## Module files

Every LunaSDK module should have its own directory under `${ROOT_DIR}/Modules`, with the module name as the directory name. Under the module root directory, every module should have one LunaBuild target rule file named `<ModuleName>.Target.cs`.

The user can use the following code as the starting point for new modules:

```csharp
namespace LunaBuild.Core.Targets;

public sealed class MyModuleTargetRules : TargetRules
{
    public MyModuleTargetRules()
        : base(
            name: "MyModule",
            targetDirectory: "Modules/Luna/MyModule",
            rulesPath: "Modules/Luna/MyModule/MyModule.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "MyDepModule1", "MyDepModule2");
    }
}
```

`Headers(...)` imports public and private header files into IDE views and install metadata. `Sources(...)` imports source files (`.c`, `.cpp`, `.mm`, `.rc`, and platform-specific variants supported by LunaBuild). `DependsOn(...)` adds target dependencies so that include paths, link inputs, frameworks, runtime files, and build order are handled correctly.

One module may be built into static libraries or shared libraries depending on the LunaBuild command line (`--static` or `--shared`). The target rule should describe the module itself; the final linkage mode is selected by the build configuration.

Every module should have one `Source` directory under the module root directory that contains all private files and directories only visible to the current module. All files and directories that are not in `Source` are considered module public files and should not contain module source files (`.c`, `.cpp`, `.mm`, and similar implementation files). LunaSDK sets `${ROOT_DIR}/Modules` as the global include directory for all modules and programs, so you can include module interface files by `#include <Luna/ModuleName/FileName.hpp>`, like `#include <Luna/RHI/RHI.hpp>`.

Use `Configure(...)` for platform-specific files or options:

```csharp
protected override void Configure(BuildWorkspace workspace, BuildOptions options)
{
    if(Platform == BuildPlatform.Windows)
    {
        Sources("Source/Platform/Windows/*.cpp");
    }
    else if(Platform is BuildPlatform.MacOS or BuildPlatform.Linux)
    {
        Sources("Source/Platform/POSIX/*.cpp");
    }
}
```

Legacy `xmake.lua` files may still exist in some module directories as migration references. New build logic should be written in `<ModuleName>.Target.cs`.

## Module namespace

Every module should declare all entities under its own namespace under `Luna` namespace. The namespace name for the module should be concise and may not be equal to the name of the module. Do not declare `using namespace` under module interface header files.

```c++
namespace Luna
{
    namespace MyModule
    {
        // Your declarations go here...
    }
}
```

## Module API declaration

Module API functions and variables should have special linkage and codec specifications to be exported and linked correctly when compiled to shared or static libraries. Every module should use `LUNA_XXX_API` macro to decorate all APIs of the module, where `XXX` is the name of your module. `LUNA_XXX_API` should be defined like so in the module header files:

```c++
#ifndef LUNA_XXX_API
#define LUNA_XXX_API
#endif

// Your API.
LUNA_XXX_API void do_something();
```

When you need to provide definitions for APIs in module source files, define `LUNA_XXX_API` before including header files like so:

```c++
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_XXX_API LUNA_EXPORT
// Include your header files...
```

This will overwrite `LUNA_XXX_API` with `LUNA_EXPORT`, which is a predefined platform-specific macro to append linkage and codec declarations for API functions and objects.

## Module registration

```c++
#include <Luna/Runtime/Module.hpp>
```

One module must be registered to LunaSDK before it can be initialized and used by your program or other modules. Modules are described by `ModuleDesc` structure. You can fill this structure and call `add_module` to add one module to LunaSDK.

```c++
ModuleDesc desc;
desc.name = "MyModule";
desc.dependencies = "MyDepModule1;MyDepModule2"; // `Runtime` is always included and should not be listed here.
desc.init_func = my_module_init;   // Can be `nullptr` if not needed.
desc.close_func = my_module_close; // Can be `nullptr` if not needed.
add_module(&desc);
```

`add_module` is one of few functions that can be called before LunaSDK is initialized. `StaticRegisterModule` uses this behavior to register modules automatically by calling `add_module` in its constructor. We can simply declare it as a global object for our module to register our module automatically when the module library is loaded.

```c++
StaticRegisterModule my_module("MyModule", "MyDepModule1;MyDepModule2", my_module_init, my_module_close);
```

## Module initialization

```c++
#include <Luna/Runtime/Module.hpp>
```

Modules are not initialized along with LunaSDK and should be manually initialized after LunaSDK is initialized. This behavior enables the user to have precise control over module initialization time and perform extra operations before a module is initialized. The module system provides three methods to initialize modules: `init_modules`, `init_module`, and `init_module_dependencies`.

`init_modules` initializes all uninitialized modules registered to LunaSDK, by their dependency order. This is the simplest way to initialize all modules in one call, but the user does not have much control during the module initialization process.

`init_module` initializes one specific module and all recursively dependency modules of that module by their dependency order, while `init_module_dependencies` only initializes all recursively dependency modules of the specified module by their dependency order, but not the specified module. These two functions let the user pause the module initialization process to perform some extra tasks (like choosing the default graphic device), then continue to initialize other modules, which makes the module initialization process more flexible.

## Module closing

Modules are closed along with LunaSDK in the reverse order of their initialization order. There is no approach to close modules manually.
