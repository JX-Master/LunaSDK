The Frontend module provides a protocol-independent resource registry and synchronous function invocation kernel for LunaSDK applications. It maps URL-like names to functions, [[Variants|Variant]] data, or opaque userdata, and lets callers invoke registered functions with a `Variant` parameter and receive an `R<Variant>` result.

Frontend deliberately does not define a wire protocol. It does not parse or construct JSON-RPC 2.0 objects, validate or match request IDs, unpack or assemble message batches, serialize data, or read and write a transport. Applications that need these features add a **protocol shell** above Frontend. This separation also lets an application use a protocol other than JSON-RPC 2.0 without changing its registered functions.

## Concepts

### Frontend instance

An `IFrontend` instance owns one independent resource registry. Create an instance with `new_frontend()`. New instances have an empty registry and do not share resources.

`IFrontend` is not thread-safe. An application must synchronize every access to the same instance when it may be used by multiple threads. Different instances may be synchronized independently.

### Resource

A **resource** is an entry identified by a non-empty `Name`. Any non-empty name is valid, but absolute paths such as `/Project/Open` are the recommended naming convention. A resource has one of these types:

1. **Function**: A `FunctionHandler` that can be called with `IFrontend::invoke`.
2. **Data**: An arbitrary `Variant` value.
3. **Userdata**: An opaque pointer with an optional destructor.
4. **Null**: The resource does not exist.

The registry does not provide reference tracking. Removing or overwriting a resource immediately invalidates references to it.

For userdata, the registry takes ownership only after `set_resource_userdata` succeeds. If a destructor is provided, it is called exactly once when the resource is removed, overwritten, or when the owning Frontend is destroyed. If registration fails, ownership remains with the caller.

### Function invocation

A `FunctionHandler` receives the owning `IFrontend` and one application-defined parameter `Variant`. It returns `R<Variant>`. `IFrontend::invoke` returns that result directly: it does not wrap successful values in a response object and does not convert an `ErrCode` to a protocol error object.

Every registered handler is stored in a reference-counted boxed object. Before calling a handler, `IFrontend::invoke` retains the box rather than copying the handler. This keeps invocation overhead independent of the size of the handler and keeps the executing handler alive if it removes or overwrites its own registry entry. A handler may therefore synchronously invoke other function resources and modify any registry entry, including itself.

### Protocol shell

A protocol shell translates a transport or message protocol into Frontend operations. For a JSON-RPC 2.0 shell, the shell is responsible for:

1. Parsing JSON-RPC 2.0 objects and extracting fields such as `method` and `params`.
2. Validating request IDs and matching responses to requests.
3. Unpacking message batches and assembling batch responses.
4. Calling `IFrontend::invoke` and converting its `R<Variant>` result into a JSON-RPC 2.0 response or error object.
5. Serializing messages and reading or writing the selected transport.

A complete JSON-RPC 2.0 object must therefore not be passed directly to `IFrontend::invoke`. The shell passes the extracted method as the resource name and the extracted parameters as the parameter `Variant`.

## Programming guide

### Initialize and create a Frontend

Register the Frontend module during application initialization, then create an instance:

```cpp
init();
lupanic_if_failed(add_modules({Frontend::module_frontend()}));
lupanic_if_failed(init_modules());

Ref<Frontend::IFrontend> frontend = Frontend::new_frontend();
```

Release every Frontend instance before calling `close()`.

### Register and invoke a function

Register functions with `set_resource_function`. By default, registration fails with `BasicError::already_exists` when the URL is occupied. Pass `true` as `overwrite` to replace the old resource.

```cpp
using namespace Luna;
using namespace Luna::Frontend;

lupanic_if_failed(frontend->set_resource_function(
    "/Math/Add",
    FunctionHandler([](IFrontend*, const Variant& params) -> R<Variant>
    {
        if(params.type() != VariantType::object)
        {
            return BasicError::bad_arguments();
        }
        i64 result = params["a"].inum() + params["b"].inum();
        return Variant(result);
    })));

Variant params(VariantType::object);
params["a"] = Variant((i64)2);
params["b"] = Variant((i64)3);
R<Variant> result = frontend->invoke("/Math/Add", params);
if(result.valid())
{
    i64 value = result.get().inum();
}
```

Invoking a missing resource or a non-function resource returns `FrontendError::method_not_found`. Errors returned by a handler are propagated unchanged.

### Store data and userdata

Use `set_resource_data` and `get_resource_data` for `Variant` resources. Use `set_resource_userdata` and `get_resource_userdata` for opaque pointers. Both getters return `FrontendError::resource_not_found` for an absent resource and `FrontendError::type_mismatch` for a resource of the wrong type.

```cpp
lupanic_if_failed(frontend->set_resource_data(
    "/Configuration", Variant(VariantType::object)));

R<Variant> configuration = frontend->get_resource_data("/Configuration");
```

`get_resource_userdata` returns a borrowed pointer. The pointer may become invalid after the resource is removed or overwritten, or after the Frontend is destroyed.

### Remove resources and shut down

`remove_resource` succeeds without effect if the URL does not exist. A successful removal releases the resource immediately. Empty resource names are rejected with `BasicError::bad_arguments`.

```cpp
lupanic_if_failed(frontend->remove_resource("/Math/Add"));
frontend.reset();
close();
```
