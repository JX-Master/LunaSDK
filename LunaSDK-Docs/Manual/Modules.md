LunaSDK is a modular framework. Every function of LunaSDK is provided by one or more modules. The fundamental functions of LunaSDK are provided by the `Runtime` module, which is initialized along with LunaSDK and can be used anywhere. Other functions are provided by dedicated modules and should be added to LunaSDK explicitly when required.

## Module files

Every LunaSDK module should have its own directory under `${ROOT_DIR}/Modules/Luna`, with the module name as the directory name. Under the module root directory, every module should have one LunaBuild target rule file named `<ModuleName>.Target.cs`.

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
        MetaHeaders("MyTypes.hpp", "Source/MyObject.hpp"); // Headers containing reflected declarations.
        Sources("Source/**.cpp");
        DependsOn("Runtime", "MyDepModule1", "MyDepModule2");
    }
}
```

`Headers(...)` imports public and private header files into IDE views and install metadata. `MetaHeaders(...)` lists the headers that LunaMetaTool must process; list reflected headers explicitly rather than every header in the target. `Sources(...)` imports source files (`.c`, `.cpp`, `.mm`, `.rc`, and platform-specific variants supported by LunaBuild). `DependsOn(...)` adds target dependencies so that include paths, link inputs, frameworks, runtime files, and build order are handled correctly.

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
#include <Luna/Runtime/Runtime.hpp>
```

One module must be added to the Runtime module system before it can be initialized and used. A module is represented by a persistent object derived from `Module`. It provides a unique name and may override registration, initialization, and shutdown callbacks.

```c++
#include "MyModule.meta.generated.hpp"

namespace Luna::MyModule
{
    struct MyModule : Module
    {
        virtual const c8* get_name() override
        {
            return "MyModule";
        }

        virtual RV on_register() override
        {
            return add_dependency_modules(this,
                { module_my_dep_module1(), module_my_dep_module2() });
        }

        virtual RV on_init() override
        {
            Meta::register_MyModule_types();
            return my_module_init();
        }

        virtual void on_close() override
        {
            my_module_close();
        }
    };

    Module* module_my_module()
    {
        static MyModule module;
        return &module;
    }
}
```

`on_register` is called when the module is first added. Declare runtime dependencies there with `add_dependency_module` or `add_dependency_modules`; a dependency that has not been added yet is added automatically. `on_init` is the normal place to register generated reflection metadata and initialize module services. When initialization is driven by `init_modules`, the complete dependency graph is initialized in dependency order before its dependents. The current single-module APIs initialize only the dependencies recorded directly on the requested module, as described below, so they do not provide the same transitive guarantee. `on_close` releases module services during Runtime shutdown.

The `Module` object must remain valid while it is registered. Returning a function-local static object from a public `module_<name>()` function is the usual pattern.

Call `add_module` only after `Luna::init` succeeds. Module registration is explicit; declaring a global object does not add it automatically.

```c++
lupanic_if_failed(Luna::init());
lupanic_if_failed(add_module(MyModule::module_my_module()));
```

LunaBuild target dependencies and Runtime module dependencies serve different purposes. `DependsOn(...)` supplies compile, link, and build-order information. `add_dependency_module` supplies the runtime initialization and shutdown graph. A module that requires another module normally declares both relationships.

## Module initialization

```c++
#include <Luna/Runtime/Module.hpp>
```

Modules are not initialized by `Luna::init` and should be initialized explicitly after they are added. This behavior enables the user to control initialization time and perform configuration between dependency initialization and module initialization. The module system provides three methods: `init_modules`, `init_module`, and `init_module_dependencies`.

`init_modules` initializes all uninitialized modules registered to LunaSDK, by their dependency order. This is the simplest way to initialize all modules in one call, but the user does not have much control during the module initialization process.

`init_module` takes a `Module*` and initializes that module after the dependency modules recorded directly for it. `init_module_dependencies` initializes only those directly recorded dependencies and leaves the specified module uninitialized. The latter lets an application initialize dependencies, perform pre-initialization configuration, and then call `init_module` for the module itself. Use `init_modules` when the complete registered dependency graph must be initialized.

```c++
Module* module = MyModule::module_my_module();
lupanic_if_failed(init_module_dependencies(module));
// Configure the module here.
lupanic_if_failed(init_module(module));
```

## Module closing

Initialized modules are closed by `Luna::close` in the reverse order of initialization. There is no API for closing one initialized module independently. `remove_module` can remove only a module that is not initialized.
