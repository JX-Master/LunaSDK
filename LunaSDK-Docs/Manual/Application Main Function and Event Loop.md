An old-school C++ application always starts with one main function similar to this:
```c++
int main(int argc, const char* argv[]);
```
this works for command-line programs, but not for GUI-based applications, since most modern operation systems have special requirements for the entry function of one GUI application. For example, Windows wants the application to use `WinMain` instead of `main`, macOS and iOS want the application to only call  `NSApplicationMain` and  `UIApplicationMain` in `main`, WASM does not have `main` at all, the application can only provide some callbacks that will be called by the system at certain condition. In order to wrap up such platform differences, we provide a "main callbacks" interface to replace the old-school `main` function as the entry point of one GUI application. If you want to develop a GUI application, you should either use the new interface, or writing different version of main functions for different platform manually.

## The Main Callbacks Interface

In order to use the main callbacks interface, the application should `#include <Luna/Window/AppMain.hpp>` in the application's `main.cpp` file. `AppMain.hpp` contains inline main function implementation for every different platform, so the application should only include this header once in only one cpp file. After including this header, the application should implement three functions that will be called by the system:
```c++
Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[]);

Window::AppStatus app_update(opaque_t app_state);

void app_close(opaque_t app_state, Window::AppStatus status);
```
as you may guess from their names, `app_init` will be called firstly when the application is initializing, then `app_update` will be called repeatedly until the return value of `app_state` is not `AppStatus::running`. After that `app_close` will be called to close the application. In `app_init`, the application can optionally returns one opaque pointer by setting `app_state` parameter, the pointer will be passed to `app_update` and `app_close` as one parameter. Both `app_init` and `app_update` returns one enumeration type `AppStatus`, which has three options: `running`, `failing`, `exiting`. When `app_init` or `app_update` returns `AppStatus::running`, the system continues to run the application event loop. When `app_init` or `app_update` returns `AppStatus::exiting` or `AppStatus::failed`, the system calls `app_close` to close the application and returns normal or abnormal exiting code.

Note that LunaSDK itself is not initialized by the system, the application should call `Luna::init` in `app_init` explicitly to initialize LunaSDK, and also call `Luna::close` in `app_close` explicitly to close LunaSDK.

## Application Event Loop

Almost all modern operating systems use a event queue pattern to post system-level events to the application, which requires the application to have some form of event loop to repeatedly pop events from the queue and handle them. If the application uses [[Application Main Function and Event Loop#The Main Callbacks Interface|main callbacks interface]], then the application event loop is managed by the system automatically, otherwise, the application should implement the event loop manually and posts events to the Window system. See [[Application Main Function and Event Loop#Implementing Main Function Manually|Implementing Main Function Manually]] for details.

### Event Handling

Events are categorized in **application events** and **window events**. Window events are events dispatched to one specific window (usually the window that gains user focus), while application events are events dispatched to the application directly. Both types of events are handled by registering callback functions to event objects provided by the system, and the system will call these callbacks when the event occurs.

## Implementing Main Function Manually

If for some reasons, your application cannot use main callbacks interface, then you should implement the application entry point ("main function") manually. In such case, the application should post system-level events to LunaSDK window system manually by calling `dispatch_xxx_event` functions in `<Luna/Window/EventDispatching.hpp>`.