GUI platforms use different native process entry points. Windows GUI programs enter through `WinMain`, macOS applications need Cocoa application state, iOS applications are launched through UIKit, and Android native applications enter through `android_main`. The Window module provides `<Luna/Window/AppMain.hpp>` to adapt these native entry points to one LunaSDK entry function:

```cpp
int luna_main(int argc, const char* argv[]);
```

`AppMain.hpp` currently supplies adapters for Windows, macOS, iOS, and Android. It does not provide a Linux adapter.

## Using the unified application entry point

Include `<Luna/Window/AppMain.hpp>` in exactly one source file and implement `luna_main` in that file. The header defines a platform entry point, so including it in multiple translation units causes duplicate symbols.

The adapter performs only the platform startup needed to reach `luna_main`. It does not initialize LunaSDK, register modules, create a window, or run the application's update loop. The application remains responsible for all of those steps.

```cpp
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Window/Application.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/Window/AppMain.hpp>

using namespace Luna;

void handle_window_event(object_t event, void*)
{
    if(auto resize = cast_object<Window::WindowFramebufferResizeEvent>(event))
    {
        log_info("Example", "Framebuffer resized to %u x %u", resize->width, resize->height);
    }
}

RV run_app()
{
    lutry
    {
        luexp(add_modules({module_window()}));

        Window::StartupParams startup_params;
        startup_params.name = "Example";
        Window::set_startup_params(startup_params);
        luexp(init_modules());

        Window::set_event_handler(handle_window_event, nullptr);

        Ref<Window::IWindow> window;
#if defined(LUNA_PLATFORM_DESKTOP)
        luset(window, Window::new_window("Example"));
#else
        window = Window::get_system_window();
#endif

        while(!window->is_closed())
        {
            Window::poll_events();
            // Update and render one frame here.
        }

        Window::set_event_handler(nullptr, nullptr);
    }
    lucatchret;
    return ok;
}

int luna_main(int argc, const char* argv[])
{
    if(failed(Luna::init())) return -1;

    RV result = run_app();
    bool app_failed = failed(result);
    if(app_failed)
    {
        log_error("Example", "%s", explain(result.errcode()));
    }

    Luna::close();
    return app_failed ? -1 : 0;
}
```

Set `Window::StartupParams` after adding the Window module but before `init_modules`, because the Window module reads these parameters during initialization. Objects that use Runtime allocation or reference counting must be released before `Luna::close`; keeping application objects inside `run_app` provides that ordering.

## Application event loop

Call `Window::poll_events` regularly on the main thread. With its default argument, it processes pending platform events and returns. Passing `true` waits until an event is available when the queue is empty.

`AppMain.hpp` does not call `poll_events` automatically. This is true on desktop and mobile platforms: the platform adapter reaches `luna_main`, and the application owns the loop that pumps events, updates state, and renders frames.

## Event handling

The Window module has one application-wide event handler, installed with `Window::set_event_handler`. `Window::poll_events` calls that handler synchronously for every fetched event. The event is a boxed object; use `cast_object<T>` to test its concrete type.

Window events derive from `Window::WindowEvent` and retain the target `IWindow` in their `window` member. Application lifecycle events derive from `Window::ApplicationEvent`. Examples include:

* `WindowRequestCloseEvent`
* `WindowFramebufferResizeEvent`
* `WindowDPIScaleChangedEvent`
* `WindowInputTextEvent`
* `ApplicationWillTerminateEvent`

`WindowRequestCloseEvent::do_close` is initialized to `true`. A handler may set it to `false` to reject a close request. Most other events are notifications and have no mutable default action.

The handler and its userdata are global to the Window module and are not retained. Clear the handler before destroying its userdata or before handing event processing to another application component.

## Supplying a native entry point manually

`AppMain.hpp` is optional. An application may define the platform-native entry point itself, perform the required native application startup, and then call its LunaSDK application logic. Platform event translation is implemented by the Window backend; applications do not call backend `dispatch_*` functions, and there is no public `EventDispatching.hpp` API.
